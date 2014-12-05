/* 
* @Author: BlahGeek
* @Date:   2014-11-30
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-05
*/

#include "./base.h"
#include <map>

using namespace usql;

LiteralData DataTypeBase::load(const void * src) const {
    auto ret = this->do_load(src);
    ret.datatype = this->type_id();
    return ret;
}

void DataTypeBase::dump(void * dest, const LiteralData & data) const {
    usql_assert(data.datatype == this->type_id(),
        "type id of data to dump does not match: %d vs %d", 
        int(data.datatype), this->type_id());
    this->do_dump(dest, data);
}
