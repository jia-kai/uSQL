/*
 * $File: serializable.cpp
 * $Date: Sun Nov 23 18:14:34 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "./serializable.h"
#include <map>

using namespace usql;

namespace {
    using factory_map_t = std::map<Serializable::TypeID,
          Serializable::deserialize_factory_t>;
    factory_map_t *factory_map = nullptr;
}

size_t Serializable::get_serialize_length() {
    return sizeof(TypeID) + do_get_serialize_length();
}

void Serializable::serialize(void *dest_) {
    auto dest = static_cast<TypeID*>(dest_);
    *dest = type_id();
    do_serialize(dest + 1);
}

std::unique_ptr<Serializable> Serializable::deserialize(
        const void *data_, size_t length) {
    static THREAD_LOCAL bool recursive_call = false;
    usql_assert(!recursive_call,
            "recursive call to Serializable::deserialize, "
            "maybe subclass forget to implement deserialize?");
    recursive_call = true;
    usql_assert(length >= sizeof(TypeID));
    auto data = static_cast<const TypeID*>(data_);
    auto type_id = *data;
    auto iter = factory_map->find(*data);
    usql_assert(iter != factory_map->end(), "failed to find type_id %d",
            int(type_id));
    auto rst = iter->second(data + 1, length - sizeof(TypeID));
    recursive_call = false;
    return rst;
}

void Serializable::register_serializable(
        TypeID type_id, deserialize_factory_t factory) {
    if (!factory_map)
        factory_map = new factory_map_t;
    auto rst = factory_map->insert({type_id, factory});
    usql_assert(rst.second, "type id %d already exists", int(type_id));
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

