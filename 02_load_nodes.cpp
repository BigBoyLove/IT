#include <iostream>
#include <metall/metall.hpp>
#include <rdf4cpp/rdf.hpp>

#include <MetallNodeStorageBackend.hpp>
int main() {
    std::cout << "Hello, World!" << std::endl;

    std::string storage_path{"/tmp/metall_test"};

    metall::manager storage_manager{metall::create_only, storage_path.c_str()};

    using namespace rdf4cpp::rdf::storage::node;

    auto *nodestore_backend = storage_manager.find_or_construct<metall_node_storage::MetallNodeStorageBackend>("node_store")(storage_manager.get_allocator());

    NodeStorage node_storage = NodeStorage::register_backend(nodestore_backend);
    NodeStorage::default_instance(node_storage);

    using namespace rdf4cpp::rdf;

    return 0;
}
