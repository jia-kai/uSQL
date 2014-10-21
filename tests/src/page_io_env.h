/*
 * $File: page_io_env.h
 * $Date: Tue Oct 21 09:54:26 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "usql/page_io.h"

using namespace usql;

#include <gtest/gtest.h>
#include <unistd.h>

class PageIOTestEnv: public ::testing::Test {
    static int cnt;
    protected:
        std::string m_db_fname;
        std::unique_ptr<PageIO> m_page_io;

        void SetUp() override {
            m_db_fname = ssprintf("data/dbtest-%d.usql",
                    __sync_fetch_and_add(&cnt, 1));
            unlink(m_db_fname.c_str());
            m_page_io = std::make_unique<PageIO>(
                    FileIO{m_db_fname.c_str()});
        }

        void TearDown() override {
            m_page_io.reset();
            int v = unlink(m_db_fname.c_str());
            usql_assert(!v, "failed to delete tmp file %s: %m",
                    m_db_fname.c_str());
        }
};

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

