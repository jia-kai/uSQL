/* 
* @Author: BlahGeek
* @Date:   2014-11-30
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-11-30
*/

#include "./int.h"

using namespace usql;

std::unique_ptr<DataBase> IntDataType::load(const void * src) {
    int64_t v = *(const int64_t *)src;
    return std::unique_ptr<IntData>(new IntData(v));
}

REGISTER_DATATYPE(IntData)
