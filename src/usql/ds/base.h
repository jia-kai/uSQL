/*
 * $File: base.h
 * $Date: Thu Oct 23 23:47:46 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "../file/page_io.h"

namespace usql {

class PagedDataStructureBase {
    public:
        using root_updator_t = std::function<void(const PageIO::Page&)>;

        PagedDataStructureBase(PageIO &page_io);

        static constexpr size_t align_elem_size(size_t s) {
            return (s + 3) & ~(size_t(3));
        }

        /*!
         * \brief load from specified root
         */
        virtual void load(PageIO::page_id_t root, root_updator_t root_updator);

        virtual ~PagedDataStructureBase() {}

    protected:
        PageIO &m_page_io;
        PageIO::Page m_root;
        root_updator_t m_root_updator;

        void set_root(const PageIO::Page &root) {
            m_root = root;
            m_root_updator(m_root);
        }
};

}   // namespace usql

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

