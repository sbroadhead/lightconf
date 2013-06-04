#ifndef _LIGHTCONF_EXCEPTIONS_H_
#define _LIGHTCONF_EXCEPTIONS_H_

#include <stdexcept>
#include <string>

namespace lightconf {
////////////////////

//
//
class lightconf_error : public std::runtime_error {
public:
    explicit lightconf_error(const std::string& what) :
        std::runtime_error(what)
    { }
};

//
//
class value_error : public lightconf_error {
public:
    explicit value_error(const std::string& what) :
        lightconf_error(what)
    { }
};

//
//
class path_error : public lightconf_error {
public:
    explicit path_error(const std::string& what) :
        lightconf_error(what)
    { }
};

//
//
class parse_error : public lightconf_error {
public:
    int                 line() const { return line_; }
    int                 col() const { return col_; }

    parse_error(const std::string& what, int line, int col) :
        lightconf_error(what),
        line_(line),
        col_(col)
    { }

private:
    int                 line_;
    int                 col_;
};

//
//
class utf8_error : public parse_error {
public:
    utf8_error(const std::string& what, int line, int col) :
        parse_error(what, line, col)
    { }
};

////////////////////
}

#endif // _LIGHTCONF_EXCEPTIONS_H_

