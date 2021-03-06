/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#pragma once

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>

namespace unity
{

namespace scopes
{

namespace internal
{

// Simple thread-safe queue of items.
// If the queue is destroyed while threads are blocked in wait_and_pop(), wait_and_pop() throws std::runtime_error.

template<typename T>
class ThreadSafeQueue final
{
public:
    NONCOPYABLE(ThreadSafeQueue);
    UNITY_DEFINES_PTRS(ThreadSafeQueue);

    typedef T value_type;

    ThreadSafeQueue();
    ~ThreadSafeQueue();

    void destroy() noexcept;
    void wait_for_destroy() noexcept;
    void push(T const& item);
    void push(T&& item);
    T wait_and_pop();
    bool try_pop(T& item);
    bool empty() const noexcept;
    void wait_until_empty() const noexcept;
    size_t size() const noexcept;

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    mutable std::condition_variable cond_;
    bool destroyed_;
    int num_waiters_;
};

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue() :
    destroyed_(false),
    num_waiters_(0)
{
}

template<typename T>
ThreadSafeQueue<T>::~ThreadSafeQueue()
{
    destroy();
    wait_for_destroy();
}

template<typename T>
void ThreadSafeQueue<T>::destroy() noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (destroyed_)
    {
        return;
    }
    destroyed_ = true;
    cond_.notify_all(); // Wake up anyone asleep in wait_and_pop() or wait_for_destroy()
}

template<typename T>
void ThreadSafeQueue<T>::wait_for_destroy() noexcept
{
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return destroyed_ && num_waiters_ == 0; });
}

template<typename T>
void ThreadSafeQueue<T>::push(T const& item)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (destroyed_)
    {
        throw std::runtime_error("ThreadSafeQueue: cannot push onto destroyed queue");
    }
    queue_.push(item);
    cond_.notify_all();
}

template<typename T>
void ThreadSafeQueue<T>::push(T&& item)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (destroyed_)
    {
        throw std::runtime_error("ThreadSafeQueue: cannot push onto destroyed queue");
    }
    queue_.emplace(std::move(item));
    cond_.notify_all();
}

template<typename T>
T ThreadSafeQueue<T>::wait_and_pop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    ++num_waiters_;
    cond_.wait(lock, [this] { return destroyed_ || queue_.size() != 0; });
    if (destroyed_)
    {
        if (--num_waiters_ == 0)
        {
            cond_.notify_all();
        }
        throw std::runtime_error("ThreadSafeQueue: queue destroyed while thread was blocked in wait_and_pop()");
    }
    T item = std::move(queue_.front());
    queue_.pop();
    if (--num_waiters_ == 0 || queue_.empty())
    {
        cond_.notify_all();
    }
    return item;
}

template<typename T>
bool ThreadSafeQueue<T>::try_pop(T& item)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty())
    {
        return false;
    }
    item = std::move(queue_.front());
    queue_.pop();
    if (queue_.empty())
    {
        cond_.notify_all();
    }
    return true;
}

template<typename T>
bool ThreadSafeQueue<T>::empty() const noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

template<typename T>
void ThreadSafeQueue<T>::wait_until_empty() const noexcept
{
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return queue_.empty(); });
}

template<typename T>
size_t ThreadSafeQueue<T>::size() const noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

} // namespace internal

} // namespace scopes

} // namespace unity
