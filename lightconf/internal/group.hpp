#ifndef _LIGHTCONF_GROUP_H_
#define _LIGHTCONF_GROUP_H_

#include <map>
#include <type_traits>
#include <vector>
#include "path.hpp"

namespace lightconf {
////////////////////

class value;
template <typename T> struct value_type_info;

typedef std::map<std::string, value> value_map_type;

template <typename T>
using return_type = decltype(value_type_info<T>::extract_value(std::declval<value>()));

//
//
class group {
public:
    typedef std::vector<std::string>::const_iterator const_iterator;
    typedef std::vector<std::string>::size_type size_type;

    template <typename T>
    return_type<T>      get(const path& key) const;
    template <typename T>
    return_type<T>      get(const path& key, const T& def) const;
    template <typename T>
    void                set(const path& key, const T& val = T());
    template <typename T>
    bool                has(const path& key) const;
    void                unset(const path& key);

    bool                operator==(const group& rhs) const;

    const_iterator      begin() const { return order_.begin(); }
    const_iterator      end() const { return order_.end(); }

    size_type           size() const { return order_.size(); }

    group();
private:
    template <typename InputIterator>
    const value *       find_value(InputIterator first, InputIterator last, const group **parent_group, path *key_rest) const;
    template <typename InputIterator, typename T>
    value *             create_value(InputIterator first, InputIterator last, const T& val);
    void                set_key(const std::string& key, const value& val);    

    value_map_type      values_;
    std::vector<std::string> order_;
};

////////////////////
}

#endif // _LIGHTCONF_GROUP_H_

