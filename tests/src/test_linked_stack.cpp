/*
 * $File: test_linked_stack.cpp
 * $Date: Tue Oct 21 10:49:49 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "./page_io_env.h"
#include "usql/ds/linked_stack.h"

TEST_F(PageIOTestEnv, test_linked_stack) {
    PageIO::page_id_t root_id = 0;
    auto root_updator = [&](const PageIO::Page &r) {
        root_id = r.id();
    };
    LinkedStack<int> stack(*m_page_io, 0, root_updator);
    std::vector<int> stack_check;

    size_t nr_item = m_page_io->page_size() * 5;

    for (int test = 0; test < 5; test ++) {
        EXPECT_TRUE(stack.empty() && stack_check.empty());
        for (size_t i = 0; i < nr_item; i ++) {
            int v = rand();
            stack.push(v);
            stack_check.push_back(v);
        }

        for (size_t i = 0; i < nr_item; i ++) {
            auto expect = stack_check.back();
            stack_check.pop_back();
            EXPECT_EQ(expect, stack.pop());
        }
    }
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

