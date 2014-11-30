/* 
* @Author: BlahGeek
* @Date:   2014-11-30
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-11-30
*/

#include "./base.h"
#include <map>

using namespace usql;

namespace {
    using compare_function_map_t = std::map<DataType,
        DataCmp::compare_function_t>;
    compare_function_map_t * compare_function_map = nullptr;
}

bool DataCmp::operator () (const DataBase & a, const DataBase & b) const {
    DataType typea = a.type_id();
    DataType typeb = b.type_id();
    usql_assert(typea == typeb, 
        "cannot compare two data with different typeid: %d <-> %d",
        int(typea), int(typeb));
    auto iter = compare_function_map->find(typea);
    usql_assert(iter != compare_function_map->end(),
        "failed to find compare function for typeid: %d", int(typea));
    return iter->second(&a, &b);
}

void DataCmp::register_datatype(DataType type_id, compare_function_t compare_f) {
    if(!compare_function_map)
        compare_function_map = new compare_function_map_t;
    auto rst = compare_function_map->insert({type_id, compare_f});
    usql_assert(rst.second, "type id %d already exists", int(type_id));
}
