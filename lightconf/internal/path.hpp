#ifndef _LIGHTCONF_INTERNAL_PATH_H_
#define _LIGHTCONF_INTERNAL_PATH_H_

#include "exceptions.hpp"
#include <iterator>
#include <string>
#include <vector>

namespace lightconf {
////////////////////

class path {
public:
    typedef std::vector<std::string>                container_type;
    typedef container_type::iterator                iterator;
    typedef container_type::const_iterator          const_iterator;
    typedef container_type::size_type               size_type;

    path&               operator=(const path& rhs);

    path&               operator+=(const std::string& part);
    path&               operator+=(const char *part);
    path&               operator+=(const path& rhs);

    path                operator+(const std::string& part) const;
    path                operator+(const char *part) const;
    path                operator+(const path& rhs) const;

    std::string         fullpath(char separator = '.') const;

    // STL container passthrough
    iterator            begin()                             { return parts_.begin(); }
    iterator            end()                               { return parts_.end(); }
    const_iterator      begin() const                       { return parts_.begin(); }
    const_iterator      end() const                         { return parts_.end(); }
    size_type           size() const                        { return parts_.size(); }
    bool                empty() const                       { return parts_.empty(); }
    path&               append(const std::string& part)     { return *this += part; }
    path&               append(const char *part)            { return *this += part; }
    path&               append(const path& rhs)             { return *this += rhs; }
    void                push_back(const std::string& part)  { *this += part; }
    void                push_back(const char *part)         { *this += part; }
    void                pop_back()                          { parts_.pop_back(); }
    void                clear()                             { parts_.clear(); }
    std::string&        operator[](size_type pos)           { return parts_[pos]; }
    const std::string&  operator[](size_type pos) const     { return parts_[pos]; }
    std::string&        at(size_type pos)                   { return parts_.at(pos); }
    const std::string&  at(size_type pos) const             { return parts_.at(pos); }
    bool                operator==(const path& rhs) const   { return parts_ == rhs.parts_; }
    bool                operator!=(const path& rhs) const   { return parts_ != rhs.parts_; }
    bool                operator<(const path& rhs) const    { return parts_ < rhs.parts_; }
    bool                operator<=(const path& rhs) const   { return parts_ <= rhs.parts_; }
    bool                operator>(const path& rhs) const    { return parts_ > rhs.parts_; }
    bool                operator>=(const path& rhs) const   { return parts_ >= rhs.parts_; }

    path(const std::string& path_string, char separator = '.');
    path(const char *path_string, char separator = '.');
    path(const path& rhs);
    template <typename InputIterator> path(InputIterator first, InputIterator last);
    path();

private:
    void                parse(const std::string& path_string, char separator = '.');

    container_type parts_;
};

//
//
inline path::path() { }

//
//
inline path::path(const std::string& path_string, char separator) {
    parse(path_string, separator);
}

//
//
inline path::path(const char *path_string, char separator) {
    parse(std::string(path_string), separator);
}

//
//
inline path::path(const path& rhs) : parts_(rhs.parts_)
{ }

//
//
template <typename InputIterator>
inline path::path(InputIterator first, InputIterator last) : parts_(first, last)
{ }

//
//
inline path& path::operator=(const path& rhs) {
    parts_ = rhs.parts_;
    return *this;
}

//
//
inline path& path::operator+=(const std::string& part) {
    parts_.push_back(part);
    return *this;
}

//
//
inline path& path::operator+=(const char *part) {
    return operator+=(std::string(part));
}

//
//
inline path& path::operator+=(const path& rhs) {
    parts_.insert(std::end(parts_), std::begin(rhs.parts_), std::end(rhs.parts_));
    return *this;
}


//
//
inline path path::operator+(const std::string& part) const {
    path new_path = *this;
    new_path += part;
    return new_path;
}

//
//
inline path path::operator+(const char *part) const {
    return operator+(std::string(part));
}

//
//
inline path path::operator+(const path& rhs) const {
    path new_path = *this;
    new_path += rhs;
    return new_path;
}

//
//
inline std::string path::fullpath(char separator) const {
    std::string p;
    for (auto it = begin(); it != end(); ++it) {
        p += *it;
        if (it != end() - 1) p += separator;
    }
    return p;
}

//
//
inline void path::parse(const std::string& path_string, char separator) {
    size_t start = 0, end = 0;
    while (start < path_string.size()) {
        end = path_string.find(separator, start);
        if (end == std::string::npos) {
            end = path_string.size();
        }
        std::string subkey(&path_string[start], &path_string[end]);
        parts_.push_back(subkey);
        start = end = end + 1;
    }
}

////////////////////
}

#endif // _LIGHTCONF_INTERNAL_PATH_H_
