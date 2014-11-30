/* 
* @Author: BlahGeek
* @Date:   2014-11-30
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-11-30
*/

#include "./string.h"

using namespace usql;

std::unique_ptr<DataBase> StringDataType::load(const void * src) {
    std::string tmp(std::string(static_cast<const char *>(src)), 0, max_size);
    return std::unique_ptr<StringData>(new StringData(tmp));
}

REGISTER_DATATYPE(StringData)
