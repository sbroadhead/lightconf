#ifndef _LIGHTCONF_VALUE_IMPL_H_
#define _LIGHTCONF_VALUE_IMPL_H_

#include "value.hpp"

namespace lightconf {
////////////////////

//
//
inline value::value()                           : type_(value_type::invalid_type) { }
inline value::value(double dbl)                 : type_(value_type::number_type), number_value_(dbl) { }
inline value::value(const std::string& str)     : type_(value_type::string_type), string_value_(str) { }
inline value::value(bool bl)                    : type_(value_type::bool_type), bool_value_(bl) { }
inline value::value(const value_vector_type& lst) : type_(value_type::vector_type), vector_value_(lst) { }
inline value::value(const group& grp)           : type_(value_type::group_type), group_value_(grp) { }

//
//
inline value::value(const value& val) :
    type_(val.type_),
    number_value_(val.number_value_),
    string_value_(val.string_value_),
    bool_value_(val.bool_value_),
    vector_value_(val.vector_value_),
    group_value_(val.group_value_)
{ }

//
//
inline value& value::operator=(const value& rhs) {
    if (&rhs != this) {
        number_value_ = rhs.number_value_;
        string_value_ = rhs.string_value_;
        bool_value_ = rhs.bool_value_;
        vector_value_ = rhs.vector_value_;
        group_value_ = rhs.group_value_;
        type_ = rhs.type_;
    }
    return *this;
}

//
//
inline bool value::operator==(const value& rhs) const {
    if (rhs.type_ != type_) return false;
    switch (type_) {
    case value_type::number_type:
        return rhs.number_value_ == number_value_;
    case value_type::string_type:
        return rhs.string_value_ == string_value_;
    case value_type::bool_type:
        return rhs.bool_value_ == bool_value_;
    case value_type::vector_type:
        return rhs.vector_value_ == vector_value_;
    case value_type::group_type:
        return rhs.group_value_ == group_value_;
    default:
        return false;
    }
}

//
//
template <typename T>
inline return_type<T> value::get(const T& def) const {
    if (!value_type_info<T>::can_convert_from(*this)) {
        return def;
    }
    return value_type_info<T>::extract_value(*this);
}

//
//
template <typename T>
inline return_type<T> value::get() const {
    if (!value_type_info<T>::can_convert_from(*this)) {
        throw value_error("incompatible type requested");
    }

    return value_type_info<T>::extract_value(*this);
}

//
//
template <typename T>
inline bool value::is() const {
    return value_type_info<T>::can_convert_from(*this);
}

////////////////////
}

#endif // _LIGHTCONF_VALUE_IMPL_H_

