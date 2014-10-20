/*
 * $File: test_page_io.cpp
 * $Date: Mon Oct 20 09:34:53 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "./page_io_env.h"

TEST_F(PageIOTestEnv, test_alloc_one) {
    auto p = m_page_io->alloc();
    EXPECT_NE(0, p.id());
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}
