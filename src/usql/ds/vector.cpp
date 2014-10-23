/*
 * $File: vector.cpp
 * $Date: Thu Oct 23 21:48:55 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "./vector.h"

#include <type_traits>
using namespace usql;

/*
 * the vector is organized as full n-ary tree;
 * all data are stored in leaf nodes
 */


enum class VectorImpl::PageType {
    ROOT, INTERNAL, LEAF,

    // used when the tree contains only one node, so root also stores data
    ROOT_AS_LEAF    
};

struct VectorImpl::PageHeader {
    PageType type;

    //! number of bytes used in this page, not including the header
    size_t used_size;

    struct RootPageHeader {
        PageIO::page_id_t freelist_root;
        
        //! total number of data elements in each child
        size_t child_nr_data_elem;
    };

    struct InternalPageHeader {
        PageIO::page_id_t parent;

        //! total number of data elements in each child
        size_t child_nr_data_elem;

        //! offset of the first data element
        size_t data_offset;
    };

    struct LeafPageHeader {
        PageIO::page_id_t parent;

        //! offset of the first data element
        size_t data_offset;
    };

    union {
        RootPageHeader root;
        InternalPageHeader internal;
        LeafPageHeader leaf;
    } header;

    char _payload[0];

    void init_as_leaf_root() {
        type = PageType::ROOT_AS_LEAF;
        used_size = 0;
        header.root.freelist_root = 0;
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

} __attribute__((aligned(8)));


VectorImpl::VectorImpl(size_t elem_size, PageIO &page_io):
    PagedDataStructureBase(page_io),
    m_elem_size(align_elem_size(elem_size)), m_freelist(m_page_io),
    m_page_payload_max(m_page_io.page_size() - sizeof(PageHeader)),
    m_leaf_nr_data_slot(m_page_payload_max / m_elem_size),
    m_nonleaf_nr_child(m_page_payload_max / sizeof(PageIO::page_id_t))
{
    static_assert(std::is_standard_layout<PageHeader>::value, "bad");
    usql_assert(m_elem_size * 4 + sizeof(PageHeader) <= page_io.page_size());
    usql_assert(m_page_payload_max % sizeof(PageIO::page_id_t) == 0);
}

void VectorImpl::load(PageIO::page_id_t root, root_updator_t root_updator) {
    // alloc a new root or load from existing one
    if (!root) {
        root = m_page_io.alloc().id();
        PagedDataStructureBase::load(root, root_updator);
        root_updator(m_root);
        m_root.write<PageHeader>()->init_as_leaf_root();
    } else {
        PagedDataStructureBase::load(root, root_updator);
    }

    m_freelist.load(m_root.read<PageHeader>()->header.root.freelist_root,
        [this](const PageIO::Page &new_root) {
            m_root.write<PageHeader>()->header.root.freelist_root =
                new_root.id();
    });

    // find last page
    m_last_page = m_root;
    auto lastch_offset = m_page_payload_max - sizeof(PageIO::page_id_t);
    for (; ; ) {
        auto hdr = m_last_page.read<PageHeader>();
        if (hdr->type == PageType::ROOT_AS_LEAF || hdr->type == PageType::LEAF)
            break;
        usql_assert(hdr->used_size == m_page_payload_max);
        m_last_page = m_page_io.lookup(*hdr->child_page_id(lastch_offset));
    }
}

void VectorImpl::sanity_check() {
    usql_assert(0);
}

void VectorImpl::expand() {
    auto old_hdr = m_last_page.read<PageHeader>();
    std::vector<PageIO::Page> leaf_chain{m_page_io.alloc()};
    if (old_hdr->type == PageType::ROOT_AS_LEAF) {
        usql_assert(m_last_page.id() == m_root.id());
        expand_add_root(m_leaf_nr_data_slot, 1);
        return;
    }
    usql_assert(old_hdr->type == PageType::LEAF);
    PageIO::Page par = m_page_io.lookup(old_hdr->header.leaf.parent);
    const PageHeader* par_hdr;
    size_t leaf_chain_length = 1;
    while ((par_hdr = par.read<PageHeader>())->used_size
            == m_page_payload_max) {
        leaf_chain_length ++;
        if (par_hdr->type == PageType::ROOT) {
            expand_add_root(par_hdr->header.root.child_nr_data_elem *
                    m_nonleaf_nr_child, leaf_chain_length);
            return;
        }
        usql_assert(par_hdr->type == PageType::INTERNAL);
        par = m_page_io.lookup(par_hdr->header.internal.parent);
    }
    {
        size_t child_nr_data_elem;
        if (par_hdr->type == PageType::ROOT)
            child_nr_data_elem = par_hdr->header.root.child_nr_data_elem;
        else {
            child_nr_data_elem = par_hdr->header.internal.child_nr_data_elem;
            usql_assert(par_hdr->type == PageType::INTERNAL);
        }
        auto leaf_id = set_leaf_chain(
                par.id(),
                par_hdr->used_size / sizeof(PageIO::page_id_t) *
                child_nr_data_elem, leaf_chain_length);
        auto p = par.write<PageHeader>();
        *p->child_page_id(p->used_size) = leaf_id;
        p->used_size += sizeof(PageIO::page_id_t);
    }
}

void VectorImpl::expand_add_root(
        size_t root_nr_data_elem, size_t leaf_chain_length) {

    PageIO::Page new_root = m_page_io.alloc();

    auto ch1_id = set_leaf_chain(new_root.id(),
            root_nr_data_elem, leaf_chain_length);

    auto old_root_hdr = m_root.write<PageHeader>();

    // setup root
    {
        auto hdr = new_root.write<PageHeader>();
        hdr->type = PageType::ROOT;
        auto &&tr = hdr->header.root;
        tr.freelist_root = old_root_hdr->header.root.freelist_root;
        tr.child_nr_data_elem = root_nr_data_elem;
        hdr->used_size = sizeof(PageIO::page_id_t) * 2;
        hdr->child_page_id(0)[0] = m_root.id();
        hdr->child_page_id(0)[1] = ch1_id;
    }

    // update old root
    {
        usql_assert(old_root_hdr->type == PageType::ROOT ||
                old_root_hdr->type == PageType::ROOT_AS_LEAF);
        old_root_hdr->type = PageType::LEAF;
        auto &&tr = old_root_hdr->header.leaf;
        tr.parent = new_root.id();
        tr.data_offset = 0;
    }

    set_root(new_root);
}

PageIO::page_id_t VectorImpl::set_leaf_chain(
        PageIO::page_id_t par, size_t data_offset, size_t length) {
    usql_assert(length);
    auto leaf = m_page_io.alloc();
    PageHeader *hdr = leaf.write<PageHeader>();
    hdr->type = PageType::LEAF;
    hdr->used_size = 0;
    hdr->header.leaf.data_offset = data_offset;
    auto prev_par = &hdr->header.leaf.parent;
    m_last_page = leaf;
    size_t child_nr_data_elem = m_leaf_nr_data_slot;
    auto last_page_id = leaf.id();
    for (size_t i = 1; i < length; i ++) {
        auto internal = m_page_io.alloc();
        *prev_par = internal.id();
        hdr = internal.write<PageHeader>();
        hdr->type = PageType::INTERNAL;
        hdr->used_size = 0;
        hdr->header.internal.child_nr_data_elem = child_nr_data_elem;
        hdr->header.internal.data_offset = data_offset;
        prev_par = &hdr->header.internal.parent;
        last_page_id = internal.id();
    }
    *prev_par = par;
    return last_page_id;
}

std::pair<VectorImpl::idx_t, void*> VectorImpl::insert() {
    if (!m_freelist.empty()) {
        FreelistNode fptr = m_freelist.pop();
        PageIO::Page dest_page = m_page_io.lookup(fptr.page);
        auto hdr = dest_page.write<PageHeader>();
        size_t idx = fptr.slot;
        if (hdr->type == PageType::LEAF) {
            idx += hdr->header.leaf.data_offset;
        } else {
            usql_assert(hdr->type ==PageType::ROOT_AS_LEAF &&
                    fptr.page == m_root.id());
        }
        return {idx, hdr->payload(fptr.slot * m_elem_size)};
    }
    auto hdr = m_last_page.write<PageHeader>();
    if (hdr->used_size + m_elem_size > m_page_payload_max) {
        expand();
        hdr = m_last_page.write<PageHeader>();
    }
    auto ptr = hdr->payload(hdr->used_size);
    auto idx = hdr->used_size / m_elem_size;
    usql_assert(hdr->used_size % m_elem_size == 0);
    hdr->used_size += m_elem_size;
    if (hdr->type == PageType::LEAF) {
        idx += hdr->header.leaf.data_offset;
    } else {
        usql_assert(hdr->type ==PageType::ROOT_AS_LEAF);
    }
    return {idx, ptr};
}

void* VectorImpl::prepare_write(idx_t idx) {
    auto p = find_page(idx);
    return p.first.write<PageHeader>()->payload(p.second);
}

const void* VectorImpl::prepare_read(idx_t idx) {
    auto p = find_page(idx);
    return p.first.read<PageHeader>()->payload(p.second);
}

std::pair<PageIO::Page, size_t> VectorImpl::find_page(idx_t idx) {
    const PageHeader* hdr = m_last_page.read<PageHeader>();
    auto last_page_start = hdr->type == PageType::LEAF ?
        hdr->header.leaf.data_offset : 0;
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

