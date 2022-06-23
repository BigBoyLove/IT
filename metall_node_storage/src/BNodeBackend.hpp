#ifndef METALL_BNODEBACKEND_HPP
#define METALL_BNODEBACKEND_HPP

#include <metall/metall.hpp>

#include <rdf4cpp/rdf/storage/node/identifier/NodeID.hpp>
#include <rdf4cpp/rdf/storage/node/view/BNodeBackendView.hpp>

#include <compare>
#include <memory>
#include <string>
#include <string_view>

namespace rdf4cpp::rdf::storage::node::metall_node_storage {

class BNodeBackend {
    std::string identifier_;

public:
    explicit BNodeBackend(std::string_view identifier) noexcept;
    explicit BNodeBackend(view::BNodeBackendView view) noexcept;
    auto operator<=>(BNodeBackend const &) const noexcept = default;
    auto operator<=>(view::BNodeBackendView const &other) const noexcept {
        return view::BNodeBackendView(*this) <=> other;
    }
    std::partial_ordering operator<=>(metall::offset_ptr<BNodeBackend> const &other) const noexcept;

    [[nodiscard]] std::string_view identifier() const noexcept;

    explicit operator view::BNodeBackendView() const noexcept;
};

std::partial_ordering operator<=>(metall::offset_ptr<BNodeBackend> const &self, metall::offset_ptr<BNodeBackend> const &other) noexcept;
}  // namespace rdf4cpp::rdf::storage::node::metall_node_storage

namespace rdf4cpp::rdf::storage::node::view {
inline std::partial_ordering operator<=>(BNodeBackendView const &lhs, metall::offset_ptr<metall_node_storage::BNodeBackend> const &rhs) noexcept {
    return lhs <=> BNodeBackendView(*rhs);
}
}  // namespace rdf4cpp::rdf::storage::node::view
#endif  //METALL_BNODEBACKEND_HPP
