#ifndef _LIGHTCONF_JSON_FORMAT_H_
#define _LIGHTCONF_JSON_FORMAT_H_

#include <algorithm>
#include <string>
#include "group.hpp"
#include "scanner.hpp"
#include "util.hpp"
#include "writer.hpp"

namespace lightconf { namespace json_format {
////////////////////


group                   read_group(scanner& sc, bool braces);
value_vector_type       read_vector(scanner& sc);
value                   read_value(scanner& sc);

void                    write_group(writer& wr, const group& gr);
void                    write_vector(writer& wr, const value_vector_type& vec);
void                    write_value(writer& wr, const value& val);

group                   read(const std::string& src);
std::string             write(const group& grp);

//
//
inline group read_group(scanner& sc, bool braces) {
    group grp;
    sc.expect('{');

    while (!sc.peek_token().is_char('}')) {
        std::string key = sc.expect_string();
        sc.expect(':');
        value val = read_value(sc);
        grp.set(key, val);

        if (sc.peek_token().is_char(',')) {
            sc.expect(',');
        } else {            
            break;
        }
    }

    sc.expect('}');
    return grp;
}

//
//
inline value_vector_type read_vector(scanner& sc) {
    sc.expect('[');

    value_vector_type vec;
    while (!sc.peek_token().is_char(']')) {
        vec.push_back(read_value(sc));
        if (sc.peek_token().is_char(',')) {
            sc.expect(',');
        } else {
            break;
        }
    }

    sc.expect(']');
    return vec;
}

//
//
inline value read_value(scanner& sc) {
    if (sc.peek_token().is_char('{')) {
        return value(read_group(sc, true));
    } else if (sc.peek_token().is_char('[')) {
        return value(read_vector(sc));
    } else {
        switch (sc.peek_token().type) {
        case token_type::identifier_token: {
            std::string ident = sc.expect_identifier();
            if (ident == "true") {
                return value(true);
            } else if (ident == "false") {
                return value(false);
            } else if (ident == "null") {
                sc.fail("null is not a valid value", sc.peek_token().line, sc.peek_token().col);
            } else {
                sc.fail("unexpected identifier", sc.peek_token().line, sc.peek_token().col);
            }
            break;
        }
        case token_type::string_token: {
            std::string str = sc.expect_string();
            return value(str);
        }
        case token_type::number_token: {
            double dbl = sc.expect_number();
            return value(dbl);
        }
        case token_type::char_token: {
            sc.fail("unexpected '" + std::string(1, sc.peek_token().char_value) + "'",
                sc.peek_token().line, sc.peek_token().col);
            break;
        }
        default: {
            sc.fail("unexpected token", sc.peek_token().line, sc.peek_token().col);
            break;
        }

        }
    }
    return value();
}

//
//
inline void write_group(writer& wr, const group& gr) {
    wr.append("{");
    wr.indent();
    wr.newline();

    unsigned int i = 0;
    for (const auto& key : gr) {
        wr.append("\"");
        wr.append(escape_string(key));
        wr.append("\": ");
        write_value(wr, gr.get<value>(key));

        if (i < gr.size() - 1) {
            wr.append(",");
            wr.newline();
        }
        i++;
    }

    wr.unindent();
    wr.newline();
    wr.append("}");
}

//
//
inline void write_vector(writer& wr, const value_vector_type& vec) {
    wr.append("[");
    wr.indent();
    wr.newline();

    unsigned int i = 0;
    for (const auto& val : vec) {
        write_value(wr, val);

        if (i < vec.size() - 1) {
            wr.append(",");
            wr.newline();
        }
        i++;
    }

    wr.unindent();
    wr.newline();
    wr.append("]");
}

//
//
inline void write_value(writer& wr, const value& val) {
    switch (val.type()) {
    case value_type::number_type: {
        wr.append(stringize_number(val.number_value()));
        break;
    }
    case value_type::string_type: {
        wr.append("\"");
        wr.append(escape_string(val.string_value()));
        wr.append("\"");
        break;
    }
    case value_type::bool_type: {
        wr.append(val.bool_value() ? "true" : "false");
        break;
    }
    case value_type::group_type: {
        write_group(wr, val.group_value());
        break;
    }
    case value_type::vector_type: {
        write_vector(wr, val.vector_value());
        break;
    }
    default: break;
    }
}

//
//
inline group read(const std::string& src) {
    scanner sc;
    sc.scan(src);
    return read_group(sc, false);
}

//
//
inline std::string write(const group& grp) {
    writer wr;
    write_group(wr, grp);
    return wr.buf;
}

////////////////////
} }
#endif // _LIGHTCONF_JSON_FORMAT_H_

