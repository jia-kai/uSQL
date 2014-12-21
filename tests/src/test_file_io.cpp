/* 
* @Author: BlahGeek
* @Date:   2014-12-21
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include <iostream>
#include <string.h>
#include "usql/file/page_io.h"

using namespace usql;

#include <gtest/gtest.h>
#include <unistd.h>

TEST(FileIOTest, save_and_open) {
    const char msg[] = "WTF!";
    const char filename[] = "data/fileio-test.usql";

    unlink(filename);

    auto page_io = std::make_unique<PageIO>(
                FileIO{filename, 8196});
    auto page = page_io->alloc();
    auto page_id = page.id();

    strcpy(page.write<char>(), msg);

    page_io = nullptr;
    page_io = std::make_unique<PageIO>(
                FileIO{filename, 8196});
    page = page_io->lookup(page_id);

    EXPECT_EQ(strcmp(page.read<char>(), msg), 0);
}
