/*
 * $File: serializable_list.h
 * $Date: Sun Nov 23 20:17:15 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

/*!
 * generate list of serializable objects
 * \param F callback which receives a name of a serializable object
 */
#define GEN_SERIALIZABLE_LIST(F) \
    F(DATATYPE_INT) \
    F(GTEST_SERIALZE) \
    F(GTEST_SERIALZE_NO_DESERIALIZE)


// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

