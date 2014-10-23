/*
 * $File: base.cpp
 * $Date: Tue Oct 21 10:55:34 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "./base.h"

using namespace usql;

PagedDataStructureBase::PagedDataStructureBase(PageIO &page_io):
    m_page_io(page_io)
{
}

void PagedDataStructureBase::load(
        PageIO::page_id_t root, root_updator_t root_updator)
{
    m_root = m_page_io.lookup(root);
    m_root_updator = root_updator;
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

