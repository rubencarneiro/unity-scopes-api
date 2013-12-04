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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTINTERFACE_H
#define UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTINTERFACE_H

#include <unity/util/DefinesPtrs.h>

#include <future>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class HttpClientInterface
{
public:
  UNITY_DEFINES_PTRS(HttpClientInterface);

  HttpClientInterface() = default;
  virtual ~HttpClientInterface() = default;

  virtual std::future< std::string > get( const std::string& request_url, int port = 80 ) = 0;

  virtual std::string to_html_escaped( const std::string& string ) = 0;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif // UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTINTERFACE_H
