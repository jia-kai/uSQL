/*
 * $File: page_io_env.h
 * $Date: Thu Oct 23 23:58:48 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "usql/file/page_io.h"

using namespace usql;

#include <gtest/gtest.h>
#include <unistd.h>

class PageIOTestEnvBase : public ::testing::Test {
    protected:
        static int cnt;
};

template<size_t PAGE_SIZE>
class PageIOTestEnvTpl : public PageIOTestEnvBase {
    protected:
        std::string m_db_fname;
        std::unique_ptr<PageIO> m_page_io;

        void SetUp() override {
            m_db_fname = ssprintf("data/dbtest-%d.usql",
                    __sync_fetch_and_add(&cnt, 1));
            unlink(m_db_fname.c_str());
            m_page_io = std::make_unique<PageIO>(
                    FileIO{m_db_fname.c_str(), PAGE_SIZE});
        }

        void TearDown() override {
            m_page_io.reset();
            int v = unlink(m_db_fname.c_str());
            usql_assert(!v, "failed to delete tmp file %s: %m",
                    m_db_fname.c_str());
        }

        static constexpr size_t page_size() {
            return PAGE_SIZE;
        }
};

using PageIOTestEnv = PageIOTestEnvTpl<8192>;

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

