#ifndef METALLNODESTORAGEBACKEND_HPP
#define METALLNODESTORAGEBACKEND_HPP

#include <metall/metall.hpp>

#include <rdf4cpp/rdf/storage/node/INodeStorageBackend.hpp>

#include "BNodeBackend.hpp"
#include "IRIBackend.hpp"
#include "LiteralBackend.hpp"
#include "VariableBackend.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include <metall/container/map.hpp>

namespace rdf4cpp::rdf::storage::node::metall_node_storage {

/**
 * Thread-safe reference implementation of a INodeStorageBackend. Deleting nodes is not supported.
 */
class MetallNodeStorageBackend : public INodeStorageBackend {
public:
    using NodeID = identifier::NodeID;
    using LiteralID = identifier::LiteralID;
    using Alloc = metall::manager::allocator_type<std::byte>;

private:
    mutable std::shared_mutex literal_mutex_;
    metall::offset_ptr<typename metall::container::map<NodeID,typename metall::offset_ptr<LiteralBackend>, std::less<>>> literal_storage;
    metall::offset_ptr<typename metall::container::map<typename metall::offset_ptr<LiteralBackend>, NodeID, std::less<>>> literal_storage_reverse;
    mutable std::shared_mutex bnode_mutex_;
    metall::offset_ptr<typename metall::container::map<NodeID,typename metall::offset_ptr<BNodeBackend>, std::less<>>> bnode_storage;
    metall::offset_ptr<typename metall::container::map<typename metall::offset_ptr<BNodeBackend>, NodeID, std::less<>>> bnode_storage_reverse;
    mutable std::shared_mutex iri_mutex_;
    metall::offset_ptr<typename metall::container::map<NodeID,typename metall::offset_ptr<IRIBackend>, std::less<>>> iri_storage;
    metall::offset_ptr<typename metall::container::map<typename metall::offset_ptr<IRIBackend>, NodeID, std::less<>>> iri_storage_reverse;
    mutable std::shared_mutex variable_mutex_;
    metall::offset_ptr<typename metall::container::map<NodeID,typename metall::offset_ptr<VariableBackend>, std::less<>>> variable_storage;
    metall::offset_ptr<typename metall::container::map<typename metall::offset_ptr<VariableBackend>, NodeID, std::less<>>> variable_storage_reverse;

    Alloc alloc;

    LiteralID next_literal_id = NodeID::min_literal_id;
    NodeID next_bnode_id = NodeID::min_bnode_id;
    NodeID next_iri_id = NodeID::min_iri_id;
    NodeID next_variable_id = NodeID::min_variable_id;

public:
    explicit MetallNodeStorageBackend(const Alloc &al);

    ~MetallNodeStorageBackend() override = default;

//    Alloc& get_allocator(){
//        return alloc;
//    }

//    const Alloc & get_allocator() const{
//        return alloc;
//    }

    [[nodiscard]] identifier::NodeID find_or_make_id(view::BNodeBackendView const &) noexcept override;
    [[nodiscard]] identifier::NodeID find_or_make_id(view::IRIBackendView const &) noexcept override;
    [[nodiscard]] identifier::NodeID find_or_make_id(view::LiteralBackendView const &) noexcept override;
    [[nodiscard]] identifier::NodeID find_or_make_id(view::VariableBackendView const &) noexcept override;

    [[nodiscard]] identifier::NodeID find_id(view::BNodeBackendView const &) const noexcept override;
    [[nodiscard]] identifier::NodeID find_id(view::IRIBackendView const &) const noexcept override;
    [[nodiscard]] identifier::NodeID find_id(view::LiteralBackendView const &) const noexcept override;
    [[nodiscard]] identifier::NodeID find_id(view::VariableBackendView const &) const noexcept override;

    [[nodiscard]] view::IRIBackendView find_iri_backend_view(identifier::NodeID id) const override;
    [[nodiscard]] view::LiteralBackendView find_literal_backend_view(identifier::NodeID id) const override;
    [[nodiscard]] view::BNodeBackendView find_bnode_backend_view(identifier::NodeID id) const override;
    [[nodiscard]] view::VariableBackendView find_variable_backend_view(identifier::NodeID id) const override;

    bool erase_iri(identifier::NodeID id) const override;
    bool erase_literal(identifier::NodeID id) const override;
    bool erase_bnode(identifier::NodeID id) const override;
    bool erase_variable(identifier::NodeID id) const override;
};

}  // namespace rdf4cpp::rdf::storage::node::metall_node_storage
#endif  //METALLNODESTORAGEBACKEND_HPP
