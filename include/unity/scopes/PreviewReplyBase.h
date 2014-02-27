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

#ifndef UNITY_SCOPES_PREVIEW_REPLY_BASE_H
#define UNITY_SCOPES_PREVIEW_REPLY_BASE_H

#include <unity/SymbolExport.h>
#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/ReplyBase.h>

#include <string>

namespace unity
{
namespace scopes
{
/**
\brief Reply allows the results of a query to be sent to the source of the query.
*/
class PreviewReplyBase : public virtual ReplyBase
{
public:
    virtual ~PreviewReplyBase() = default;
    PreviewReplyBase(PreviewReplyBase const&) = delete;

    /**
     \brief Registers a list of column layouts for current preview.
     Layouts need to be registered before pushing PreviewWidgetList, and only once in the lieftime of this PreviewReply lifetime.
     This method throws unity::LogicException if this constrains are violated.
     */
    virtual bool register_layout(ColumnLayoutList const& layouts) const = 0;

    /**
     \brief Sends widget definitions to the sender of the preview query.
     */
    virtual bool push(PreviewWidgetList const& widget_list) const = 0;

    /**
     \brief Sends data for a preview widget attribute.
     */
    virtual bool push(std::string const& key, Variant const& value) const = 0;

protected:
    PreviewReplyBase() = default;
};

} // namespace scopes

} // namespace unity

#endif