#ifndef _REGEX_H
#define _REGEX_H

#include "string.h"

/**
 * 正则表达式
 * @tparam T 字符类型
 */
template<class T>
class Regex {
private:
    String<T> pat; // 正则表达式字符串
    int local{-1}; // 当前字符
    bool spec{false}; // 是否关键字
    int index{0}; // pat遍历下标
    int length; // pat长度

    enum recls_t {
        rc_dight = -10,
        rc_nondight = -11,
        rc_word = -20,
        rc_nonword = -21,
        rc_space = -30,
        rc_nonspace = -31,
        rc_end = -1,
    };

    enum nfacls_t {
        nc_nil = -1,
        nc_start = -2,
        nc_end = -3,
    };

    struct nfa_t {
        int begin, end, index, reserved;
    };

    Vector<nfa_t> nfa; // nfa

    int nfa_start, nfa_end;

private:
    void build_regex() {
        start();
        parse();
    }

    void start() {
        nfa += nfa_t{nc_nil, 2, nc_start, nc_nil};
        nfa += nfa_t{3, nc_nil, nc_end, nc_nil};
        nfa += nfa_t{0, nc_nil, nc_nil, nc_nil}; // start
        nfa += nfa_t{nc_nil, 1, nc_nil, nc_nil}; // end
        nfa_start = 2;
        nfa_end = 3;
    }

    void error(const char *str) const {
        std::cerr << "[REGEX ERROR] " << str << std::endl;
        assert(!"ERROR");
        exit(-1);
    }

    void parse() {
        next();
        while (local != rc_end) {
            if (spec) {
                if (local == '(') {
                    // match group start
                    parse();
                } else if (local == ')') {
                    next();
                    // match group end
                } else if (local == '?') {
                    next();
                    // match repeat
                    parse_repeat(0, 1);
                } else if (local == '*') {
                    next();
                    // match repeat
                    parse_repeat(0, -1);
                    if (spec && local == '?')
                        next();
                } else if (local == '+') {
                    next();
                    // match repeat
                    parse_repeat(1, -1);
                    if (spec && local == '?')
                        next();
                } else if (local == '{') {
                    next();
                    // match repeat
                    auto begin = 0, end = -1;
                    if (!isdigit(local)) error("invalid begin in {begin, end}");
                    do
                    {
                        begin = begin * 10 + (local - '0');
                        next();
                    } while (isdigit(local));
                    if (local == ',') {
                        next();
                        if (isdigit(local)) {
                            end = 0;
                            do
                            {
                                end = end * 10 + (local - '0');
                                next();
                            } while (isdigit(local));
                        }
                    }
                    if (spec && local == '}') {
                        next();
                        parse_repeat(begin, end);
                    } else {
                        error("invalid number in {begin, end}");
                    }
                    parse_repeat(begin, end);
                    if (spec && local == '?')
                        next();
                } else if (local == '[') {
                    next();
                    // match range
                    bool inv = false;
                    if (spec && local == '^') {
                        inv = true;
                        next();
                    }
                    while (!(spec && local == ']'))
                    {
                        next();
                    };
                    next();
                } else if (local == '|') {
                    next();
                } else {
                    error("invalid special character");
                }
            } else {
                next();
                // match char
            }
        }
    }

    void parse_repeat(int begin, int end) {

    }

    void next() {
        if (index >= length) {
            local = rc_end;
            return;
        }
        local = (int) pat[index++];
        if (local == '\\') {
            spec = false;
            if (index < length) {
                local = pat[index++];
                switch (local) {
                    case 'r':
                        local = '\r';
                        break;
                    case 'n':
                        local = '\n';
                        break;
                    case 'b':
                        local = '\b';
                        break;
                    case 'v':
                        local = '\v';
                        break;
                    case 'f':
                        local = '\f';
                        break;
                    case '\\':
                        local = '\\';
                        break;
                    case 'd':
                        local = rc_dight;
                        break;
                    case 'w':
                        local = rc_word;
                        break;
                    case 's':
                        local = rc_space;
                        break;
                    case 'D':
                        local = rc_nondight;
                        break;
                    case 'W':
                        local = rc_nonword;
                        break;
                    case 'S':
                        local = rc_nonspace;
                        break;
                    default:
                        break;
                }
            }
        } else {
            switch (local) {
                case '|':
                case '.':
                case '*':
                case '+':
                case '?':
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                    spec = true;
                    break;
                default:
                    spec = false;
                    break;
            }
        }
    }

public:
    explicit Regex(const String<T> &str) : pat(str), length(str.get_length()) { build_regex(); }
};

#endif
