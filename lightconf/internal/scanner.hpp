#ifndef _LIGHTCONF_SCANNER_H_
#define _LIGHTCONF_SCANNER_H_

#include <cstdlib>
#include <functional>
#include <string>
#include "exceptions.hpp"

namespace lightconf {
////////////////////

struct token;
class scanner;

//
//
enum scanner_flags {
    empty_flag                  = 0x00,
    allow_comments_flag         = 0x01,     
    utf8_exceptions_flag        = 0x02,     
    blank_line_comment_flag     = 0x04
};

//
//
enum class token_type {
    none_token,
    string_token,
    identifier_token,
    number_token,
    char_token,
    whitespace_token,
    comment_token,
    eof_token
};

typedef std::function<void (const std::string&)> comment_function_t;

//
//
struct token {
    token_type          type;
 
    std::string         string_value;
    double              number_value;
    char                char_value;

    int                 pos;
    int                 line;
    int                 col;

    bool                is_char(char c) const { return type == token_type::char_token && char_value == c; }

    explicit token(token_type type = token_type::none_token) :
        type(type),
        string_value(),
        number_value(),
        char_value(),
        pos(),
        line(),
        col()
    { }
};

//
//
struct scanner_params {
    comment_function_t  comment_function;
    int                 flags;
};


const scanner_params default_scanner_params = {
    nullptr,
    allow_comments_flag | blank_line_comment_flag
};

//
//
class scanner {
public:
    void                scan(const std::string& input, scanner_params params = default_scanner_params);

    void                next_token();
    void                skip_whitespace(bool ignore_comments);
    void                expect(token_type type, bool optional = false);
    void                expect(char c, bool optional = false);
    std::string         expect_string();
    std::string         expect_identifier();
    double              expect_number();

    bool                token_available() const;
    token               cur_token() const;
    token               peek_token() const;

    void                fail(const std::string& message, int line, int col) const;

    scanner();
private:
    bool                eof() const;
    char                ch() const;
    void                next_ch();
    std::string         utf8_unescape();

    std::string         scan_identifier();
    std::string         scan_string();
    double              scan_number();

    char32_t            try_read_hex(bool *failed);
    
    std::string         input_;
    std::vector<token>  tokens_;
    unsigned int        cur_token_;
    bool                eof_;

    unsigned int        pos_;
    int                 line_;
    int                 col_;

    scanner_params      params_;
};

//
// Convert a 32-bit wide character to a UTF-8 byte sequence
inline std::string wc_to_utf8(char32_t ch) {
    char temp[5] = { 0, 0, 0, 0, 0 };
    if (ch < 0x80) {
        temp[0] = (char)ch;
    } else if (ch < 0x800) {
        temp[0] = (ch>>6) | 0xc0;
        temp[1] = (ch & 0x3f) | 0x80;
    } else if (ch < 0x10000) { 
        temp[0] = (ch>>12) | 0xe0;
        temp[1] = ((ch>>6) & 0x3f) | 0x80;
        temp[2] = (ch & 0x3f) | 0x80;
    } else if (ch < 0x110000) {
        temp[0] = (ch>>18) | 0xf0;
        temp[1] = ((ch>>12) & 0x3f) | 0x80;
        temp[2] = ((ch>>6) & 0x3f) | 0x80;
        temp[3] = (ch & 0x3f) | 0x80;
    }
    return std::string(temp);
}

//
//
inline std::string token_name(const token& tok) {
    switch (tok.type) {
    case token_type::none_token: return "<none>";
    case token_type::identifier_token: return "identifier";
    case token_type::string_token: return "string";
    case token_type::number_token: return "number";
    case token_type::char_token: return "'" + std::string(1, tok.char_value) + "'";
    case token_type::comment_token: return "comment";
    case token_type::whitespace_token: return "whitespace";
    case token_type::eof_token: return "eof";
    default: return "<unknown>";
    }
}

//
//
inline scanner::scanner() :
    cur_token_(0),
    eof_(true),
    pos_(0),
    line_(1),
    col_(1),
    params_({ comment_function_t(), empty_flag })
{ }

//
//
inline bool scanner::token_available() const {
    return cur_token_ <= tokens_.size() - 1;
}

//
//
inline token scanner::cur_token() const {
    if (cur_token_ == tokens_.size()) {
        token tok;
        tok.type = token_type::eof_token;
        return tok;
    }
    return tokens_[cur_token_];
}

//
//
inline token scanner::peek_token() const {
    unsigned int i = cur_token_;
    while (i < tokens_.size()) {
        if (tokens_[i].type != token_type::whitespace_token && tokens_[i].type != token_type::comment_token) {
            break;
        }
        i++;
    }
    if (i == tokens_.size()) {
        return token(token_type::eof_token);
    }
    return tokens_[i];
}

//
//
inline bool scanner::eof() const {
    return eof_;
}

//
//
inline char scanner::ch() const {
    if (eof()) {
        return 0;
    }
    return input_[pos_];
}

//
//
inline void scanner::next_ch() {
    if (eof()) return;

    col_++;
    if (ch() == '\n') {
        line_++;
        col_ = 1;
    }

    pos_++;
    if (pos_ >= input_.length()) {
        eof_ = true;
    }
}

//
//
inline void scanner::scan(const std::string& input, scanner_params params) {
    input_ = input;
    pos_ = 0;
    line_ = 1;
    col_ = 1;
    cur_token_ = 0;
    eof_ = input.empty();
    params_ = params;

    while (!eof()) {
        token tok;
        tok.pos = pos_;
        tok.line = line_;
        tok.col = col_;

        switch (ch()) {
        case '"': 
            tok.type = token_type::string_token;
            tok.string_value = scan_string();
            break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case '-': case '.':
            tok.type = token_type::number_token;
            tok.number_value = scan_number(); 
            break;

        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
        case 's': case 't': case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
            tok.type = token_type::identifier_token;
            tok.string_value = scan_identifier();
            break;

        case '/':
            if (params_.flags & allow_comments_flag && pos_ < input_.size() - 1 && input[pos_ + 1] == '/') {
                tok.type = token_type::comment_token;
                tok.string_value = "";
                while (!eof() && ch() != '\n') {
                    if (ch() != '\r') {
                        tok.string_value.append(1, ch());
                    }
                    next_ch();
                }
            } else {
                tok.type = token_type::char_token;
                tok.char_value = '/';
                next_ch();
            }
            break;

        default:
            if (ch() <= 0x20) {
                bool newline_seen = false;

                tok.type = token_type::whitespace_token;
                tok.string_value = "";

                while (!eof() && ch() <= 0x20) {
                    // If we have more than one newline in a block of whitespace, we consider that
                    // a blank comment (if the appropriate scanner flag is enabled)
                    if (ch() == '\n') {
                        if (newline_seen && params_.flags & blank_line_comment_flag) {
                            token blank_line;
                            blank_line.type = token_type::comment_token;
                            blank_line.string_value = "";
                            tokens_.push_back(tok);
                            tokens_.push_back(blank_line);
                            next_ch();
                            continue;
                        }
                        newline_seen = true;
                    }
                    tok.string_value.append(1, ch());
                    next_ch();
                }
            } else {
                tok.type = token_type::char_token;
                tok.char_value = ch();
                next_ch();
            }
            break;
        }

        tokens_.push_back(tok);
    }
}

//
//
inline void scanner::next_token() {
    if (token_available()) {
        cur_token_++;
    }
}

//
//
inline void scanner::skip_whitespace(bool ignore_comments) {
    while (cur_token().type == token_type::whitespace_token || cur_token().type == token_type::comment_token) {
        if (cur_token().type == token_type::comment_token && !ignore_comments) {
            if (params_.comment_function) {
                params_.comment_function(cur_token().string_value);
            }
        }
        next_token();
    }
}

//
//
inline void scanner::expect(token_type tok, bool optional) {
    if (peek_token().type == tok) {
        skip_whitespace(false);
        next_token();
    } else {
        if (!optional) {
            fail("expected " + token_name(token(tok)) + " but found " + token_name(cur_token()),
                cur_token().line, cur_token().col);
        }
    }
}

//
//
inline void scanner::expect(char c, bool optional) {
    if (peek_token().is_char(c)) {
        skip_whitespace(false);
        next_token();
    } else {
        if (!optional) {
            fail("expected '" + std::string(1, c) + "' but found " + token_name(cur_token()),
                cur_token().line, cur_token().col);
        }
    }
}

//
//
inline std::string scanner::expect_string() {
    skip_whitespace(false);
    if (cur_token().type != token_type::string_token) {
        fail("expected " + token_name(token(token_type::string_token)) + " but found " + token_name(cur_token()),
            cur_token().line, cur_token().pos);
    }
    std::string val = cur_token().string_value;
    next_token();
    return val;
}

//
//
inline std::string scanner::expect_identifier() {
    skip_whitespace(false);
    if (cur_token().type != token_type::identifier_token) {
        fail("expected " + token_name(token(token_type::identifier_token)) + " but found " + token_name(cur_token()),
            cur_token().line, cur_token().pos);
    }
    std::string val = cur_token().string_value;
    next_token();
    return val;
}

//
//
inline double scanner::expect_number() {
    skip_whitespace(false);
    if (cur_token().type != token_type::number_token) {
        fail("expected " + token_name(token(token_type::number_token)) + " but found " + token_name(cur_token()),
            cur_token().line, cur_token().pos);
    }
    double val = cur_token().number_value;
    next_token();
    return val;
}

//
//
inline std::string scanner::scan_identifier() {
    std::string buf;
    while ((ch() >= 'A' && ch() <= 'Z')
        || (ch() >= 'a' && ch() <= 'z')
        || (ch() >= '0' && ch() <= '9')
        || ch() == '_' || ch() == '-')
    {
        buf += ch();
        next_ch();
    }
    return buf;
}


//
//
inline std::string scanner::scan_string() {
    std::string buf;
    next_ch();
    while (ch() != '"') {
        if (eof_) {
            fail("unclosed string literal", line_, col_);
        }
        if (ch() == '\r' || ch() == '\n') {
            fail("newline in string literal", line_, col_);
        }
        if (ch() == '\\') {
            next_ch();
            if (ch() == 'u') {
                next_ch();
                buf += utf8_unescape();
            } else {
                switch (ch()) {
                case '"':  buf += '"'; break;
                case '\\': buf += '\\'; break;
                case '/':  buf += '/'; break;
                case 'b':  buf += '\b'; break;
                case 'f':  buf += '\f'; break;
                case 't':  buf += '\t'; break;
                case 'r':  buf += '\r'; break;
                case 'n':  buf += '\n'; break;
                default:   buf += ch(); break;
                }
                next_ch();
            }
        } else {
            buf += ch();
            next_ch();
        }
    }
    next_ch();
    return buf;
}

//
//
inline double scanner::scan_number() {
    const char *ptr = input_.c_str() + pos_;
    char *endptr;
    double val = strtod(ptr, &endptr);
    int diff = endptr - ptr;
    for (int i = 0; i < diff; i++) {
        next_ch();
    }
    return val;
}

//
//
inline std::string scanner::utf8_unescape() {
    bool failed;

    char32_t uch = try_read_hex(&failed);
    char32_t uch2 = 0;
    std::string utf8;
    if (failed) {
        goto invalid;
    }

    // check for utf-16 surrogate pairs
    if (uch >= 0xd800 && uch <= 0xdfff) {
        // second half without first half
        if (uch >= 0xdc00) {
            goto invalid;
        }
        // check to make sure that the second half follows the first half
        if (ch() != '\\' || pos_ == input_.size() - 1 || input_[pos_+1] != 'u') {
            goto invalid;
        }
        next_ch(); next_ch(); // skip the \u
        uch2 = try_read_hex(&failed);
        if (failed) {
            goto invalid;
        }
        // is valid second half?
        if (uch2 < 0xdc00 || uch2 > 0xdfff) {
            goto invalid;
        }
        uch = 0x10000 + (((uch - 0xd800) << 10) | ((uch2 - 0xdc00) & 0x3ff));
    }

    utf8 = wc_to_utf8(uch);
    if (utf8 == "") {
        goto invalid;
    } else {
        return utf8;
    }

invalid:
    if (params_.flags & utf8_exceptions_flag) {
        throw utf8_error("invalid unicode escape character", line_, col_);
    } else {
        return u8"\ufffd"; // return a replacement character
    }
}

//
//
inline char32_t scanner::try_read_hex(bool *failed) {
    char32_t uch = 0;
    for (int i = 0; i < 4; i++) {
        if (eof()) {
            *failed = true;
            return 0;
        }
        if (ch() >= '0' && ch() <= '9') {
            uch = (uch * 16) + (ch() - '0');
        } else if (ch() >= 'a' && ch() <= 'f') {
            uch = (uch * 16) + (ch() - 'a' + 0xa);
        } else if (ch() >= 'A' && ch() <= 'F') {
            uch = (uch * 16) + (ch() - 'A' + 0xa);
        } else {
            *failed = true;
            return 0;
        }
        next_ch();
    }

    return uch;
}


//
//
inline void scanner::fail(const std::string& message, int line, int col) const {
    throw parse_error(message, line, col);
}

////////////////////
}

#endif // _LIGHTCONF_SCANNER_H_

