/*
 * $File: linked_stack.h
 * $Date: Tue Oct 21 11:31:43 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "./base.h"

#include <functional>
#include <type_traits>

namespace usql {

/*!
 * stack implemented by linked list, stored in pages
 */
class LinkedStackImpl: public PagedDataStructureBase {
    struct PageHeader;

    void alloc_new_root();
    void dealloc_root();

    protected:
        size_t m_elem_size;

        void* prepare_push();

        void finish_push() {
        }

        const void* prepare_pop();
        void finish_pop();

    public:
        LinkedStackImpl(size_t elem_size, PageIO &page_io);

        bool empty() const;

        static size_t header_size();
};

class DynSizeLinkedStack final : public LinkedStackImpl {
    public:
        using LinkedStackImpl::LinkedStackImpl;

        void push(const void *data) {
            memcpy(prepare_push(), data, m_elem_size);
            finish_push();
        }

        void pop(void *data) {
            memcpy(data, prepare_pop(), m_elem_size);
            finish_pop();
        }
};

template<class T>
class LinkedStack final : public LinkedStackImpl {
    static_assert(std::is_standard_layout<T>::value,
            "must have standard layout");

    public:
        LinkedStack(PageIO &page_io):
            LinkedStackImpl(sizeof(T), page_io)
        {}

        LinkedStack(PageIO &page_io, 
                PageIO::page_id_t root, root_updator_t root_updator):
            LinkedStack(page_io)
        {
            load(root, root_updator);
        }

        void push(const T &val) {
            *static_cast<T*>(prepare_push()) = val;
            finish_push();
        }

        T pop() {
            T ret = *static_cast<const T*>(prepare_pop());
            finish_pop();
            return ret;
        }
};

}   // namespace usql

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

