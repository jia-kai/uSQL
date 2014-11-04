/*
 * $File: btree_impl.h
 * $Date: Tue Nov 04 23:57:13 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#ifndef __HEADER_USQL_BTREE_IMPL__
#define __HEADER_USQL_BTREE_IMPL__

#include "./btree.h"

namespace usql {

#define DEF(before, after) \
    template<class Key, class KeyLess> \
    before BTree<Key, KeyLess>::after
#define CLS typename BTree<Key, KeyLess>

/* --------------- page type and header types ---------------*/
DEF(enum class, PageType) {
    INTERNAL, LEAF
};

#define HEADER_BASE \
    PageType type; \
    uint32_t nr_item

DEF(struct, HeaderBase) {
    HEADER_BASE;

    InternalHeader* as_internal() {
        return reinterpret_cast<InternalHeader*>(this);
    }

    const InternalHeader* as_internal() const {
        return const_cast<HeaderBase*>(this)->as_internal();
    }

    LeafHeader* as_leaf() {
        return reinterpret_cast<LeafHeader*>(this);
    }

    const LeafHeader* as_leaf() const {
        return const_cast<HeaderBase*>(this)->as_leaf();
    }
};

DEF(struct, InternalHeader) {
    HEADER_BASE;

    PageIO::page_id_t ch0;
    struct Item {
        Key key;
        PageIO::page_id_t ch;
    };
    Item _item[0];

    /*
     * a[ch0] < item[0].key
     * item[i].key <= a[item[i].ch] < item[i+1].key
     */

    Item& item(BTree *, int idx) {
        return _item[idx];
    }

    const Item& item(BTree *tree, int idx) const {
        return const_cast<InternalHeader*>(this)->item(tree, idx);
    }

    Item& insert(BTree *tree, uint32_t idx) {
        usql_assert(idx <= nr_item && nr_item < tree->m_internal_nr_child);
        Item *src = _item + idx, *dest = src + 1;
        move(dest, src, nr_item - idx);
        nr_item ++;
        return *src;
    }

    void erase(uint32_t idx) {
        usql_assert(nr_item);
        nr_item --;
        move(_item + idx, _item + idx + 1, nr_item - idx);
    }

    bool redistribute_with(BTree *tree, InternalHeader *rhs,
            Key &mid_key) {
        uint32_t sum = nr_item + rhs->nr_item;
        if (sum < tree->m_internal_merge_thresh * 2)
            return false;
        uint32_t tgt = sum / 2, delta = tgt - nr_item;
        usql_assert(tgt > nr_item && rhs->nr_item > delta &&
                tgt <= tree->m_internal_nr_child);
        _item[nr_item].key = mid_key;
        _item[nr_item].ch = rhs->ch0;
        nr_item ++;
        copy(_item + nr_item, rhs->_item, delta - 1);
        nr_item += delta - 1;
        mid_key = rhs->_item[delta].key;
        rhs->ch0 = rhs->_item[delta].ch;
        move(rhs->_item, &rhs->item(tree, delta), rhs->nr_item - delta);
        rhs->nr_item -= delta;
        return true;
    }

    void merge_from(BTree *tree, InternalHeader *rhs, const Key &mid_key) {
        usql_assert(nr_item + rhs->nr_item + 1 <= tree->m_internal_nr_child);
        _item[nr_item].key = mid_key;
        _item[nr_item].ch = rhs->ch0;
        copy(_item + nr_item + 1, rhs->_item, rhs->nr_item);
        nr_item += rhs->nr_item + 1;
    }

    static void copy(Item *dest, const Item *src, size_t nr) {
        memcpy(dest, src, sizeof(Item) * nr);
    }

    static void move(Item *dest, const Item *src, size_t nr) {
        memmove(dest, src, sizeof(Item) * nr);
    }

};

DEF(struct, LeafHeader) {
    HEADER_BASE;

    PageIO::page_id_t next_sibling;
    struct Item {
        Key key;
        char payload[0];

        Item *next(BTree *tree) {
            return reinterpret_cast<Item*>(reinterpret_cast<char*>(this) +
                    tree->m_leaf_item_size);
        }
    };
    char _item[0];

    Item& item(BTree *tree, int idx) {
        return *reinterpret_cast<Item*>(_item + idx * tree->m_leaf_item_size);
    }
    const Item& item(BTree *tree, int idx) const {
        return const_cast<LeafHeader*>(this)->item(tree, idx);
    }

    Item& insert(BTree *tree, uint32_t idx) {
        usql_assert(idx <= nr_item && nr_item < tree->m_leaf_nr_data_slot);
        auto src = &item(tree, idx);
        move(tree, src->next(tree), src, nr_item - idx);
        nr_item ++;
        return *reinterpret_cast<Item*>(src);
    }

    bool erase(BTree *tree, uint32_t idx, const Key &key) {
        usql_assert(nr_item);
        Item *dest = &item(tree, idx);
        if (tree->m_cmpkey(dest->key, key) || tree->m_cmpkey(key, dest->key))
            return false;
        nr_item --;
        move(tree, dest, dest->next(tree), nr_item - idx);
        return true;
    }

    bool redistribute_with(BTree *tree, LeafHeader *rhs, Key &mid_key) {
        auto sum = nr_item + rhs->nr_item;
        if (sum < tree->m_leaf_nr_data_slot)
            return false;
        auto tgt = sum / 2, delta = tgt - nr_item;
        usql_assert(tgt > nr_item && rhs->nr_item > delta &&
                tgt <= tree->m_leaf_nr_data_slot);
        copy(tree, &item(tree, nr_item), &rhs->item(tree, 0), delta);
        move(tree, &rhs->item(tree, 0), &rhs->item(tree, delta),
                rhs->nr_item - delta);
        nr_item += delta;
        rhs->nr_item -= delta;
        mid_key = rhs->item(tree, 0).key;
        return true;
    }

    void merge_from(BTree *tree, LeafHeader *rhs, const Key &/*mid_key*/) {
        next_sibling = rhs->next_sibling;
        usql_assert(nr_item + rhs->nr_item <= tree->m_leaf_nr_data_slot);
        copy(tree, &item(tree, nr_item), &rhs->item(tree, 0), rhs->nr_item);
        nr_item += rhs->nr_item;
    }

    static void copy(BTree *tree,
            Item *dest, const Item *src, size_t nr) {
        memcpy(dest, src, tree->m_leaf_item_size * nr);
    }

    static void move(BTree *tree,
            Item *dest, const Item *src, size_t nr) {
        memmove(dest, src, tree->m_leaf_item_size * nr);
    }
};

#undef HEADER_BASE

DEF(struct, LookupHistEntry) {
    PageIO::Page page;
    int idx;
};

/* ------------------------- methods ------------------ */

DEF(, BTree) (PageIO &page_io, size_t payload_size, const KeyLess &cmpkey):
    PagedDataStructureBase(page_io),
    m_cmpkey(cmpkey),
    m_leaf_item_size(sizeof(Key) + payload_size),
    m_leaf_nr_data_slot((page_io.page_size() - sizeof(LeafHeader)) /
            m_leaf_item_size),
    m_internal_nr_child((page_io.page_size() - sizeof(InternalHeader)) /
            sizeof(typename InternalHeader::Item)),
    m_leaf_merge_threh(m_leaf_nr_data_slot / 3 + 1),
    m_internal_merge_thresh(m_internal_nr_child / 3 + 1)
{
    static_assert(sizeof(InternalHeader) == internal_header_size(),
            "outdated internal_header_size()");
    static_assert(std::is_standard_layout<LeafHeader>::value &&
            std::is_standard_layout<InternalHeader>::value,
            "bad headers");
    usql_assert(std::max(sizeof(LeafHeader), sizeof(InternalHeader)) <
            page_io.page_size());
    usql_assert(m_leaf_nr_data_slot >= 4 && m_internal_nr_child >= 4);
}

DEF(template<class Hdr> int, bsearch) (const Hdr *hdr, const Key &key) {
    uint32_t left = 0, right = hdr->nr_item;
    // the next item after last is thought to be infinity
    // i exists in [left, right] such that a[i] > key, find such minimal i
    while (left < right) {
        int mid = (left + right) / 2;
        bool cond = !m_cmpkey(key, hdr->item(this, mid).key); // a[mid] <= key
        left = cond ? mid + 1 : left;
        right = cond ? right : mid;
    }
    return int(left) - 1;
}

DEF(const CLS::LeafHeader*, do_lookup) (const Key &key) {
    m_lookup_hist.clear();
    PageIO::Page node = m_root;
    for (; ; ) {
        auto bhdr = node.read<HeaderBase>();
        if (bhdr->type == PageType::LEAF) {
            auto lhdr = bhdr->as_leaf();
            int idx = bsearch(lhdr, key);
            if (idx < 0 || m_cmpkey(lhdr->item(this, idx).key, key)) {
                idx ++;
            }
            m_lookup_hist.push_back({node, idx});
            return lhdr;
        }
        auto ihdr = bhdr->as_internal();
        int idx = bsearch(ihdr, key);
        m_lookup_hist.push_back({node, idx});
        node = m_page_io.lookup(ihdr->item(this, idx).ch);
    }
}

DEF(CLS::Iterator, lookup) (const Key &key, bool insert_on_missing) {
    auto pos = find_data_pos(key, insert_on_missing);
    return {*this, pos.first, pos.second};
}

DEF(void*, visit) (const Key &key) {
    auto pos = find_data_pos(key, true);
    return pos.first.template write<LeafHeader>()
        ->item(this, pos.second).payload;
}

DEF(CLS::Datapos, find_data_pos) (
        const Key &key, bool insert_on_missing) {
    auto lhdr = do_lookup(key);
    uint32_t idx = m_lookup_hist.back().idx;
    PageIO::Page tp = m_lookup_hist.back().page;
    if (insert_on_missing && (idx >= lhdr->nr_item ||
                m_cmpkey(lhdr->item(this, idx).key, key))) {
        if (lhdr->nr_item == m_leaf_nr_data_slot)
            std::tie(tp, idx) = split_last_lookup_leaf(key);
        else
            tp.write<LeafHeader>()->insert(this, idx).key = key;
    }
    return {tp, idx};
}

DEF(CLS::Datapos, split_last_lookup_leaf) (const Key &new_key) {
    // for leaf, a[idx] > new_key and a[idx - 1] < new_key
    uint32_t idx = m_lookup_hist.back().idx, mid = m_leaf_nr_data_slot / 2;
    PageIO::Page old_page = m_lookup_hist.back().page,
        new_page = m_page_io.alloc();
    auto old_hdr = old_page.write<LeafHeader>(),
         new_hdr = new_page.write<LeafHeader>();
    usql_assert(
            (!idx || m_cmpkey(old_hdr->item(this, idx - 1).key, new_key)) &&
            m_cmpkey(new_key, old_hdr->item(this, idx).key));
    new_hdr->nr_item = m_leaf_nr_data_slot - mid;
    old_hdr->nr_item = mid;
    new_hdr->type = PageType::LEAF;
    new_hdr->next_sibling = old_hdr->next_sibling;
    old_hdr->next_sibling = new_page.id();
    LeafHeader::copy(this, &new_hdr->item(this, 0), &old_hdr->item(this, mid),
            new_hdr->nr_item);
    if (m_lookup_hist.size() == 1) {
        setup_new_root(old_page, new_page, new_hdr->item(this, 0).key);
    } else {
        insert_internal(m_lookup_hist.size() - 2,
                new_hdr->item(this, 0).key, new_page.id());
    }
    if (idx <= mid) {
        old_hdr = old_page.write<LeafHeader>();
        old_hdr->insert(this, idx).key = new_key;
        return {old_page, idx};
    }
    idx -= mid;
    new_hdr = new_page.write<LeafHeader>();
    new_hdr->insert(this, idx).key = new_key;
    return {new_page, idx};
}

DEF(void, insert_internal) (size_t hist_idx, const Key &key,
        PageIO::page_id_t chid)  {
    // for internal nodes, a[idx] < key and a[idx + 1] > key
    // a[chid] >= key
    auto &&hist = m_lookup_hist[hist_idx];
    auto hdr = hist.page.template write<InternalHeader>();
    uint32_t idx = hist.idx + 1; // actual idx of the new slot
    usql_assert(
            (!idx || m_cmpkey(hdr->item(this, idx - 1).key, key)) &&
            (idx == hdr->nr_item || m_cmpkey(key, hdr->item(this, idx).key)));
    if (hdr->nr_item < m_internal_nr_child) {
        auto &&item = hdr->insert(this, idx);
        item.key = key;
        item.ch = chid;
        return;
    }

    // already full, split node

    usql_assert(hdr->nr_item == m_internal_nr_child);
    uint32_t mid_idx = (m_internal_nr_child + 1) / 2;
    auto new_page = m_page_io.alloc();
    hdr = hist.page.template write<InternalHeader>();
    auto new_hdr = new_page.template write<InternalHeader>();
    new_hdr->type = PageType::INTERNAL;
    hdr->nr_item = mid_idx;
    new_hdr->nr_item = m_internal_nr_child - mid_idx;

    // assume hdr->insert(this, idx) is already finished and process the result
    // array here
    Key mid_key;
    if (mid_idx == idx) {
        mid_key = key;
        new_hdr->ch0 = chid;
        InternalHeader::copy(&new_hdr->item(this, 0), &hdr->item(this, mid_idx),
                new_hdr->nr_item);
    } else if (mid_idx < idx) {
        mid_key = hdr->item(this, mid_idx).key;
        new_hdr->ch0 = hdr->item(this, mid_idx).ch;
        auto before = idx - mid_idx - 1;
        InternalHeader::copy(
                &new_hdr->item(this, 0), &hdr->item(this, mid_idx + 1), before);
        new_hdr->_item[before].key = key;
        new_hdr->_item[before].ch = chid;
        InternalHeader::copy(
                &new_hdr->item(this, before + 1), &hdr->item(this, idx),
                new_hdr->nr_item - before - 1);
    } else {
        mid_key = hdr->item(this, mid_idx - 1).key;
        new_hdr->ch0 = hdr->item(this, mid_idx - 1).ch;
        InternalHeader::copy(&new_hdr->item(this, 0), &hdr->item(this, mid_idx),
                new_hdr->nr_item);
        hdr->nr_item --;
        auto &&dest = hdr->insert(this, idx);
        dest.key = key;
        dest.ch = chid;
    }

    if (!hist_idx) {
        setup_new_root(hist.page, new_page, mid_key);
    } else {
        insert_internal(hist_idx - 1, mid_key, new_page.id());
    }
}

DEF(bool, erase) (const Key &key) {
    do_lookup(key);
    auto &&hist = m_lookup_hist.back();
    auto hdr = hist.page.template write<LeafHeader>();
    if (!hdr->erase(this, hist.idx, key))
        return false;
    if (m_lookup_hist.size() == 1 || hdr->nr_item >= m_leaf_merge_threh)
        return true;
    redistribute_or_merge<LeafHeader>(m_lookup_hist.size() - 1);
    return true;
}

DEF(template<class Hdr> void, redistribute_or_merge) (size_t hist_idx) {
    usql_assert(hist_idx);
    auto hdr = m_lookup_hist[hist_idx].page.template write<Hdr>();
    auto par_hdr = m_lookup_hist[hist_idx - 1].page.
        template write<InternalHeader>();
    auto idx = m_lookup_hist[hist_idx - 1].idx;
    Hdr *sib_prev = nullptr, *sib_next = nullptr;
    Key &key_prev = par_hdr->item(this, idx).key,
        &key_next = par_hdr->item(this, idx + 1).key;
    PageIO::Page sib_prev_page, sib_next_page;
    if (idx >= 0) {
        int chid = par_hdr->item(this, idx - 1).ch;
        usql_assert(chid);
        sib_prev_page = m_page_io.lookup(chid);
        sib_prev = sib_prev_page.write<Hdr>();
        if (sib_prev->redistribute_with(this, hdr, key_prev))
            return;
    }
    if (idx + 1 < int(par_hdr->nr_item)) {
        sib_next_page = m_page_io.lookup(par_hdr->item(this, idx + 1).ch);
        sib_next = sib_next_page.write<Hdr>();
        if (hdr->redistribute_with(this, sib_next, key_next))
            return;
    }

    if (sib_prev) {
        sib_prev->merge_from(this, hdr, key_prev);
        m_page_io.free(std::move(m_lookup_hist[hist_idx].page));
        par_hdr->erase(idx);
    } else {
        usql_assert(sib_next);
        hdr->merge_from(this, sib_next, key_next);
        m_page_io.free(std::move(sib_next_page));
        par_hdr->erase(idx + 1);
    }

    if (par_hdr->nr_item < m_internal_merge_thresh) {
        // propagate merging

        if (hist_idx >= 2)
            redistribute_or_merge<InternalHeader>(hist_idx - 1);
        else {
            // parent is root
            if (!par_hdr->nr_item) {
                if (sib_prev)
                    set_root(sib_prev_page);
                else
                    set_root(m_lookup_hist[hist_idx].page);
                m_page_io.free(std::move(m_lookup_hist[hist_idx - 1].page));
            }
        }
    }
}

DEF(void, setup_new_root) (PageIO::Page &ch0, PageIO::Page &ch1,
        const Key &ch1_split) {
    auto root_page = m_page_io.alloc();
    auto hdr = root_page.template write<InternalHeader>();
    hdr->type = PageType::INTERNAL;
    hdr->nr_item = 1;
    hdr->ch0 = ch0.id();
    hdr->item(this, 0).key = ch1_split;
    hdr->item(this, 0).ch = ch1.id();
    set_root(root_page);
}

/* ------------------------- Iterator ------------------ */

DEF(, Iterator::Iterator) (BTree &btree, PageIO::Page &page, uint32_t idx):
    m_btree(btree), m_cur_page(page)
{
    load(page);
    m_cur_offset += idx * btree.m_leaf_item_size;
}

DEF(void, Iterator::load) (const PageIO::Page &page) {
    auto hdr = page.read<HeaderBase>();
    usql_assert(hdr->type == PageType::LEAF);
    m_cur_page = page;
    m_cur_offset = sizeof(LeafHeader);
    m_max_offset = m_cur_offset + hdr->nr_item * m_btree.m_leaf_item_size;
}

DEF(const Key&, Iterator::key) () const {
    usql_assert(valid());
    return reinterpret_cast<const typename LeafHeader::Item*>(
            m_cur_page.read<char>() + m_cur_offset)->key;
}

DEF(void*, Iterator::payload) () {
    usql_assert(valid());
    return reinterpret_cast<typename LeafHeader::Item*>(
            m_cur_page.write<char>() + m_cur_offset)->payload;
}

DEF(void, Iterator::next) () {
    if (!valid())
        return;
    m_cur_offset += m_btree.m_leaf_item_size;
    if (m_cur_offset >= m_max_offset) {
        auto hdr = m_cur_page.read<LeafHeader>();
        if (!hdr->next_sibling)
            return;
        load(m_btree.m_page_io.lookup(hdr->next_sibling));
    }
}

#undef DEF
#undef CLS

}   // namespace usql

#endif // __HEADER_USQL_BTREE_IMPL__

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}
