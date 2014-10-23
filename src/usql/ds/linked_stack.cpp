/*
 * $File: linked_stack.cpp
 * $Date: Wed Oct 22 00:08:36 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "./linked_stack.h"

using namespace usql;

struct LinkedStackImpl::PageHeader {
    //! page for earlier items; 0 for the first page
    PageIO::page_id_t next_page = 0;

    //! number of bytes used in this page, not including this header
    size_t used_size = 0;

    char data[0];

    char *next_elem() {
        return data + used_size;
    }
};

LinkedStackImpl::LinkedStackImpl(size_t elem_size, PageIO &page_io):
    PagedDataStructureBase(page_io), m_elem_size(align_elem_size(elem_size))
{
    usql_assert(m_elem_size * 2 + sizeof(PageHeader) <= page_io.page_size());
}

size_t LinkedStackImpl::header_size() {
    return sizeof(PageHeader);
}

void* LinkedStackImpl::prepare_push() {
    if (!m_root.valid() ||
            m_root.read<PageHeader>()->used_size +
            m_elem_size + sizeof(PageHeader)
            > m_page_io.page_size()) {
        alloc_new_root();
    }
    auto hdr = m_root.write<PageHeader>();
    auto ret = hdr->next_elem();
    hdr->used_size += m_elem_size;
    return ret;
}

const void* LinkedStackImpl::prepare_pop() {
    auto hdr = m_root.write<PageHeader>();
    usql_assert(hdr->used_size >= m_elem_size);
    hdr->used_size -= m_elem_size;
    return hdr->next_elem();
}

void LinkedStackImpl::finish_pop() {
    auto hdr = m_root.read<PageHeader>();
    if (!hdr->used_size)
        dealloc_root();
}

bool LinkedStackImpl::empty() const {
    return !m_root.valid() || !m_root.read<PageHeader>()->used_size;
}

void LinkedStackImpl::alloc_new_root() {
    // if alloc uses freelist implemented by LinkedStack,
    // result would still be consistent; 
    // and used_size for old root would not be full, so dealloc could also work
    auto new_root = m_page_io.alloc();

    auto hdr = new_root.write<PageHeader>();
    hdr->next_page = m_root.id();
    hdr->used_size = 0;
    set_root(new_root);
}

void LinkedStackImpl::dealloc_root() {
    auto old_root = m_root;
    set_root(m_page_io.lookup(old_root.read<PageHeader>()->next_page));
    m_page_io.free(std::move(old_root));
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

