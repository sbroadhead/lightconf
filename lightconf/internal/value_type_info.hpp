#ifndef _LIGHTCONF_VALUE_TYPE_INFO_H_
#define _LIGHTCONF_VALUE_TYPE_INFO_H_

#include <tuple>
#include <vector>
#include <map>
#include "value.hpp"

namespace lightconf {
////////////////////

template <typename T> struct value_type_info;

#define DEFINE_VALUE_TYPE(type_name, req_type,  extract, construct) \
template <> struct value_type_info<type_name> { \
    static bool can_convert_from(const value& val) { return val.type() == req_type; } \
    static type_name extract_value(const value& val) { return extract; } \
    static value create_value(const type_name& x) { return construct; } \
};
#define DEFINE_VALUE_TYPE_REF(type_name, req_type,  extract, construct) \
template <> struct value_type_info<type_name> { \
    static bool can_convert_from(const value& val) { return val.type() == req_type; } \
    static const type_name& extract_value(const value& val) { return extract; } \
    static value create_value(const type_name& x) { return construct; } \
};


DEFINE_VALUE_TYPE    (double,            value_type::number_type, val.number_value(),   value(x))
DEFINE_VALUE_TYPE    (int,               value_type::number_type, val.number_value(),   value((double)x))
DEFINE_VALUE_TYPE    (float,             value_type::number_type, val.number_value(),   value((double)x))
DEFINE_VALUE_TYPE    (bool,              value_type::bool_type,   val.bool_value(),     value(x))
DEFINE_VALUE_TYPE_REF(std::string,       value_type::string_type, val.string_value(),   value(x))
DEFINE_VALUE_TYPE_REF(value_vector_type, value_type::vector_type, val.vector_value(),   value(x))
DEFINE_VALUE_TYPE_REF(group,             value_type::group_type,  val.group_value(),    value(x))

#undef DEFINE_VALUE_TYPE
#undef DEFINE_VALUE_TYPE_REF

#define LIGHTCONF_BEGIN_ENUM(enumname) \
template <> struct value_type_info<enumname> { \
    static const std::map<enumname, std::string> names; \
    static bool can_convert_from(const value& val) { \
        if (val.type() != value_type::string_type) return false; \
        for (auto kv : names) { if (kv.second == val.string_value()) return true; } \
        return false; \
    } \
    static enumname extract_value(const value& val) { \
        for (auto kv : names) { if (kv.second == val.string_value()) return kv.first; } \
        return std::begin(names)->first; \
    } \
    static value create_value(enumname x) { \
        auto it = names.find(x); \
        if (it != std::end(names)) { return value(it->second); } \
        return value(std::begin(names)->second); \
    } \
}; \
const std::map<enumname, std::string> value_type_info<enumname>::names = {
#define LIGHTCONF_ENUM_VALUE(value, string) { value, string },
#define LIGHTCONF_END_ENUM() \
};


#define LIGHTCONF_BEGIN_TYPE(tyname) \
template <> struct value_type_info<tyname> { \
    static bool convert(const value *inval, const tyname *inty, group *outgrp, tyname *outty); \
    static bool can_convert_from(const value& val) { \
        return convert(&val, nullptr, nullptr, nullptr); \
    } \
    static tyname extract_value(const value& val) { \
        tyname out; \
        convert(&val, nullptr, nullptr, &out); \
        return out; \
    } \
    static value create_value(const tyname& x) { \
        group out; \
        convert(nullptr, &x, &out, nullptr); \
        return value(out); \
    } \
}; \
bool value_type_info<tyname>::convert(const value *inval, const tyname *inty, group *outgrp, tyname *outty) { \
    const group *grp = nullptr; \
    if (inval) grp = &inval->group_value();


#define LIGHTCONF_TYPE_MEMBER_REQ(type, name, key) \
    if (grp && !grp->has<type>(key)) return false; \
    if (grp && outty) outty->name = grp->get<type>(key); \
    if (outgrp && inty) outgrp->set<type>(key, inty->name);

#define LIGHTCONF_TYPE_MEMBER_OPT(type, name, key, def) \
    if (grp && outty) outty->name = grp->get<type>(key, def); \
    if (outgrp && inty) outgrp->set<type>(key, inty->name);

#define LIGHTCONF_END_TYPE() \
    return true; \
};



//
//
template <>
struct value_type_info<value> {
    static bool can_convert_from(const value& val) { return true; }
    static const value& extract_value(const value& val) { return val; }
    static value create_value(const value& val) { return val; }
};

//
//
template <>
struct value_type_info<const char *> {
    static bool can_convert_from(const value& val) { return val.type() == value_type::string_type; }
    static const char *extract_value(const value& val) { return val.string_value().c_str(); }
    static value create_value(const char *x) { return value(std::string(x)); }
};

//
//
template <typename U>
struct value_type_info<std::vector<U>> {
    static bool can_convert_from(const value& val) { return val.type() == value_type::vector_type; }
    static std::vector<U> extract_value(const value& val) {
        std::vector<U> v;
        const value_vector_type& inner_vals = val.get<value_vector_type>();
        for (const auto& u : inner_vals) {
            v.push_back(u.get<U>());
        }
        return v;
    }
    static value create_value(const std::vector<U>& x) {
        value_vector_type inner_vals;
        for (const auto& u : x) {
            inner_vals.push_back(value_type_info<U>::create_value(u));
        }
        return value(inner_vals);
    }
};

//
//
template <typename... Args>
struct tuple_value_converter;

//
//
template <typename First, typename... Rest>
struct tuple_value_converter<First, Rest...> {
    static std::tuple<First, Rest...> extract(const value_vector_type& vec, int i) {
        return std::tuple_cat(std::make_tuple(vec[i].get<First>()),
            tuple_value_converter<Rest...>::extract(vec, i+1));
    }
    template <int idx, typename... Args>
    static void append(value_vector_type& vec, const std::tuple<Args...>& tup) {
        vec.push_back(value_type_info<First>::create_value(std::get<idx>(tup)));
        tuple_value_converter<Rest...>::template append<idx+1>(vec, tup);
    }
    static bool types_match(const value_vector_type& vec, int i) {
        return vec[i].is<First>() && tuple_value_converter<Rest...>::types_match(vec, i+1);
    }
};

//
//
template <>
struct tuple_value_converter<> {
    static std::tuple<> extract(const value_vector_type& vec, int i) {
        return std::make_tuple();
    }
    template <int idx, typename... Args>
    static void append(value_vector_type& vec, const std::tuple<Args...>& tup) { }
    static bool types_match(const value_vector_type& vec, int i) {
        return true;
    }
};

//
//
template <typename... T>
struct value_type_info<std::tuple<T...>> {
    static bool can_convert_from(const value& val) {
        if (val.type() != value_type::vector_type) {
            return false;
        }
        const auto& vec = val.get<value_vector_type>();
        return vec.size() == sizeof...(T)
            && tuple_value_converter<T...>::types_match(vec, 0);
    }
    static std::tuple<T...> extract_value(const value& val) {
        return tuple_value_converter<T...>::extract(val.get<value_vector_type>(), 0);
    }
    static value create_value(const std::tuple<T...>& x) {
        value_vector_type vec;
        tuple_value_converter<T...>::template append<0>(vec, x);
        return value(vec);
    }
};


////////////////////
}

#endif // _LIGHTCONF_VALUE_TYPE_INFO_H_
