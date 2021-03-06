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

#include <unity/scopes/internal/ScopeObject.h>

#include <unity/scopes/internal/ActivationQueryObject.h>
#include <unity/scopes/internal/MWQuery.h>
#include <unity/scopes/internal/MWReply.h>
#include <unity/scopes/internal/PreviewQueryObject.h>
#include <unity/scopes/internal/QueryBaseImpl.h>
#include <unity/scopes/internal/QueryCtrlObject.h>
#include <unity/scopes/internal/QueryObject.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeBaseImpl.h>
#include <unity/scopes/internal/SearchMetadataImpl.h>
#include <unity/scopes/internal/SearchQueryBaseImpl.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <sstream>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

namespace unity
{

namespace scopes
{

namespace internal
{

ScopeObject::ScopeObject(ScopeBase* scope_base, bool debug_mode) :
    scope_base_(scope_base),
    debug_mode_(debug_mode)
{
    assert(scope_base);
}

ScopeObject::~ScopeObject()
{
}

MWQueryCtrlProxy ScopeObject::query(MWReplyProxy const& reply,
        MiddlewareBase* mw_base,
        std::string const& method,
        std::function<QueryBase::SPtr()> const& query_factory_fun,
        std::function<QueryObjectBase::SPtr(QueryBase::SPtr, MWQueryCtrlProxy)> const& query_object_factory_fun)
{
    if (!reply)
    {
        // We can't assert here because the null proxy may have been sent by a broken client, that is,
        // it can be null because it was sent by the remote end as null. This should never happen but,
        // to be safe, we don't assert, in case someone is running a broken client.

        // TODO: log error about incoming request containing an invalid reply proxy.
        throw LogicException("Scope \"" + mw_base->runtime()->scope_id() + "\": "
                             + method + " called with null reply proxy");
    }

    // Ask scope to instantiate a new query.
    QueryBase::SPtr query_base;
    try
    {
        query_base = query_factory_fun();
        if (!query_base)
        {
            string msg = "Scope \"" + mw_base->runtime()->scope_id() + "\" returned nullptr from " + method + "()";
            mw_base->runtime()->logger()() << msg;
            throw ResourceException(msg);
        }
        query_base->p->set_settings_db(scope_base_->p->settings_db());
    }
    catch (...)
    {
        string msg = "Scope \"" + mw_base->runtime()->scope_id() + "\" threw an exception from " + method + "()";
        mw_base->runtime()->logger()() << msg;
        throw ResourceException(msg);
    }

    MWQueryCtrlProxy ctrl_proxy;
    try
    {
        // Instantiate the query ctrl and connect it to the middleware.
        QueryCtrlObject::SPtr co(make_shared<QueryCtrlObject>());
        ctrl_proxy = mw_base->add_query_ctrl_object(co);

        // Instantiate the query. We tell it what the ctrl is so,
        // when the query completes, it can tell the ctrl object
        // to destroy itself.
        QueryObjectBase::SPtr qo(query_object_factory_fun(query_base, ctrl_proxy));
        MWQueryProxy query_proxy = mw_base->add_query_object(qo);

        // We tell the ctrl what the query facade is so, when cancel() is sent
        // to the ctrl, it can forward it to the facade.
        co->set_query(qo);

        // Start the query. We call via the middleware, which calls
        // the run() implementation in a different thread, so we cannot block here.
        // We pass a shared_ptr to the qo to the qo itself, so the qo can hold the reference
        // count high until the run() request arrives in the query via the middleware.
        qo->set_self(qo);

        query_proxy->run(reply);
    }
    catch (std::exception const& e)
    {
        try
        {
            reply->finished(CompletionDetails(CompletionDetails::Error, e.what()));  // Oneway, can't block
        }
        catch (...)
        {
        }
        mw_base->runtime()->logger()() << "ScopeObject::query(): " << e.what();
        throw;
    }
    catch (...)
    {
        try
        {
            reply->finished(CompletionDetails(CompletionDetails::Error, "unknown exception"));  // Oneway, can't block
        }
        catch (...)
        {
        }
        mw_base->runtime()->logger()() << "ScopeObject::query(): unknown exception";
        throw;
    }
    return ctrl_proxy;
}

MWQueryCtrlProxy ScopeObject::search(CannedQuery const& q,
                                     SearchMetadata const& hints,
                                     VariantMap const& context,
                                     MWReplyProxy const& reply,
                                     InvokeInfo const& info)
{
    return query(reply,
                 info.mw,
                 "search",
                 [&q, &hints, &context, this]() -> SearchQueryBase::UPtr {
                      auto search_query = this->scope_base_->search(q, hints);
                      search_query->set_department_id(q.department_id());

                      auto sqb = dynamic_cast<SearchQueryBaseImpl*>(search_query->fwd());
                      assert(sqb);

                      // Set client ID and history that we received in the SearchQueryBase
                      // for loop detection.
                      auto const c_it = context.find("client_id");
                      if (c_it != context.end())
                      {
                          string client_id;
                          client_id = c_it->second.get_string();
                          sqb->set_client_id(client_id);
                      }

                      auto const h_it = context.find("history");
                      if (h_it != context.end())
                      {
                         auto const hlist = h_it->second.get_array();
                         SearchQueryBaseImpl::History history;
                         for (auto const& t : hlist)
                         {
                             string client_id = t.get_dict()["c"].get_string();
                             string agg = t.get_dict()["a"].get_string();
                             string recv = t.get_dict()["r"].get_string();
                             string details = t.get_dict()["d"].get_string();
                             SearchQueryBaseImpl::HistoryData hd = make_tuple(client_id, agg, recv, details);
                             history.push_back(hd);
                         }
                         sqb->set_history(history);
                      }

                      return search_query;
                 },
                 [&reply, &hints, this](QueryBase::SPtr query_base, MWQueryCtrlProxy ctrl_proxy) -> QueryObjectBase::SPtr {
                     return make_shared<QueryObject>(query_base, hints.cardinality(), reply, ctrl_proxy);
                 }
    );
}

MWQueryCtrlProxy ScopeObject::activate(Result const& result,
                                       ActionMetadata const& hints,
                                       MWReplyProxy const& reply,
                                       InvokeInfo const& info)
{
    return query(reply,
                 info.mw,
                 "activate",
                 [&result, &hints, this]() -> QueryBase::SPtr {
                     return this->scope_base_->activate(result, hints);
                 },
                 [&reply, this](QueryBase::SPtr query_base, MWQueryCtrlProxy ctrl_proxy) -> QueryObjectBase::SPtr {
                     auto activation_base = dynamic_pointer_cast<ActivationQueryBase>(query_base);
                     assert(activation_base);
                     return make_shared<ActivationQueryObject>(activation_base, reply, ctrl_proxy);
                 }
    );
}

MWQueryCtrlProxy ScopeObject::perform_action(Result const& result,
                                             ActionMetadata const& hints,
                                             std::string const& widget_id,
                                             std::string const& action_id,
                                             MWReplyProxy const &reply,
                                             InvokeInfo const& info)
{
    return query(reply,
                 info.mw,
                 "perform_action",
                 [&result, &hints, &widget_id, &action_id, this]() -> QueryBase::SPtr {
                     return this->scope_base_->perform_action(result, hints, widget_id, action_id);
                 },
                 [&reply, this](QueryBase::SPtr query_base, MWQueryCtrlProxy ctrl_proxy) -> QueryObjectBase::SPtr {
                     auto activation_base = dynamic_pointer_cast<ActivationQueryBase>(query_base);
                     assert(activation_base);
                     return make_shared<ActivationQueryObject>(activation_base, reply, ctrl_proxy);
                 }
    );
}

MWQueryCtrlProxy ScopeObject::activate_result_action(Result const& result,
                                                    ActionMetadata const& hints,
                                                    std::string const& action_id,
                                                    MWReplyProxy const &reply,
                                                    InvokeInfo const& info)
{
    return query(reply,
                 info.mw,
                 "activate_result_action",
                 [&result, &hints, &action_id, this]() -> QueryBase::SPtr {
                     return this->scope_base_->activate_result_action(result, hints, action_id);
                 },
                 [&reply, this](QueryBase::SPtr query_base, MWQueryCtrlProxy ctrl_proxy) -> QueryObjectBase::SPtr {
                     auto activation_base = dynamic_pointer_cast<ActivationQueryBase>(query_base);
                     assert(activation_base);
                     return make_shared<ActivationQueryObject>(activation_base, reply, ctrl_proxy);
                 }
    );
}

MWQueryCtrlProxy ScopeObject::preview(Result const& result,
                                      ActionMetadata const& hints,
                                      MWReplyProxy const& reply,
                                      InvokeInfo const& info)
{
    return query(reply,
                 info.mw,
                 "preview",
                 [&result, &hints, this]() -> QueryBase::SPtr {
                     return this->scope_base_->preview(result, hints);
                 },
                 [&reply, this](QueryBase::SPtr query_base, MWQueryCtrlProxy ctrl_proxy) -> QueryObjectBase::SPtr {
                     auto preview_query = dynamic_pointer_cast<PreviewQueryBase>(query_base);
                     assert(preview_query);
                     return make_shared<PreviewQueryObject>(preview_query, reply, ctrl_proxy);
                 }
    );
}

ChildScopeList ScopeObject::child_scopes() const
{
    return scope_base_->child_scopes();
}

bool ScopeObject::set_child_scopes(ChildScopeList const& child_scopes)
{
    return scope_base_->p->set_child_scopes(child_scopes);
}

bool ScopeObject::debug_mode() const
{
    return debug_mode_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
