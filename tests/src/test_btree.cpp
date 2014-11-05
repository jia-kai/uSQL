/*
 * $File: test_btree.cpp
 * $Date: Thu Nov 06 01:13:10 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "./page_io_env.h"
#include "./utils.h"

#include "usql/ds/btree.h"

#include <map>

constexpr size_t TREE_INTERNAL_BRANCH = 4;

namespace {

struct Key {
    int key;

    Key() = default;

    explicit Key(int n):
        key(n)
    {
    }
};

struct KeyCmp {
    bool operator () (const Key &a, const Key & b) const {
        return a.key < b.key;
    }
};

using val_t = int;
using Btree = TypedBTree<Key, val_t, KeyCmp>;

using BTreeTestEnvBase = PageIOTestEnvTpl<
    (Btree::internal_header_size() + sizeof(Key) + sizeof(PageIO::page_id_t)) *
    TREE_INTERNAL_BRANCH>;

class BTreeTestEnv : public BTreeTestEnvBase {
    protected:
        PageIO::page_id_t m_tree_root = 0;
        std::unique_ptr<Btree> m_tree;

        void SetUp() override {
            BTreeTestEnvBase::SetUp();
            m_tree = std::make_unique<Btree>(*m_page_io);
            m_tree->load(m_tree_root, [this](const PageIO::Page &root) {
                m_tree_root = root.id();
            });
        }

        void TearDown() override {
            m_tree.reset();
        }
};

/*!
 * map that allows sampling
 */
class RandMap {
    std::map<Key, val_t, KeyCmp> m_map;
    std::vector<Key> m_key;

    public:
        void assign(const Key &key, const val_t &val) {
            auto r = m_map.insert({key, val});
            if (r.second)
                m_key.push_back(key);
            else
                r.first->second = val;
        }

        const val_t& operator[] (const Key &key) const {
            auto iter = m_map.find(key);
            usql_assert(iter != m_map.end());
            return iter->second;
        }

        const Key& sample() {
            usql_assert(!m_key.empty());
            auto idx = randi(m_key.size());
            std::swap(m_key[idx], m_key.back());
            return m_key.back();
        }

        void remove_prev_sampled() {
            usql_assert(!m_key.empty());
            m_map.erase(m_key.back());
            m_key.pop_back();
        }

        size_t size() const {
            return m_map.size();
        }

        auto&& map() const {
            return m_map;
        }
};

} // annonymous namespace

TEST_F(BTreeTestEnv, simple_insert) {
    m_tree->visit(Key{123}) = 456;
    m_tree->sanity_check();
    EXPECT_EQ(456, m_tree->visit(Key{123}));
}

TEST_F(BTreeTestEnv, erase_after_insert) {
    RandMap check;
    const size_t NR = pow(TREE_INTERNAL_BRANCH, 7);

    auto check_read = [&](size_t i) {
        auto &&k = check.sample();
        auto v_check = check[k],
             v_get = m_tree->visit(k);
        usql_assert(v_check == v_get, "unequal: i=%zd expect=%d get=%d key=%d",
                i, v_check, v_get, k.key);
        return k;
    };

    for (size_t i = 0; i < NR; i ++) {
        // insert
        Key k{rand()};
        val_t v = rand();
        check.assign(k, v);
        m_tree->visit(k) = v;
        m_tree->sanity_check();

        check_read(i);
    }

    for (size_t i = 0; i < NR; i ++) {
        auto &&k = check_read(i);
        ASSERT_TRUE(m_tree->erase(k));
        m_tree->sanity_check();
        check.remove_prev_sampled();
    }
}

TEST_F(BTreeTestEnv, rand_opr) {
    RandMap check;
    const size_t NR_OPR = pow(TREE_INTERNAL_BRANCH, 10);
    for (size_t i = 0; i < NR_OPR; i ++) {
        bool do_insert = true;
        if (check.size()) {
            Key k{rand()};
            auto iter_expect = check.map().lower_bound(k);
            auto iter_get = m_tree->lookup(k, false);
            for (int t = randi(TREE_INTERNAL_BRANCH * 4); t >= 0; t --) {
                if (iter_expect == check.map().end()) {
                    ASSERT_FALSE(iter_get.valid());
                    break;
                }
                ASSERT_TRUE(iter_get.valid());
                ASSERT_EQ(iter_expect->first.key, iter_get.key().key);
                ASSERT_EQ(iter_expect->second, iter_get.payload());
                iter_get.next();
                iter_expect ++;
            }

            if (rand() <= RAND_MAX / 2) {
                // rand erase, do not insert
                do_insert = false;
                k = check.sample();
                ASSERT_TRUE(m_tree->erase(k));
                check.remove_prev_sampled();
            }
        }
        if (do_insert) {
            Key k{rand()};
            val_t v = rand();
            check.assign(k, v);
            m_tree->lookup(k, true).payload() = v;
        }

        if (rand() <= RAND_MAX / 5) {
            // check erase unexisting key
            Key k{rand()};
            if (check.map().find(k) == check.map().end()) {
                ASSERT_FALSE(m_tree->erase(k));
            }
        }

        if (rand() <= RAND_MAX / 5 && check.size()) {
            // overwrite existing entry
            auto &&k = check.sample();
            val_t v = rand();
            check.assign(k, v);
            m_tree->visit(k) = v;
        }

        m_tree->sanity_check();
    }

    while (check.size()) {
        auto &&k = check.sample();
        ASSERT_TRUE(m_tree->erase(k));
        check.remove_prev_sampled();
    }

    ASSERT_EQ(0, m_tree_root);
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

