#ifndef _LIGHTCONF_CONFIG_FORMAT_H_
#define _LIGHTCONF_CONFIG_FORMAT_H_

#include <algorithm>
#include <set>
#include <string>
#include "group.hpp"
#include "scanner.hpp"
#include "util.hpp"
#include "writer.hpp"

namespace lightconf { namespace config_format {
////////////////////


group                   read_group(scanner& sc, bool braces);
value_vector_type       read_vector(scanner& sc);
value                   read_value(scanner& sc);

void                    write_group(scanner& sc, writer& wr, bool braces, const group& gr);
void                    write_vector(scanner& sc, writer& wr, const value_vector_type& vec);
void                    write_value(scanner& sc, writer& wr, const value& val);

int                     value_length(const value& val, int wrap_length = 120);
int                     group_length(const group& gr, int wrap_length = 120);
int                     vector_length(const value_vector_type& vec, int wrap_length = 120);

scanner                 make_scanner(const std::string& input);

group                   read(const std::string& src);
std::string             write(const group& grp, const std::string& src, int wrap_length = 120);

//
//
inline group read_group(scanner& sc, bool braces) {
    group grp;
    if (braces) {
        sc.expect('{');
    }
    while (sc.peek_token().type != token_type::eof_token && !(braces && sc.peek_token().is_char('}'))) {
        std::string key = sc.expect_identifier();
        sc.expect('=');
        value val = read_value(sc);
        grp.set(key, val);

        sc.expect(',', true);
        if (braces && sc.peek_token().is_char('}')) {
            sc.expect('}');
            break;
        }
    }

    return grp;
}

//
//
inline value_vector_type read_vector(scanner& sc) {
    sc.expect('[');
    value_vector_type vec;
    while (!sc.peek_token().is_char(']')) {
        vec.push_back(read_value(sc));
        sc.expect(',', true);
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
inline void write_group(scanner& sc, writer& wr, bool braces, const group& gr) {
    bool wrap = !braces || group_length(gr) > wr.wrap_length;

    if (braces) {
        wr.append("{ ");
        wr.indent();
        if (wrap) {
            wr.newline();
        }
        sc.expect('{');
    }

    std::vector<std::string> keys;
    for (const auto& key : gr) {
        keys.push_back(key);
    }

    while (sc.peek_token().type != token_type::eof_token && !(braces && sc.peek_token().is_char('}'))) {
        std::string key = sc.expect_identifier();
        sc.expect('=');

        auto key_it = std::find(std::begin(keys), std::end(keys), key);
        if (key_it != keys.end()) {
            wr.append(key);
            wr.append(" = ");
            write_value(sc, wr, gr.get<value>(*key_it));
            keys.erase(key_it);
            bool terminate = sc.peek_token().is_char('}') && keys.empty();
            if (!terminate) {
                if (wrap) {
                    wr.newline();
                } else {
                    wr.append(", ");
                }
            } else if (!wrap) {
                wr.append(" ");
            }
        } else {
            read_value(sc);
        }

        sc.expect(',', true);
        if (braces && sc.peek_token().is_char('}')) {
            break;
        }
    }

    unsigned int i = 0;
    for (const auto& key : keys) {
        wr.append(key);
        wr.append(" = ");
        scanner dummy_scanner = make_scanner("0");
        write_value(dummy_scanner, wr, gr.get<value>(key));
        if (i < keys.size() - 1) {
            if (wrap) {
                wr.newline();
            } else {
                wr.append(", ");
            }
        } else if (!wrap) {
            wr.append(" ");
        }
        i++;
    }

    if (braces) {
        wr.unindent();
        if (wrap) {
            wr.newline();
        }
        sc.expect('}');
        wr.append("}");
    }
}

//
//
inline void write_vector(scanner& sc, writer& wr, const value_vector_type& vec) {
    bool wrap = vector_length(vec) > wr.wrap_length;

    wr.append("[ ");
    wr.indent();
    if (wrap) {
        wr.newline();
    }
    sc.expect('[');

    unsigned int vals_read = 0;
    while (!sc.peek_token().is_char(']')) {
        if (vals_read < vec.size()) {
            write_value(sc, wr, vec[vals_read]);
            vals_read++;
        } else {
            read_value(sc);
        }

        bool terminate = sc.peek_token().is_char(']') && vals_read == vec.size();
        if (!terminate) {
            if (wrap) {
                wr.newline();
            } else {
                wr.append(", ");
            }
        } else if (!wrap) {
            wr.append(" ");
        }
        sc.expect(',', true);
    }

    for (unsigned int i = vals_read; i < vec.size(); i++) {
        scanner dummy_scanner = make_scanner("0");
        write_value(dummy_scanner, wr, vec[i]);
        if (i < vec.size() - 1) {
            if (wrap) {
                wr.newline();
            } else {
                wr.append(", ");
            }
        } else if (!wrap) {
            wr.append(" ");
        }
    }

    wr.unindent();
    if (wrap) {
        wr.newline();
    }

    sc.expect(']');
    wr.append("]");
}

//
//
inline void write_value(scanner& sc, writer& wr, const value& val) {
    switch (val.type()) {
    case value_type::number_type: {
        read_value(sc);
        wr.append(stringize_number(val.number_value()));
        break;
    }
    case value_type::string_type: {
        read_value(sc);
        wr.append("\"");
        wr.append(escape_string(val.string_value()));
        wr.append("\"");
        break;
    }
    case value_type::bool_type: {
        read_value(sc);
        wr.append(val.bool_value() ? "true" : "false");
        break;
    }
    case value_type::group_type: {
        if (sc.peek_token().is_char('{')) {
            write_group(sc, wr, true, val.group_value());
        } else {
            read_value(sc);
            scanner dummy_scanner = make_scanner("{}");
            write_group(dummy_scanner, wr, true, val.group_value());
        }
        break;
    }
    case value_type::vector_type: {
        if (sc.peek_token().is_char('[')) {
            write_vector(sc, wr, val.vector_value());
        } else {
            read_value(sc);
            scanner dummy_scanner = make_scanner("[]");
            write_vector(dummy_scanner, wr, val.vector_value());
        }
        break;
    }
    default: {
        break;
    }
    }
}

//
//
inline int value_length(const value& val, int wrap_length) {
    switch (val.type()) {
    case value_type::number_type: {
        return stringize_number(val.number_value()).size();
    }
    case value_type::string_type: {
        return 2 + escape_string(val.string_value()).size();
    }
    case value_type::bool_type: {
        return val.bool_value() ? 4 : 5;
    }
    case value_type::group_type: {
        return group_length(val.group_value());
    }
    case value_type::vector_type: {
        return vector_length(val.vector_value());
    }
    default: {
        return 0;
    }
    }
}

//
//
inline int group_length(const group& gr, int wrap_length) {
    int sum = 3; // length of "{ }"
    for (const auto& key : gr) { 
        sum += 4 + key.size() + value_length(gr.get<value>(key)); // ", = "
    }
    return sum;
}

//
//
inline int vector_length(const value_vector_type& vec, int wrap_length) {
    int sum = 3; // length of "[ ]"
    for (const auto& val : vec) {
        sum += 2 + value_length(val); // ", "
    }
    return sum;
}

//
//
inline scanner make_scanner(const std::string& input) {
    scanner sc;
    sc.scan(input);
    return sc;
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
inline std::string write(const group& grp, const std::string& src, int wrap_length) {
    scanner sc;
    writer wr;
    wr.wrap_length = wrap_length;
    scanner_params params;

    params.flags = allow_comments_flag | blank_line_comment_flag;
    params.comment_function = [&](const std::string& str) {
        wr.append(str);
        wr.newline();
    };
    sc.scan(src, params);
    try {
        write_group(sc, wr, false, grp);
    } catch (parse_error e) {
        // if we fail to parse the source file to update, just write as if writing a brand new file
        scanner dummy_scanner = make_scanner("");
        write_group(dummy_scanner, wr, false, grp);
    }
    return wr.buf;
}

////////////////////
} }
#endif // _LIGHTCONF_CONFIG_FORMAT_H_

