/*
 * $File: vector.h
 * $Date: Thu Oct 23 21:44:36 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "./base.h"
#include "./linked_stack.h"

#include <vector>

namespace usql {

class VectorImpl: private PagedDataStructureBase {
    enum class PageType;
    struct PageHeader;

    size_t m_elem_size;

    struct FreelistNode {
        PageIO::page_id_t page = 0;
        size_t slot = 0;   //! element index in the page

        FreelistNode() = default;
        FreelistNode(PageIO::page_id_t p, size_t s):
            page(p), slot(s)
        {}
    };

    LinkedStack<FreelistNode> m_freelist;
    PageIO::Page m_last_page;
    size_t m_page_payload_max, m_leaf_nr_data_slot, m_nonleaf_nr_child;

    /*!
     * expand the tree to make a last_page with empty slots
     * \param root_nr_data_elem number of data elements of current tree
     * \param leaf_chain_length number of nodes parallel to current tree
     */
    inline void expand();
    inline void expand_add_root(size_t root_nr_data_elem,
            size_t leaf_chain_length);

    /*!
     * setup leaf chain, and update m_last_page to the tail
     * \return head of leaf chain
     */
    inline PageIO::page_id_t set_leaf_chain(
            PageIO::page_id_t par, size_t data_offset, size_t length);

    public:
        using PagedDataStructureBase::root_updator_t;
        using idx_t = size_t;

        VectorImpl(size_t elem_size, PageIO &page_io);

        /*!
         * load root vector; if root == 0, a new root would be allocated
         */
        void load(PageIO::page_id_t root, root_updator_t root_updator) override;

        size_t size() const;

        void sanity_check();

    protected:
        std::pair<idx_t, void*> insert();

        void* prepare_write(idx_t idx);
        const void* prepare_read(idx_t idx);

        /*!
         * find a page and internal offset for specified data idx
         */
        inline std::pair<PageIO::Page, size_t> find_page(idx_t idx);
};

}   // namespace usql

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

