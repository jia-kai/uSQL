/*
 * $File: page_io.h
 * $Date: Sun Dec 21 23:46:51 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "../os/file_io.h"

namespace usql {

template<typename T>
class LinkedStack;

/*!
 * \brief page-based file management, with allocation and deallocation support
 */
class PageIO {
    public:
        using page_id_t = size_t;
        class Page;

        template<typename T>
        class Ptr;

        PageIO(FileIO &&fio);
        ~PageIO();

        size_t page_size() const {
            return m_fio.page_size();
        }

        Page alloc();
        inline Page lookup(page_id_t id);
        void free(Page &&page);

        FileIO& file_io() {
            return m_fio;
        }

    private:
        FileIO m_fio;
        std::unique_ptr<LinkedStack<page_id_t>> m_freelist;
};

class PageIO::Page {
    friend class PageIO;

    page_id_t m_id = 0;
    FileIO::Page m_fpage;

    Page(page_id_t id, FileIO::Page &&fpage):
        m_id(id), m_fpage(std::move(fpage))
    {}

    public:
        Page() = default;

        page_id_t id() const {
            return m_id;
        }

        bool valid() const {
            // page with 0 id is the meta page, and considered to be invalid
            return m_id != 0;
        }

        template<typename T>
        const T* read(size_t offset = 0) const {
            usql_assert(valid());
            return reinterpret_cast<const T*>(
                    static_cast<const uint8_t*>(m_fpage.read())
                    + offset);
        }

        template<typename T>
        T* write(size_t offset = 0) {
            usql_assert(valid());
            return reinterpret_cast<T*>(
                    static_cast<uint8_t*>(m_fpage.write())
                    + offset);
        }

        void reset() {
            m_id = 0;
            m_fpage.reset();
        }

        template<typename T>
        inline PageIO::Ptr<T> make_ptr(size_t offset);
};

template<typename T>
class PageIO::Ptr {
    friend class PageIO::Page;
    PageIO::Page &m_page;
    size_t m_offset;

    Ptr(PageIO::Page &p, size_t offset):
        m_page(p), m_offset(offset)
    {
    }

    public:
        T* operator -> () {
            return m_page.read<T>(m_offset);
        }

        T& operator * () {
            return *(this->operator->());
        }
};

template<typename T>
PageIO::Ptr<T> PageIO::Page::make_ptr(size_t offset) {
    usql_assert(valid());
    return {*this, offset};
}

PageIO::Page PageIO::lookup(page_id_t id) {
    return {id, m_fio.get_page(id)};
}

}   // namespace usql

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

