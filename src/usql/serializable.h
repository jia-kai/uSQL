/*
 * $File: serializable.h
 * $Date: Sun Nov 23 20:14:20 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "./common.h"
#include "./serializable_list.h"

namespace usql {

class Serializable { 
    protected:
        virtual size_t do_get_serialize_length() const = 0;

        virtual void do_serialize(void *dest) const = 0;

    public:

        virtual ~Serializable() {}

        enum class TypeID;;
        using deserialize_factory_t = std::function<
            std::unique_ptr<Serializable>(const void *, size_t)>;

        /*!
         * get the object type id; each subclass of Serializable should have
         * its unique type id
         */
        virtual TypeID type_id() const = 0;

        /*!
         * return the length of serialized result, in bytes
         */
        size_t get_serialize_length();

        void serialize(void *dest);

        static std::unique_ptr<Serializable> deserialize(
                const void *data, size_t length);

        static void register_serializable(
                TypeID type_id, deserialize_factory_t factory);
};

/*!
 * helper macro to register a serializable class; it must provide a static
 * function `std::unique_ptr<cls> deserialize(const void *, size_t)`
 */
#define REGISTER_SERIALIZABLE(cls) \
    namespace { namespace __serializable_ctor_##cls { \
        static_assert(std::is_base_of<::usql::Serializable, cls>::value, \
                "must be derived from Serializable"); \
        class Ctor { \
            public: \
                Ctor() { \
                    cls v; \
                    ::usql::Serializable::register_serializable(\
                            v.type_id(), cls::deserialize); \
                } \
        }; \
        Ctor ctor; \
    }}

enum class Serializable::TypeID {
#define F(v) v,
    GEN_SERIALIZABLE_LIST(F)
#undef F
};

} // namespace usql

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

