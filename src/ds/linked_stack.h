/*
 * $File: linked_stack.h
 * $Date: Mon Oct 20 09:25:44 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "../page_io.h"
#include <functional>
#include <type_traits>

/*!
 * stack implemented by linked list, stored in pages
 */
class LinkedStackImpl {
    public:
        using root_updator_t = std::function<void(const PageIO::Page&)>;

        LinkedStackImpl(size_t elem_size, PageIO &page_io,
                const PageIO::Page &root, root_updator_t root_updator);

        bool empty() const;

        /*!
         * whether the last page should be deallocated if the whole linked
         * stack is empty
         */
        void set_dealloc_last(bool flag) {
            m_dealloc_last = flag;
        }

    protected:
        size_t m_elem_size;

        void* prepare_push();

        void finish_push() {
            m_root.write_finish();
        }

        const void* prepare_pop();
        void finish_pop();

    private:
        bool m_dealloc_last = true;
        PageIO &m_page_io;
        PageIO::Page m_root;
        root_updator_t m_root_updator;

        struct PageHeader;

        void alloc_new_root();
        void dealloc_root();

};

class DynamicLinkedStack: public LinkedStackImpl {
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
class LinkedStack: public LinkedStackImpl {
    static_assert(std::is_standard_layout<T>::value,
            "must have standard layout");

    public:
        LinkedStack(PageIO &page_io, 
                const PageIO::Page &root, root_updator_t root_updator):
            LinkedStackImpl(sizeof(T), page_io, root, root_updator)
        {}

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

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

