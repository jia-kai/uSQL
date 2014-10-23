/*
 * $File: test_page_io.cpp
 * $Date: Tue Oct 21 23:39:17 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "./page_io_env.h"
#include "usql/ds/linked_stack.h"

#include <unordered_set>

TEST_F(PageIOTestEnv, test_alloc_one) {
    auto p = m_page_io->alloc();
    EXPECT_NE(0, p.id());
}

TEST_F(PageIOTestEnv, test_dealloc_reuse_one) {
    auto p = m_page_io->alloc();
    auto id = p.id();
    m_page_io->free(std::move(p));
    EXPECT_FALSE(p.valid());
    EXPECT_EQ(id, m_page_io->alloc().id());
}

TEST_F(PageIOTestEnv, test_dealloc_reuse_many) {
    constexpr size_t SIZE_FACTOR = 10;
    size_t nr_page = m_page_io->page_size() * SIZE_FACTOR,
           max_alloc = nr_page * sizeof(PageIO::page_id_t) /
               (m_page_io->page_size() - LinkedStackImpl::header_size()) + 1
               + nr_page;
    for (int test = 0; test < 10; test ++) {
        std::unordered_set<PageIO::page_id_t> used_id;
        std::vector<PageIO::Page> pages;
        for (size_t i = 0; i < nr_page; i ++) {
            pages.emplace_back(m_page_io->alloc());
            used_id.insert(pages.back().id());
        }

        EXPECT_EQ(nr_page, used_id.size());

        for (auto &&i: pages) {
            m_page_io->free(std::move(i));
            EXPECT_FALSE(i.valid());
        }
    }
    EXPECT_LE(m_page_io->file_io().get_meta().nr_page_allocated, max_alloc);
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}
