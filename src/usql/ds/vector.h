/*
 * $File: vector.h
 * $Date: Fri Oct 24 00:10:25 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "./base.h"
#include "./linked_stack.h"

#include <vector>

namespace usql {

class VectorImpl: private PagedDataStructureBase {
    public:
        using PagedDataStructureBase::root_updator_t;
        using idx_t = size_t;

        VectorImpl(size_t elem_size, PageIO &page_io);

        /*!
         * load root vector; if root == 0, a new root would be allocated
         */
        void load(PageIO::page_id_t root, root_updator_t root_updator) override;

        /*!
         * perform sanity check, and return tree height
         */
        size_t sanity_check_get_height();

        static constexpr size_t header_size() {
            return 5 * sizeof(size_t);
        }

        void erase(size_t idx);

    protected:
        std::pair<idx_t, void*> insert();

        void* prepare_write(idx_t idx);
        const void* prepare_read(idx_t idx);

    private:
        enum class PageType;
        struct PageHeader;

        bool m_init_done = false;
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
         * \param root_nr_data_elem number of data elements of the new tree root
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

        inline void check_init();

        size_t do_sanity_check(const PageIO::Page &root,
                PageIO::page_id_t expected_par,
                size_t depth, bool should_full,
                size_t expected_data_offset);

        /*!
         * find a page and internal offset for specified data idx
         * note that visiting beyond boundary causes std::range_error
         */
        inline std::pair<PageIO::Page, size_t> find_page(idx_t idx);

};


template<typename T>
class Vector final: public VectorImpl {
    public:
        Vector(PageIO &page_io):
            VectorImpl(sizeof(T), page_io)
        {}

        idx_t insert(const T &val) {
            auto r = VectorImpl::insert();
            *static_cast<T*>(r.second) = val;
            return r.first;
        }

        T read(size_t idx) {
            return *static_cast<const T*>(prepare_read(idx));
        }

        void write(size_t idx, const T &val) {
            *static_cast<T*>(prepare_write(idx)) = val;
        }
};

}   // namespace usql

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

