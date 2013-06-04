#ifndef _LIGHTCONF_GROUP_IMPL_H_
#define _LIGHTCONF_GROUP_IMPL_H_

#include <algorithm>
#include "group.hpp"
#include "value_impl.hpp"
#include "path.hpp"
#include "value_type_info.hpp"

namespace lightconf {
////////////////////

//
//
inline group::group() : values_(), order_()
{ }

//
//
inline bool group::operator==(const group& rhs) const {
    if (values_.size() != rhs.values_.size()) {
        return false;
    }

    auto it1 = std::begin(values_);
    auto it2 = std::begin(rhs.values_);
    while (it1 != std::end(values_)) {
        if (*it1 != *it2) {
            return false;
        }
        ++it1;
        ++it2;
    }

    auto it3 = std::begin(order_);
    auto it4 = std::begin(rhs.order_);
    while (it3 != std::end(order_)) {
        if (*it3 != *it4) {
            return false;
        }
        ++it3;
        ++it4;
    }
    return true;
}

//
//
template <typename T>
inline return_type<T> group::get(const path& key) const {
    const value *val;
    const group *parent_group;
    path key_rest;

    val = find_value(std::begin(key), std::end(key), &parent_group, &key_rest);
    if (val) {
        return val->get<T>();
    }
    throw path_error("non-existent path requested: " + key.fullpath());
}

//
//
template <typename T>
inline return_type<T> group::get(const path& key, const T& def) const {
    const value *result;
    const group *parent_group;
    path key_rest;

    result = find_value(std::begin(key), std::end(key), &parent_group, &key_rest);
    if (result) {
        return result->get<T>(def);
    }
    return def;
}

//
//
template <typename T>
inline void group::set(const path& key, const T& val) {
    const value *result_const;
    const group *parent_group_const;
    group *parent_group;
    value *result;
    path key_rest;

    result_const = find_value(std::begin(key), std::end(key), &parent_group_const, &key_rest);

    parent_group = const_cast<group *>(parent_group_const);
    result = const_cast<value *>(result_const);

    if (result) {
        *result = value_type_info<T>::create_value(val);
    } else {
        parent_group->create_value(std::begin(key_rest), std::end(key_rest), val);
    }
}

//
//
template <typename T>
inline bool group::has(const path& key) const {
    const value *result;
    const group *parent_group;
    path key_rest;

    result = find_value(std::begin(key), std::end(key), &parent_group, &key_rest);
    if (result) {
        return result->is<T>();
    }
    return false;
}

//
//
inline void group::unset(const path& key) {
    const value *result_const;
    const group *parent_group_const;
    group *parent_group;
    value *result;
    path key_rest;

    result_const = find_value(std::begin(key), std::end(key), &parent_group_const, &key_rest);

    parent_group = const_cast<group *>(parent_group_const);
    result = const_cast<value *>(result_const);

    if (result) {
        parent_group->values_.erase(*(std::end(key) - 1));
        auto it = std::find(parent_group->order_.begin(), parent_group->order_.end(), *(std::end(key) - 1));
        parent_group->order_.erase(it);
    } 
}

//
//
template <typename InputIterator>
inline const value *group::find_value(InputIterator first, InputIterator last,
        const group **parent_group, path *key_rest) const {

    if (first == last) {
        throw path_error("value at empty path requested");
    }

    auto it = values_.find(*first);
    if (first == last - 1) {
        if (it == values_.end()) {
            *parent_group = this;
            *key_rest = path(first, last);
            return 0;
        } else {
            *parent_group = this;
            *key_rest = path();
            return &it->second;
        }
    } else {
        if (it == values_.end() || !it->second.template is<group>()) {
            *parent_group = this;
            *key_rest = path(first, last);
            return 0;
        } else {
            const group& grp = it->second.template get<group>();
            return grp.find_value(first + 1, last, parent_group, key_rest);
        }
    }
}

//
//
template <typename InputIterator, typename T>
inline value *group::create_value(InputIterator first, InputIterator last, const T& val) {
    if (first == last) {
        throw path_error("value at empty path created");
    }

    char c = (*first)[0];
    if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z')) {
        throw path_error("key starts with an invalid character");
    }

    for (char c : *first) {
        if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z')
            && (c < '0' || c > '9') && c != '_' && c != '-') {
            throw path_error("key contains an invalid character");
        }
    }

    if (first == last - 1) {
        set_key(*first, value_type_info<T>::create_value(val));
        return &values_.at(*first);
    } else {
        set_key(*first, value(group()));
        const group& grp_const = values_.at(*first).template get<group>();

        // we can safely strip constness because we know our child groups are not const
        group& grp = const_cast<group&>(grp_const);
        return grp.create_value(first + 1, last, val);
    }
}

//
//
inline void group::set_key(const std::string& key, const value& val) {
    if (values_.find(key) == values_.end()) {
        order_.push_back(key);
    }
    values_[key] = val;
}

////////////////////
} 

#endif // _LIGHTCONF_GROUP_IMPL_H_
