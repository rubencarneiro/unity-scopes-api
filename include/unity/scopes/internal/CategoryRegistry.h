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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#pragma once

#include <unity/scopes/Category.h>
#include <unity/scopes/Variant.h>
#include <string>
#include <memory>
#include <mutex>

namespace unity
{

namespace scopes
{

class CategoryRenderer;

namespace internal
{

/**
\brief Provides factory method for creating Category instances and keeping track of them.
This class is for internal use, most of its functionality is exposed via register_category and find_category method
of Reply class.
*/
class CategoryRegistry
{
public:
    CategoryRegistry() = default;

    /**
    \brief Deserializes category from a variant_map and registers it.
    \throws unity::InvalidArgumentException if a category with the same id exists already.
    \return category instance
    */
    Category::SCPtr register_category(VariantMap const& variant_map);

    /**
    \brief Creates category from supplied parameters.
    \throws unity::InvalidArgumentException if a category with the same id exists already.
    \return category instance
    */
    Category::SCPtr register_category(std::string const& id, std::string const& title, std::string const& icon, CannedQuery::SCPtr const& query, CategoryRenderer const& renderer_template);

    /**
    \brief Finds category instance with give id.
    \return category instance or nullptr if no such category has been registered
    */
    Category::SCPtr lookup_category(std::string const& id) const;

    /**
    \brief Register an existing category instance with this registry.
    \throws unity::InvalidArgumentException if a category with the same id exists already.
    */
    void register_category(Category::SCPtr category);

    /**
    \brief Serializes all categories in the registry.
    \return VariantArray containing all categories.
    */
    VariantArray serialize() const;

private:
    mutable std::mutex mutex_;
    typedef std::pair<std::string, Category::SCPtr> CatPair;
    std::vector<CatPair> categories_;                         // vector instead of map, so we preserve order
};

} // namespace internal

} // namespace scopes

} // namespace unity
