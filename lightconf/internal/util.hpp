#ifndef _LIGHTCONF_UTIL_H_
#define _LIGHTCONF_UTIL_H_

#include <sstream>

namespace lightconf {
////////////////////

//
//
inline std::string stringize_number(double num) {
    std::stringstream ss;
    ss << num;
    return ss.str();
}

//
//
inline std::string escape_string(const std::string& str) {
    const char *table = "0123456789abcdef";
    char utf8[7] = "\\u0000";
    std::string buf;

    for (unsigned char c : str) {
        switch (c) {
        case '\r': buf.append("\\r"); break;
        case '\n': buf.append("\\n"); break;
        case '\\': buf.append("\\\\"); break;
        case '/':  buf.append("\\/"); break;
        case '"':  buf.append("\\\""); break;
        case '\f': buf.append("\\f"); break;
        case '\b': buf.append("\\b"); break;
        case '\t': buf.append("\\t"); break;
        default:
            if (c < 0x20 || c == 0x7f) {
                utf8[4] = table[c >> 4];
                utf8[5] = table[c & 0xf];
                buf.append(utf8);
            } else {
                buf.append(1, c);
            }
        }
    }

    return buf;
}


////////////////////
}
#endif // _LIGHTCONF_UTIL_H_

