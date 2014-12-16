/*
 * $File: test_vector.cpp
 * $Date: Wed Nov 05 19:56:38 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "./page_io_env.h"
#include "./utils.h"

#include "usql/ds/vector.h"

#include <algorithm>
#include <unordered_set>
#include <cmath>

constexpr size_t TREE_LEAF_BRANCH = 6;

using VectorTestEnvBase = PageIOTestEnvTpl<
    VectorImpl::header_size() + sizeof(int) * TREE_LEAF_BRANCH>;

namespace {
class VectorTestEnv : public VectorTestEnvBase {
    protected:
        PageIO::page_id_t m_vec_root = 0;
        std::unique_ptr<Vector<int>> m_vec;

        void SetUp() override {
            VectorTestEnvBase::SetUp();
            m_vec = std::make_unique<Vector<int>>(*m_page_io);
            m_vec->load(m_vec_root, [this](const PageIO::Page &root) {
                m_vec_root = root.id();
            });
        }

        void TearDown() override {
            m_vec.reset();
        }

        static constexpr size_t internal_branch() {
            return (page_size() - VectorImpl::header_size()) /
                sizeof(PageIO::page_id_t);
        }
};
}

TEST_F(VectorTestEnv, simple_insert) {
    int v = rand();
    auto idx = m_vec->insert(v);
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(m_vec->read(idx), v);
}

TEST_F(VectorTestEnv, range_err) {
    ASSERT_THROW(m_vec->read(0), std::range_error);
    ASSERT_THROW(m_vec->write(0, 0), std::range_error);
    ASSERT_THROW(m_vec->erase(0), std::range_error);
    m_vec->insert(0);
    ASSERT_THROW(m_vec->read(1), std::range_error);
    ASSERT_THROW(m_vec->write(1, 0), std::range_error);
    ASSERT_THROW(m_vec->erase(1), std::range_error);
}

TEST_F(VectorTestEnv, insert_only) {
    std::vector<int> truth, visiting_idx;
    const size_t HEIGHT = 10,
          NR = pow(internal_branch(), HEIGHT - 1) * TREE_LEAF_BRANCH - 3;
    for (size_t i = 0; i < NR; i ++) {
        visiting_idx.push_back(i);
        auto v = rand();
        ASSERT_EQ(i, m_vec->insert(v));
        truth.push_back(v);
        m_vec->sanity_check_get_height();
    }
    EXPECT_EQ(HEIGHT, m_vec->sanity_check_get_height());

    std::random_shuffle(visiting_idx.begin(), visiting_idx.end());

    for (auto i: visiting_idx)
        ASSERT_EQ(truth[i], m_vec->read(i));

    ASSERT_THROW(m_vec->read(NR), std::range_error);
    ASSERT_THROW(m_vec->read(NR + 1), std::range_error);
}

TEST_F(VectorTestEnv, random_opr) {
    for (int test_time = 0; test_time < 2; test_time ++) {
        std::vector<int> truth;
        std::vector<size_t> usable_idx;
        std::unordered_set<size_t> usable_idx_set;
        auto get_idx = [&](bool remove) {
            size_t p = rand() / (RAND_MAX + 1.0) * usable_idx.size(),
                   v = usable_idx[p];
            if (!remove)
                return v;
            usable_idx[p] = usable_idx.back();
            usable_idx.pop_back();
            auto iter = usable_idx_set.find(v);
            usql_assert(iter != usable_idx_set.end());
            usable_idx_set.erase(iter);
            return v;
        };
        const size_t HEIGHT = 13,
              NR_INSERT = pow(internal_branch(), HEIGHT - 1) * TREE_LEAF_BRANCH;
        while (usable_idx.size() < NR_INSERT) {
            if (!usable_idx.empty() && rand() <= RAND_MAX / 4) {
                // delete
                auto idx = get_idx(true);
                ASSERT_NE(0, truth[idx]);
                m_vec->erase(idx);
                truth[idx] = 0;
                m_vec->sanity_check_get_height();
                continue;
            }

            // insert
            int v = rand();
            v += !v;
            auto idx = m_vec->insert(v);
            if (idx >= truth.size()) {
                if (!test_time) {
                    ASSERT_EQ(truth.size(), idx);
                    truth.push_back(v);
                } else {
                    truth.resize(idx + 1, 0);
                    truth[idx] = v;
                }
            } else {
                ASSERT_EQ(0, truth[idx]);
                truth[idx] = v;
            }
            usable_idx.push_back(idx);
            usable_idx_set.insert(idx);
            m_vec->sanity_check_get_height();

            // read
            idx = get_idx(false);
            ASSERT_EQ(truth[idx], m_vec->read(idx));

            // write
            idx = get_idx(false);
            v = rand();
            v += !v;
            truth[idx] = v;
            m_vec->write(idx, v);
        }

        EXPECT_EQ(HEIGHT, m_vec->sanity_check_get_height());

        std::random_shuffle(usable_idx.begin(), usable_idx.end());

        for (auto i: usable_idx)
            ASSERT_EQ(truth[i], m_vec->read(i));

        ASSERT_EQ(NR_INSERT - 1,
                *std::max_element(usable_idx.begin(), usable_idx.end()));

        for (size_t i = 0; i < NR_INSERT; i ++)
            m_vec->erase(i);
    }
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

