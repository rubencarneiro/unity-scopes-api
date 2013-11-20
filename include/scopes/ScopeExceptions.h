/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_SCOPEEXCEPTIONS_H
#define UNITY_API_SCOPES_SCOPEEXCEPTIONS_H

#include <unity/Exception.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class MiddlewareExceptionImpl;
}

/**
\brief Exception to indicate that something went wrong with the middleware layer.
*/

class UNITY_API MiddlewareException : public unity::Exception
{
public:
    /**
    \brief Constructs the exception.
    \param reason Further details about the cause of the exception.
    */
    explicit MiddlewareException(std::string const& reason);
    //! @cond
    MiddlewareException(MiddlewareException const&);
    MiddlewareException& operator=(MiddlewareException const&);
    virtual ~MiddlewareException() noexcept;
    //! @endcond

    /**
    \brief Returns the fully-qualified name of the exception.
    */
    virtual char const* what() const noexcept override;

    /**
    \brief Returns a <code>std::exception_ptr</code> to <code>this</code>.
    */
    virtual std::exception_ptr self() const override;
};

namespace internal
{
class ConfigExceptionImpl;
}

/**
\brief Exception to indicate that something went wrong with the contents of configuration files.
*/

class UNITY_API ConfigException : public unity::Exception
{
public:
    /**
    \brief Constructs the exception.
    \param reason Further details about the cause of the exception.
    */
    explicit ConfigException(std::string const& reason);
    //! @cond
    ConfigException(ConfigException const&);
    ConfigException& operator=(ConfigException const&);
    virtual ~ConfigException() noexcept;
    //! @endcond

    /**
    \brief Returns the fully-qualified name of the exception.
    */
    virtual char const* what() const noexcept override;

    /**
    \brief Returns a <code>std::exception_ptr</code> to <code>this</code>.
    */
    virtual std::exception_ptr self() const override;
};

namespace internal
{
class NotFoundExceptionImpl;
}

/**
\brief Exception to indicate that an object wasn't found by a lookup function.
*/

class UNITY_API NotFoundException : public unity::Exception
{
public:
    /**
    \brief Constructs the exception.
    \param reason Further details about the cause of the exception.
    \param name The name of the object that was not found.
    */
    explicit NotFoundException(std::string const& reason, std::string const& name);
    //! @cond
    NotFoundException(NotFoundException const&);
    NotFoundException& operator=(NotFoundException const&);
    virtual ~NotFoundException() noexcept;
    //! @endcond

    /**
    \brief Returns the fully-qualified name of the exception.
    */
    virtual char const* what() const noexcept override;

    /**
    \brief Returns a <code>std::exception_ptr</code> to <code>this</code>.
    */
    virtual std::exception_ptr self() const override;

    /**
    \brief Returns the name that was passed to the constructor.
    */
    virtual std::string name() const;

private:
    std::shared_ptr<internal::NotFoundExceptionImpl> p;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif