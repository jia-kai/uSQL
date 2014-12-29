/*
 * $File: common.cpp
 * $Date: Mon Dec 22 01:33:25 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "common.h"
#include "exception.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <unistd.h>

bool usql::g_usql_log_enable = true;

void usql::__usql_log__(const char *file, const char *func, int line,
		const char *fmt, ...) {

    if(!usql::g_usql_log_enable)
        return;

#define TIME_FMT	"[%s %s@%s:%d]"
	static std::mutex mtx;

	static const char *time_fmt = nullptr;
	if (!time_fmt) {
		if (isatty(fileno(stderr)))
			time_fmt = "\033[33m" TIME_FMT "\033[0m ";
		else
			time_fmt = TIME_FMT " ";
	}
	time_t cur_time;
	time(&cur_time);
	char timestr[64];
	strftime(timestr, sizeof(timestr), "%H:%M:%S",
			localtime(&cur_time));

	{
		LOCK_GUARD(mtx);
        #ifdef _GNU_SOURCE
		fprintf(stderr, time_fmt, timestr, func, basename(strdupa(file)), line);
        #else
        // strdupa is only present on GNU GCC
        fprintf(stderr, time_fmt, timestr, func, file, line);
        #endif

		va_list ap;
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fputc('\n', stderr);
	}

}


void usql::__usql_assert_fail__(
        const char *file, int line, const char *func,
        const char *expr, const char *msg_fmt, ...) {
    std::string msg;
    if (msg_fmt) {
        va_list ap;
        va_start(ap, msg_fmt);
        msg = "\nextra message: ";
        msg.append(svsprintf(msg_fmt, ap));
        va_end(ap);
    }
    throw AssertionError{ssprintf(
            "assertion `%s' failed at %s:%d: %s%s\n",
            expr, file, line, func, msg.c_str())};
}

std::string usql::svsprintf(const char *fmt, va_list ap_orig) {
	int size = 100;     /* Guess we need no more than 100 bytes */
	char *p;

	if ((p = (char*)malloc(size)) == nullptr)
		goto err;

	for (; ;) {
		va_list ap;
		va_copy(ap, ap_orig);
		int n = vsnprintf(p, size, fmt, ap);
		va_end(ap);

		if (n < 0)
			goto err;

		if (n < size) {
			std::string rst(p);
			free(p);
			return rst;
		}

		size = n + 1;

		char *np = (char*)realloc(p, size);
		if (!np) {
			free(p);
			goto err;
		} else 
			p = np;
	}

err:
	fprintf(stderr, "could not allocate memory for svsprintf\n");
    __builtin_trap();
}

std::string usql::ssprintf(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	auto rst = svsprintf(fmt, ap);
	va_end(ap);
	return rst;
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

