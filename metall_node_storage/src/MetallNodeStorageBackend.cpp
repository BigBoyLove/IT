#include "MetallNodeStorageBackend.hpp"

#include <functional>
//#include "../metall/include/metall/metall.hpp"
#include <metall/metall.hpp>
namespace rdf4cpp::rdf::storage::node::metall_node_storage {

    using Alloc = metall::manager::allocator_type<std::byte>;

    template<typename T>
    using metallMap = typename metall::container::map<identifier::NodeID,typename metall::offset_ptr<T>, std::less<>>;

    template<typename T>
    using metallReverseMap = typename metall::container::map<typename metall::offset_ptr<T>,identifier::NodeID, std::less<>>;

    template<typename T>
    using ptrMetallMap = metall::offset_ptr<metallMap<T>>;

    template<typename T>
    using ptrmMetallReverseMap = metall::offset_ptr<metallReverseMap<T>>;

    template<typename U>
    using allocMap = typename metall::manager::allocator_type<U>;

    template<typename U>
    using AllocTraitsMetallMap = std::allocator_traits<allocMap<U>>;

    template<typename U>
    void initStorage(ptrMetallMap<U>& ptrStorage, ptrmMetallReverseMap<U>& ptrReverseStorage, const Alloc& alloc){
        typename metall::manager::allocator_type<metallMap<U>> rebindAlloc = alloc;
        ptrStorage = AllocTraitsMetallMap<metallMap<U>>::allocate(rebindAlloc, 1);
        rebindAlloc.construct(ptrStorage, alloc);

        typename metall::manager::allocator_type<metallReverseMap<U>> rebindReverseAlloc = alloc;
        ptrReverseStorage = AllocTraitsMetallMap<metallReverseMap<U>>::allocate(rebindReverseAlloc, 1);
        rebindReverseAlloc.construct(ptrReverseStorage, alloc);
    }

MetallNodeStorageBackend::MetallNodeStorageBackend(const Alloc &al) : INodeStorageBackend(), alloc(al) {  // TODO why call INodeStorageBackend ?

        initStorage<LiteralBackend>(literal_storage,literal_storage_reverse, alloc);
        initStorage<BNodeBackend>(bnode_storage,bnode_storage_reverse, alloc);
        initStorage<IRIBackend>(iri_storage,iri_storage_reverse, alloc);
        initStorage<VariableBackend>(variable_storage,variable_storage_reverse, alloc);

    for (const auto &[id, iri] : NodeID::predefined_iris) {
        typename metall::manager::allocator_type<IRIBackend> rebind_alloc = alloc;  // cast the allocator to the type you require
        auto mem = rebind_alloc.allocate(1);                                       // request memory
        rebind_alloc.construct(mem, iri);                            // construct MetallIRIBackend at mem

        auto [iter, inserted_successfully] = iri_storage_reverse->emplace(metall::offset_ptr<IRIBackend>(mem), id);

        assert(inserted_successfully);
        iri_storage->insert({id, metall::offset_ptr<IRIBackend>(mem)});
    }
}
template<class Backend_t, bool create_if_not_present, class View_t, class Storage_t, class ReverseStorage_t, class NextIDFromView_func = void *>
inline identifier::NodeID lookup_or_insert_impl(View_t view, std::shared_mutex &mutex,const metall::offset_ptr<Storage_t> &storage,
                                                const metall::offset_ptr<ReverseStorage_t> &reverse_storage, const Alloc& alloc, const
                                                NextIDFromView_func next_id_func = nullptr) noexcept {
    std::shared_lock<std::shared_mutex> shared_lock{mutex};
    auto found = reverse_storage->find(view);
    if (found == reverse_storage->end()) {
        if constexpr (create_if_not_present) {
            shared_lock.unlock();
            std::unique_lock<std::shared_mutex> unique_lock{mutex};
            // update found (might have changed in the meantime)
            found = reverse_storage->find(view);
            if (found == reverse_storage->end()) {
                identifier::NodeID id = next_id_func(view);

                typename metall::manager::allocator_type<Backend_t> rebind_alloc = alloc;
                auto mem = rebind_alloc.allocate(1);
                rebind_alloc.construct(mem, view);

                auto [found, inserted_successfully] = reverse_storage->emplace(metall::offset_ptr<typename ReverseStorage_t::key_type::element_type>(mem), id);
                assert(inserted_successfully);

                storage->insert({id, metall::offset_ptr<typename ReverseStorage_t::key_type::element_type>(mem)});
                return id;
            } else {
                unique_lock.unlock();
                return found->second;
            }
        } else {
            return {};
        }
    } else {
        shared_lock.unlock();
        return found->second;
    }
}
identifier::NodeID MetallNodeStorageBackend::find_or_make_id(view::LiteralBackendView const &view) noexcept {
    return lookup_or_insert_impl<LiteralBackend, true>(
            view, literal_mutex_, literal_storage, literal_storage_reverse, alloc,
            [this]([[maybe_unused]] view::LiteralBackendView const &literal_view) {
                // TODO: actually use LiteralType (therefore, we will need literal_view)
                return identifier::NodeID{next_literal_id++, identifier::LiteralType::OTHER};
            });
}
identifier::NodeID MetallNodeStorageBackend::find_or_make_id(view::IRIBackendView const &view) noexcept {
    return lookup_or_insert_impl<IRIBackend, true>(
            view, iri_mutex_, iri_storage, iri_storage_reverse, alloc,
            [this]([[maybe_unused]] view::IRIBackendView const &view) {
                return next_iri_id++;
            });
}

identifier::NodeID MetallNodeStorageBackend::find_or_make_id(view::BNodeBackendView const &view) noexcept {
    return lookup_or_insert_impl<BNodeBackend, true>(
            view, bnode_mutex_, bnode_storage, bnode_storage_reverse, alloc,
            [this]([[maybe_unused]] view::BNodeBackendView const &view) {
                return next_bnode_id++;
            });
}
identifier::NodeID MetallNodeStorageBackend::find_or_make_id(view::VariableBackendView const &view) noexcept {
    return lookup_or_insert_impl<VariableBackend, true>(
            view, variable_mutex_, variable_storage, variable_storage_reverse, alloc,
            [this]([[maybe_unused]] view::VariableBackendView const &view) {
                return next_variable_id++;
            });
}

identifier::NodeID MetallNodeStorageBackend::find_id(const view::BNodeBackendView &view) const noexcept {
    return lookup_or_insert_impl<BNodeBackend, false>(
            view, bnode_mutex_, bnode_storage, bnode_storage_reverse, alloc);
}
identifier::NodeID MetallNodeStorageBackend::find_id(const view::IRIBackendView &view) const noexcept {
    return lookup_or_insert_impl<IRIBackend, false>(
            view, iri_mutex_, iri_storage, iri_storage_reverse, alloc);
}
identifier::NodeID MetallNodeStorageBackend::find_id(const view::LiteralBackendView &view) const noexcept {
    return lookup_or_insert_impl<LiteralBackend, false>(
            view, literal_mutex_, literal_storage, literal_storage_reverse, alloc);
}
identifier::NodeID MetallNodeStorageBackend::find_id(const view::VariableBackendView &view) const noexcept {
    return lookup_or_insert_impl<VariableBackend, false>(
            view, variable_mutex_, variable_storage, variable_storage_reverse, alloc);
}

view::IRIBackendView MetallNodeStorageBackend::find_iri_backend_view(identifier::NodeID id) const {
    std::shared_lock<std::shared_mutex> shared_lock{iri_mutex_};
    return view::IRIBackendView(*iri_storage->at(id));
}
view::LiteralBackendView MetallNodeStorageBackend::find_literal_backend_view(identifier::NodeID id) const {
    std::shared_lock<std::shared_mutex> shared_lock{literal_mutex_};
    return view::LiteralBackendView(*literal_storage->at(id));
}
view::BNodeBackendView MetallNodeStorageBackend::find_bnode_backend_view(identifier::NodeID id) const {
    std::shared_lock<std::shared_mutex> shared_lock{bnode_mutex_};
    return view::BNodeBackendView(*bnode_storage->at(id));
}
view::VariableBackendView MetallNodeStorageBackend::find_variable_backend_view(identifier::NodeID id) const {
    std::shared_lock<std::shared_mutex> shared_lock{variable_mutex_};
    return view::VariableBackendView(*variable_storage->at(id));
}
bool MetallNodeStorageBackend::erase_iri([[maybe_unused]] identifier::NodeID id) const {
    throw std::runtime_error("Deleting nodes is not implemented in MetallNodeStorageBackend.");
}
bool MetallNodeStorageBackend::erase_literal([[maybe_unused]] identifier::NodeID id) const {
    throw std::runtime_error("Deleting nodes is not implemented in MetallNodeStorageBackend.");
}
bool MetallNodeStorageBackend::erase_bnode([[maybe_unused]] identifier::NodeID id) const {
    throw std::runtime_error("Deleting nodes is not implemented in MetallNodeStorageBackend.");
}
bool MetallNodeStorageBackend::erase_variable([[maybe_unused]] identifier::NodeID id) const {
    throw std::runtime_error("Deleting nodes is not implemented in MetallNodeStorageBackend.");
}
}  // namespace rdf4cpp::rdf::storage::node::metall_node_storage