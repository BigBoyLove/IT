#include "IRIBackend.hpp"

namespace rdf4cpp::rdf::storage::node::metall_node_storage {

IRIBackend::IRIBackend(std::string_view iri) noexcept : iri(iri) {}
IRIBackend::IRIBackend(view::IRIBackendView view) noexcept : iri(view.identifier) {}
std::partial_ordering IRIBackend::operator<=>(metall::offset_ptr<IRIBackend> const &other) const noexcept {
    if (other)
        return *this <=> *other;
    else
        return std::partial_ordering::greater;
}
std::string_view IRIBackend::identifier() const noexcept {
    return iri;
}
IRIBackend::operator view::IRIBackendView() const noexcept {
    return {.identifier = identifier()};
}
std::partial_ordering operator<=>(const metall::offset_ptr<IRIBackend> &self, const metall::offset_ptr<IRIBackend> &other) noexcept {
    return *self <=> *other;
}
}  // namespace rdf4cpp::rdf::storage::node::metall_node_storage