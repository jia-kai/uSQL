/*
 * $File: common.h
 * $Date: Tue Oct 21 09:51:23 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include <memory>
#include <string>
#include <mutex>

#include <cstdint>
#include <cstddef>
#include <cstdarg>

namespace usql {

#define likely(v)   __builtin_expect(bool(v), 1)
#define unlikely(v)   __builtin_expect(bool(v), 0)

#define LOCK_GUARD(mtx) \
	std::lock_guard<decltype(mtx)> __lock_guard_##mtx(mtx)

#define usql_log(fmt...) \
	__usql_log__(__FILE__, __func__, __LINE__, fmt)

using rowid_t = int64_t;

extern bool g_usql_log_enable;

/*!
 * \brief printf-like std::string constructor
 */
std::string svsprintf(const char *fmt, va_list ap);

std::string ssprintf(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));


#define usql_assert(expr, msg...) \
    do { \
        if (unlikely(!(expr))) \
            __usql_assert_fail__(__FILE__, __LINE__, \
                    __PRETTY_FUNCTION__, # expr, ##msg); \
    } while(0)

void __usql_assert_fail__(
        const char *file, int line, const char *func,
        const char *expr, const char *msg_fmt = nullptr, ...)
    __attribute__((format(printf, 5, 6), noreturn));

void __usql_log__(const char *file, const char *func, int line,
		const char *fmt, ...)
    __attribute__((format(printf, 4, 5)));

}   // namespace usql

#ifdef __APPLE__
#define THREAD_LOCAL __thread
#else
#define THREAD_LOCAL thread_local
#endif

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

