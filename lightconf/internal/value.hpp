#ifndef _LIGHTCONF_VALUE_H_
#define _LIGHTCONF_VALUE_H_

#include <map>
#include <string>
#include <vector>
#include "group.hpp"

namespace lightconf {
////////////////////

template <typename T> struct value_type_info;

template <typename T>
using return_type = decltype(value_type_info<T>::extract_value(std::declval<value>()));

typedef std::vector<value> value_vector_type;

//
//
enum class value_type {
    invalid_type,
    number_type,
    string_type,
    bool_type,
    group_type,
    vector_type
};

//
//
class value {
public:
    template <typename T>
    return_type<T>      get(const T& def) const;
    template <typename T>
    return_type<T>      get() const;
    template <typename T>
    bool                is() const;
    value_type          type() const { return type_; }

    value&              operator=(const value& rhs);
    bool                operator==(const value& rhs) const;

    double              number_value() const { return number_value_; }
    const std::string&  string_value() const { return string_value_; }
    bool                bool_value() const   { return bool_value_; }
    const value_vector_type& vector_value() const { return vector_value_; }
    const group&        group_value() const  { return group_value_; }

    value();
    explicit value(double dbl);
    explicit value(const std::string& str);
    explicit value(bool bl);
    explicit value(const value_vector_type& lst);
    explicit value(const group& grp);
    value(const value& val);
private:
    value_type          type_;
    double              number_value_;
    std::string         string_value_;
    bool                bool_value_;
    value_vector_type   vector_value_;
    group               group_value_;
};

////////////////////
}

#endif // _LIGHTCONF_VALUE_H_

