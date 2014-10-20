/*
 * $File: page_io.h
 * $Date: Mon Oct 20 22:32:12 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "os/file_io.h"
template<typename T>
class LinkedStack;

class PageIO {
    public:
        using page_id_t = size_t;
        class Page;

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
            // page with 0 id is the meta page
            return m_id != 0;
        }

        template<typename T>
        const T* read(size_t offset = 0) const {
            return reinterpret_cast<const T*>(
                    static_cast<const uint8_t*>(m_fpage.read())
                    + offset);
        }

        template<typename T>
        T* write_start(size_t offset = 0) {
            return reinterpret_cast<T*>(
                    static_cast<uint8_t*>(m_fpage.write_start())
                    + offset);
        }

        void write_finish() {
            m_fpage.write_finish();
        }
};

PageIO::Page PageIO::lookup(page_id_t id) {
    return {id, m_fio.get_page(id)};
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

