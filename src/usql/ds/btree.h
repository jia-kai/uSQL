/*
 * $File: btree.h
 * $Date: Thu Apr 09 09:32:01 2015 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include <functional>
#include <vector>

#include "./base.h"

namespace usql {

template<class Key, class KeyLess = std::less<Key>>
class BTree: public PagedDataStructureBase {
    public:
        class Iterator;

        BTree(PageIO &page_io, size_t payload_size,
                const KeyLess &cmpkey = KeyLess());

        /*!
         * find the least entry that is not less than key
         */
        Iterator lookup(const Key &key, bool insert_on_missing);

        /*!
         * visit the payload of given key; insert on missing
         */
        void *visit(const Key &key);

        /*!
         * return whether the key exists
         */
        bool erase(const Key &key);

        static inline constexpr size_t internal_header_size();

        void sanity_check() const;

    private:

        enum class PageType;
        struct HeaderBase;
        struct InternalHeader;
        struct LeafHeader;
        struct LookupHistEntry;
        using Datapos = std::pair<PageIO::Page, uint32_t>;  // page, idx

        KeyLess m_cmpkey;
        size_t m_leaf_item_size,
               m_leaf_nr_data_slot, m_internal_nr_child,
               m_leaf_merge_thresh, m_internal_merge_thresh;
        // merge or redistribute when < thresh

        std::vector<LookupHistEntry> m_lookup_hist;

        /*!
         * perform lookup and store in m_lookup_hist
         * for internal nodes, item[idx] <= key < item[idx + 1]
         * for leaf node, item[idx - 1] < key <= item[idx]
         */
        std::pair<const LeafHeader*, uint32_t> do_lookup(const Key &key);

        /*!
         * \return i such that item[i] <= key < item[i+1]
         */
        template<class Hdr>
        int bsearch(const Hdr *hdr, const Key &key);

        Datapos split_last_lookup_leaf(const Key &new_key);

        void insert_internal(size_t hist_idx, const Key &key,
                PageIO::page_id_t chid);

        void setup_new_root(PageIO::Page &ch0, PageIO::Page &ch1,
                const Key &ch1_split);

        void erase_internal(size_t hist_idx);

        template<class Hdr>
        void redistribute_or_merge(size_t hist_idx);

        Datapos find_data_pos(
                const Key &key, bool insert_on_missing);

        void do_sanity_check(const PageIO::Page &root,
                const Key *lower, const Key *upper,
                PageIO::page_id_t &expected_next_leaf) const;
};

template<class Key, class KeyLess>
class BTree<Key, KeyLess>::Iterator {
    BTree &m_btree;
    PageIO::Page m_cur_page;
    uint32_t m_cur_offset, m_max_offset;

    Iterator(BTree &btree, PageIO::Page &page, uint32_t idx);
    friend class BTree;

    void load(const PageIO::Page &page);

    public:
        void recalc(const Key & key);
        const Key& key() const;
        void* payload();

        bool valid() const {
            return m_cur_offset < m_max_offset;
        }

        void next();
};

template<class Key, class Payload, class KeyLess = std::less<Key>>
class TypedBTree: public BTree<Key, KeyLess> {
    using BaseIter = typename BTree<Key, KeyLess>::Iterator;

    public:
        TypedBTree(PageIO &page_io, const KeyLess &cmpkey = KeyLess()):
            BTree<Key, KeyLess>(page_io, sizeof(Payload), cmpkey)
        {}

        class Iterator: public BaseIter {
            Iterator(const BaseIter &iter):
                BaseIter(iter)
            {}

            friend class TypedBTree;

            public:
                Payload& payload() {
                    return *static_cast<Payload*>(BaseIter::payload());
                }
        };

        Iterator lookup(const Key &key, bool insert_on_missing) {
            return {BTree<Key, KeyLess>::lookup(key, insert_on_missing)};
        }

        Payload& visit(const Key &key) {
            return *static_cast<Payload*>(BTree<Key, KeyLess>::visit(key));
        }
};

}   // namespace usql

#include "./btree_impl.h"

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

