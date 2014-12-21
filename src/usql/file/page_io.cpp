/*
 * $File: page_io.cpp
 * $Date: Sun Dec 21 23:52:51 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "page_io.h"
#include "../ds/linked_stack.h"

using namespace usql;

PageIO::PageIO(FileIO &&fio):
    m_fio(std::move(fio))
{
    auto root_updator = [this](const Page &new_root) {
        m_fio.update_meta_start().page_freelist_root = new_root.id();
        m_fio.update_meta_finish();
    };
    m_freelist = std::make_unique<LinkedStack<page_id_t>>(
            *this, m_fio.get_meta().page_freelist_root,
                root_updator);
}

PageIO::~PageIO() {
}

PageIO::Page PageIO::alloc() {
    if (!m_freelist->empty())
        return lookup(m_freelist->pop());
    auto &&old_meta = m_fio.get_meta();
    if (old_meta.nr_page_allocated == old_meta.nr_page) {
        size_t new_nr = old_meta.nr_page;
        if (old_meta.nr_page < 1000)
            new_nr *= 2;
        else
            new_nr += new_nr / 4;
        m_fio.extend_size(new_nr);
    }
    size_t idx = m_fio.update_meta_start().nr_page_allocated ++;
    m_fio.update_meta_finish();
    return lookup(idx);
}

void PageIO::free(Page &&page) {
    if (page.valid()) {
        m_freelist->push(page.m_id);
        page.m_id = 0;
    }
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

