#ifndef __usql_index_h__
#define __usql_index_h__ value

#include "../ds/btree.h"
#include "./table.h"
#include "../datatype/base.h"
#include <limits>

using namespace usql;

namespace usql {

template <typename T>
class KeyWithRowID {
public:
    T key;
    rowid_t rowid;

    KeyWithRowID() = default;
    KeyWithRowID(T k): key(k), rowid(-1) {}
    KeyWithRowID(T k, rowid_t r): key(k), rowid(r) {}

};

template <typename T>
struct KeyWithRowIDCmp {
    bool operator () (const KeyWithRowID<T> & a, const KeyWithRowID<T> & b) {
        if(a.key != b.key)
            return a.key < b.key;
        return a.rowid < b.rowid;
    }
};

class IndexBase {
public:
    enum class BoundType {
        DISABLE, INCLUDE, EXCLUDE
    };

    virtual ~IndexBase() = default;

    std::set<rowid_t> find() {
        return this->find(BoundType::DISABLE, LiteralData(),
                          BoundType::DISABLE, LiteralData());
    }
    std::set<rowid_t> find(const LiteralData & val) {
        return this->find(BoundType::INCLUDE, val,
                          BoundType::INCLUDE, val);
    }
    virtual std::set<rowid_t> find(BoundType lower_bound_type,
                                   const LiteralData& lower_bound,
                                   BoundType upper_bound_type,
                                   const LiteralData& upper_bound) = 0;

    virtual bool erase(const LiteralData& key, rowid_t rowid) = 0;
    virtual void insert(const LiteralData&key, rowid_t rowid) = 0;
};

template <typename T>
class Index: public IndexBase {
public:
    using hash_t = std::function<T(const LiteralData &)>;

protected:
    using IndexTree = BTree<KeyWithRowID<T>, KeyWithRowIDCmp<T>>;
    std::unique_ptr<IndexTree> index_tree = nullptr;

    hash_t hash = nullptr;

public:
    Index(hash_t hash, 
          PageIO &page_io, PageIO::page_id_t root, 
          PagedDataStructureBase::root_updator_t root_updator):
    hash(hash) {
        index_tree = std::make_unique<IndexTree>(page_io, 0);
        index_tree->load(root, root_updator);
    }

    std::set<rowid_t> find(BoundType lower_bound_type,
                           const LiteralData& lower_bound,
                           BoundType upper_bound_type,
                           const LiteralData& upper_bound) override {

        T lower_key = (lower_bound_type == BoundType::DISABLE) ?
            (std::numeric_limits<T>::min()) : hash(lower_bound);
        T upper_key = hash(upper_bound);

        std::set<rowid_t> ret;

        auto it = index_tree->lookup(
            KeyWithRowID<T>(lower_key), false);

        // if exclude lower bound, skip through it
        if(lower_bound_type == BoundType::EXCLUDE) {
            while(true) {
                if(!it.valid())
                    return ret;
                if(it.key().key > lower_key)
                    break;
                it.next();
            }
        }

        while(true) {
            if(!it.valid())
                return ret;
            if(upper_bound_type != BoundType::DISABLE && 
                it.key().key >= upper_key)
                break;
            ret.insert(it.key().rowid);
            it.next();
        }

        if(upper_bound_type == BoundType::INCLUDE) {
            while(true) {
                if(!it.valid())
                    return ret;
                if(it.key().key > upper_key)
                    break;
                ret.insert(it.key().rowid);
                it.next();
            }
        }

        return ret;
    }

    bool erase(const LiteralData& key, rowid_t rowid) override {
        return index_tree->erase(KeyWithRowID<T>(hash(key), rowid));
    }
    void insert(const LiteralData&key, rowid_t rowid) override {
        index_tree->lookup(KeyWithRowID<T>(hash(key), rowid), true);
    }
};

}


#endif
