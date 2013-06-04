#ifndef _LIGHTCONF_WRITER_H_
#define _LIGHTCONF_WRITER_H_

#include <string>
#include <stack>

namespace lightconf {
////////////////////

//
//
struct writer {
    std::string         buf;
    std::stack<int>     tabstops;
    int                 line;
    int                 col;
    int                 wrap_length;

    void                indent() { tabstops.push(tabstops.top() + 4); }
    void                indent_to(int pos) { tabstops.push(pos); }
    void                unindent() { tabstops.pop(); }

    void                newline();
    void                append(const std::string& str);

    writer() :
        tabstops({0}),
        line(0),
        col(0)
    { }
};

//
//
void writer::newline() {
    append("\n");
    append(std::string(tabstops.top(), ' '));
    line++;
    col = tabstops.top();
}

//
//
void writer::append(const std::string& str) {
    buf.append(str);
    col += str.size();
}

////////////////////
}

#endif // _LIGHTCONF_WRITER_H_
