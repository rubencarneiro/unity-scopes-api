#pragma once

#include <unity/scopes/SearchReply.h>
#include <unity/scopes/SearchReplyProxyFwd.h>
#include <memory>

namespace unity
{

namespace scopes
{

namespace internal
{

class BufferedSearchReplyImpl : public virtual unity::scopes::SearchReply
{
public:
    BufferedSearchReplyImpl(unity::scopes::SearchReplyProxy const& upstream);

    void register_departments(Department::SCPtr const& parent) override;

    Category::SCPtr register_category(std::string const& id,
                                              std::string const& title,
                                              std::string const &icon,
                                              CategoryRenderer const& renderer_template) override;
    Category::SCPtr register_category(std::string const& id,
                                              std::string const& title,
                                              std::string const &icon,
                                              CannedQuery const &query,
                                              CategoryRenderer const& renderer_template) override;
    void register_category(Category::SCPtr category) override;
    Category::SCPtr lookup_category(std::string const& id)  override;

    bool push(unity::scopes::experimental::Annotation const& annotation) override;

    bool push(unity::scopes::CategorisedResult const& result) override;
    bool push(unity::scopes::Filters const& filters, unity::scopes::FilterState const& filter_state) override;

    // Reply interface
    void finished() override;
    void error(std::exception_ptr ex) override;
    void info(OperationInfo const& op_info) override;

    // Object interface
    std::string endpoint() override;
    std::string identity() override;
    std::string target_category() override;
    int64_t timeout() override;
    std::string to_string() override;

private:
    unity::scopes::SearchReplyProxy const& upstream_;
};

}

}

}

