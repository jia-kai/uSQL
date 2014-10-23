/*
 * $File: vector.cpp
 * $Date: Fri Oct 24 00:09:29 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "./vector.h"

#include <type_traits>
using namespace usql;

/*
 * the vector is organized as full n-ary tree;
 * all data are stored in leaf nodes
 */


struct VectorImpl::PageHeader {
    //! number of bytes used in this page, not including the header
    size_t used_size;

    PageIO::page_id_t
        freelist_root,  //! valid for root page
        parent;         //! valid for other pages

    //! total number of data elements in each child; 0 for leaf nodes
    size_t child_nr_data_elem;

    //! offset of the first data element in the whole vector
    size_t data_offset;

    char _payload[0];

    void init_zero() {
        memset(this, 0, sizeof(PageHeader));
    }

    void* payload(size_t offset) {
        return _payload + offset;
    }

    const void* payload(size_t offset) const {
        return _payload + offset;
    }

    PageIO::page_id_t* child_page_id(size_t offset) {
        return static_cast<PageIO::page_id_t*>(payload(offset));
    }

    const PageIO::page_id_t* child_page_id(size_t offset) const {
        return static_cast<const PageIO::page_id_t*>(payload(offset));
    }

    bool is_leaf() const {
        return !child_nr_data_elem;
    }

    bool is_root() const {
        return !parent;
    }

} __attribute__((aligned(8)));


VectorImpl::VectorImpl(size_t elem_size, PageIO &page_io):
    PagedDataStructureBase(page_io),
    m_elem_size(align_elem_size(elem_size)), m_freelist(m_page_io),
    m_page_payload_max(m_page_io.page_size() - sizeof(PageHeader)),
    m_leaf_nr_data_slot(m_page_payload_max / m_elem_size),
    m_nonleaf_nr_child(m_page_payload_max / sizeof(PageIO::page_id_t))
{
    static_assert(std::is_standard_layout<PageHeader>::value, "bad");
    static_assert(header_size() == sizeof(PageHeader), "outdated header_size");
    usql_assert(m_elem_size * 4 + sizeof(PageHeader) <= page_io.page_size());
    usql_assert(m_page_payload_max % sizeof(PageIO::page_id_t) == 0);
}

void VectorImpl::load(PageIO::page_id_t root, root_updator_t root_updator) {
    m_init_done = true;
    // alloc a new root or load from existing one
    if (!root) {
        root = m_page_io.alloc().id();
        PagedDataStructureBase::load(root, root_updator);
        root_updator(m_root);
        m_root.write<PageHeader>()->init_zero();
    } else {
        PagedDataStructureBase::load(root, root_updator);
    }

    m_freelist.load(m_root.read<PageHeader>()->freelist_root,
        [this](const PageIO::Page &new_root) {
            m_root.write<PageHeader>()->freelist_root = new_root.id();
    });

    // find last page
    m_last_page = m_root;
    auto lastch_offset = m_page_payload_max - sizeof(PageIO::page_id_t);
    for (; ; ) {
        auto hdr = m_last_page.read<PageHeader>();
        if (hdr->is_leaf())
            break;
        usql_assert(hdr->used_size == m_page_payload_max);
        m_last_page = m_page_io.lookup(*hdr->child_page_id(lastch_offset));
    }
}

void VectorImpl::check_init() {
    usql_assert(m_init_done);
}

size_t VectorImpl::do_sanity_check(
        const PageIO::Page &root,
        PageIO::page_id_t expected_par,
        size_t depth, bool should_full,
        size_t expected_data_offset) {

    auto hdr = root.read<PageHeader>();
    if (!hdr->is_root())
        usql_assert(!hdr->freelist_root);
    usql_assert(hdr->data_offset == expected_data_offset);
    usql_assert(hdr->parent == expected_par);
    if (hdr->is_leaf()) {
        usql_assert(!hdr->child_nr_data_elem);
        usql_assert(hdr->used_size <= m_page_payload_max &&
                hdr->used_size % m_elem_size == 0);
        if (should_full)
            usql_assert(hdr->used_size + m_elem_size > m_page_payload_max);
        else
            usql_assert(root.id() == m_last_page.id());
        return depth + 1;
    }
    if (should_full)
        usql_assert(hdr->used_size == m_page_payload_max);
    size_t height = 0;
    constexpr size_t d = sizeof(PageIO::page_id_t);
    usql_assert(hdr->used_size % d == 0);
    for (size_t choff = 0, t = hdr->used_size; choff < t; choff += d) {
        auto cur_height = do_sanity_check(
                m_page_io.lookup(*hdr->child_page_id(choff)),
                root.id(),
                depth + 1, should_full || (choff + d < t),
                expected_data_offset);
        expected_data_offset += hdr->child_nr_data_elem;
        if (!height)
            height = cur_height;
        else {
            usql_assert(height == cur_height,
                    "child height differs: %zd %zd",
                    height, cur_height);
        }
    }
    return height;
}

size_t VectorImpl::sanity_check_get_height() {
    return do_sanity_check(m_root, 0, 0, false, 0);
}

void VectorImpl::expand() {
    std::vector<PageIO::Page> leaf_chain{m_page_io.alloc()};
    auto old_hdr = m_last_page.read<PageHeader>();
    if (old_hdr->is_root()) {
        usql_assert(m_last_page.id() == m_root.id());
        expand_add_root(m_leaf_nr_data_slot, 1);
        return;
    }
    PageIO::Page par = m_page_io.lookup(old_hdr->parent);
    const PageHeader* par_hdr;
    size_t leaf_chain_length = 1;
    while ((par_hdr = par.read<PageHeader>())->used_size
            == m_page_payload_max) {
        leaf_chain_length ++;
        if (par_hdr->is_root()) {
            auto data_offset = par_hdr->child_nr_data_elem * m_nonleaf_nr_child;
            expand_add_root(data_offset, leaf_chain_length);
            return;
        }
        par = m_page_io.lookup(par_hdr->parent);
    }
    auto data_offset = par_hdr->used_size / sizeof(PageIO::page_id_t) *
            par_hdr->child_nr_data_elem + par_hdr->data_offset;
    auto leaf_id = set_leaf_chain(
            par.id(), data_offset, leaf_chain_length);
    auto p = par.write<PageHeader>();
    *p->child_page_id(p->used_size) = leaf_id;
    p->used_size += sizeof(PageIO::page_id_t);
}

void VectorImpl::expand_add_root(
        size_t root_nr_data_elem, size_t leaf_chain_length) {

    PageIO::Page new_root = m_page_io.alloc();

    auto ch1_id = set_leaf_chain(new_root.id(),
            root_nr_data_elem, leaf_chain_length);

    auto old_root_hdr = m_root.write<PageHeader>();
    auto freelist_root = old_root_hdr->freelist_root;

    // update old root
    {
        old_root_hdr->parent = new_root.id();
        old_root_hdr->freelist_root = 0;
    }


    // setup root
    {
        auto hdr = new_root.write<PageHeader>();
        hdr->init_zero();
        hdr->freelist_root = freelist_root;
        hdr->child_nr_data_elem = root_nr_data_elem;
        hdr->used_size = sizeof(PageIO::page_id_t) * 2;
        hdr->child_page_id(0)[0] = m_root.id();
        hdr->child_page_id(0)[1] = ch1_id;
    }

    set_root(new_root);
}

PageIO::page_id_t VectorImpl::set_leaf_chain(
        PageIO::page_id_t par, size_t data_offset, size_t length) {
    usql_assert(length);
    auto leaf = m_page_io.alloc();
    PageHeader *hdr = leaf.write<PageHeader>();
    hdr->init_zero();
    hdr->data_offset = data_offset;
    m_last_page = leaf;
    size_t child_nr_data_elem = m_leaf_nr_data_slot;
    auto last_page_id = leaf.id();
    for (size_t i = 1; i < length; i ++) {
        auto internal = m_page_io.alloc();
        hdr = m_page_io.lookup(last_page_id).write<PageHeader>();
        hdr->parent = internal.id();
        hdr = internal.write<PageHeader>();
        hdr->init_zero();
        hdr->child_nr_data_elem = child_nr_data_elem;
        hdr->data_offset = data_offset;
        hdr->used_size = sizeof(PageIO::page_id_t);
        hdr->child_page_id(0)[0] = last_page_id;
        last_page_id = internal.id();
        child_nr_data_elem *= m_nonleaf_nr_child;
    }
    hdr->parent = par;
    return last_page_id;
}

std::pair<VectorImpl::idx_t, void*> VectorImpl::insert() {
    check_init();
    if (!m_freelist.empty()) {
        FreelistNode fptr = m_freelist.pop();
        PageIO::Page dest_page = m_page_io.lookup(fptr.page);
        auto hdr = dest_page.write<PageHeader>();
        size_t idx = fptr.slot + hdr->data_offset;
        return {idx, hdr->payload(fptr.slot * m_elem_size)};
    }
    auto hdr = m_last_page.write<PageHeader>();
    if (hdr->used_size + m_elem_size > m_page_payload_max) {
        expand();
        hdr = m_last_page.write<PageHeader>();
    }
    auto ptr = hdr->payload(hdr->used_size);
    auto idx = hdr->used_size / m_elem_size + hdr->data_offset;
    usql_assert(hdr->used_size % m_elem_size == 0);
    hdr->used_size += m_elem_size;
    return {idx, ptr};
}

void* VectorImpl::prepare_write(idx_t idx) {
    check_init();
    auto p = find_page(idx);
    return p.first.write<PageHeader>()->payload(p.second);
}

const void* VectorImpl::prepare_read(idx_t idx) {
    check_init();
    auto p = find_page(idx);
    return p.first.read<PageHeader>()->payload(p.second);
}

std::pair<PageIO::Page, size_t> VectorImpl::find_page(idx_t idx) {
    PageIO::Page page;
    const PageHeader* hdr = m_last_page.read<PageHeader>();
    if (idx < hdr->data_offset) {
        page = m_root;
        while (!(hdr = page.read<PageHeader>())->is_leaf()) {
            auto off = (idx - hdr->data_offset) / hdr->child_nr_data_elem *
                sizeof(PageIO::page_id_t);
            usql_assert(idx >= hdr->data_offset && off < hdr->used_size);
            page = m_page_io.lookup(*hdr->child_page_id(off));
        }
    } else {
        page = m_last_page;
    }
    size_t off = (idx - hdr->data_offset) * m_elem_size;
    if (off >= hdr->used_size) {
        throw std::range_error(ssprintf(
                    "attempt to access beyond vector: idx=%zd", idx));
    }
    return {page, off};
}

void VectorImpl::erase(size_t idx) {
    auto p = find_page(idx);
    m_freelist.push({p.first.id(), p.second / m_elem_size});
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

