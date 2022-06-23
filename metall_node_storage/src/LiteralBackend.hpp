#ifndef METALL_LITERALBACKEND_HPP
#define METALL_LITERALBACKEND_HPP

#include <metall/metall.hpp>

#include <rdf4cpp/rdf/storage/node/identifier/NodeID.hpp>
#include <rdf4cpp/rdf/storage/node/view/LiteralBackendView.hpp>

#include <compare>
#include <memory>
#include <string>
#include <string_view>

namespace rdf4cpp::rdf::storage::node::metall_node_storage {

class LiteralBackend {
    identifier::NodeID datatype_id_;
    std::string lexical;
    std::string lang_tag;

public:
    [[nodiscard]] std::string quote_lexical() const noexcept;
    LiteralBackend(std::string_view lexical, identifier::NodeID dataType, std::string_view langTag = "") noexcept;
    explicit LiteralBackend(view::LiteralBackendView view) noexcept;
    std::partial_ordering operator<=>(LiteralBackend const &) const noexcept;
    auto operator<=>(view::LiteralBackendView const &other) const noexcept {
        return view::LiteralBackendView(*this) <=> other;
    }
    std::partial_ordering operator<=>(metall::offset_ptr<LiteralBackend> const &other) const noexcept;

    bool operator==(const LiteralBackend &) const noexcept;

    [[nodiscard]] std::string_view lexical_form() const noexcept;

    [[nodiscard]] const identifier::NodeID &datatype_id() const noexcept;

    [[nodiscard]] std::string_view language_tag() const noexcept;

    explicit operator view::LiteralBackendView() const noexcept;
};


std::partial_ordering operator<=>(metall::offset_ptr<LiteralBackend> const &self, metall::offset_ptr<LiteralBackend> const &other) noexcept;
}  // namespace rdf4cpp::rdf::storage::node::metall_node_storage

namespace rdf4cpp::rdf::storage::node::view {
inline std::partial_ordering operator<=>(LiteralBackendView const &lhs, metall::offset_ptr<metall_node_storage::LiteralBackend> const &rhs) noexcept {
    return lhs <=> LiteralBackendView(*rhs);
}
}  // namespace rdf4cpp::rdf::storage::node::view

#endif  //METALL_LITERALBACKEND_HPP
