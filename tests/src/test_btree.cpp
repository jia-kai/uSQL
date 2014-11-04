/*
 * $File: test_btree.cpp
 * $Date: Tue Nov 04 23:46:54 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "./page_io_env.h"

#include "usql/ds/btree.h"

constexpr size_t TREE_INTERNAL_BRANCH = 4;

struct Key {
    int key;

    bool operator < (const Key &rhs) const {
        return key < rhs.key;
    }

    Key() = default;

    explicit Key(int n):
        key(n)
    {
    }
};

using Btree = TypedBTree<Key, int>;

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

TEST_F(BTreeTestEnv, simple_insert) {
    m_tree->visit(Key{123}) = 456;
    EXPECT_EQ(456, m_tree->visit(Key{123}));
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}
