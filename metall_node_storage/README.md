I've put together an CMake setup where you can work in. 

I changed the things listed below:
- moved your classes from namespace rdf4cpp::rdf::storage::node::default_node_storage to rdf4cpp::rdf::storage::node::metall_node_storage
- renamed DefaultNodeStorageBackend to MetallNodeStorageBackend
- removed the allocator template parameter from MetallNodeStorageBackend. This is a small change in the task. It is enough if you complete the non-templated MetallNodeStorageBackend.
- put things into a CMake library metall_node_storage
- created dummy executables for 01_store_nodes and 02_load_nodes

You might need to install, besides libboost-all-dev also libserd-dev:
```shell
sudo apt install -y libboost-all-dev libserd-dev
```
