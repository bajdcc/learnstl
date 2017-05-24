#ifndef _STRING_H
#define _STRING_H

#include "vector.h"

/**
 * 字符串
 * @tparam T 字符类型
 */
template<class T>
class String {
private:
    Vector<T> data; // 字符数组

    static const int CHAR_SIZE = sizeof(T); // 字符大小

private:
    /**
     * 从字符串指针中创建对象
     * @param str 字符串指针
     */
    void copy(const T *str) {
        T c;
        while (c = *str++) {
            data.add(c);
        }
    }

    /**
     * 设置最后一个字符为零
     */
    void set_end() const {
        const_cast<T*>(get_data())[get_length()] = '\0'; // 由于边界检查，只好出此下策 :(
    }

public:
    String() {}

    String(const T *str) { copy(str); }

    String(int size, const T &t) : data(size, t) {}

    String(const String &s) : data(s.data) {}

    int get_length() const { return data.get_size(); }

    int is_empty() const { return data.get_size() == 0; }

    const T *get_data() const {
        return data.get_data();
    }

    String<T> &operator+(const T &t) { // string + char
        return (data.add(t), *this);
    }

    friend String<T> &operator+(const T &t, const String<T> &s) { // char + string
        return s + t;
    }

    String<T> &operator+=(const T &t) { // string += char
        return *this + t;
    }

    String<T> &operator=(const String<T> &s) { // string = string2
        return (data = s.data, *this);
    }

    String<T> &operator=(const T *str) { // string = "abc"
        return (copy(str), *this);
    }

    bool operator==(const String<T> &s) {
        return compare(s) == 0;
    }

    bool operator!=(const String<T> &s) {
        return compare(s) != 0;
    }

    bool operator>(const String<T> &s) {
        return compare(s) > 0;
    }

    bool operator<(const String<T> &s) {
        return compare(s) < 0;
    }

    bool operator>=(const String<T> &s) {
        return compare(s) >= 0;
    }

    bool operator<=(const String<T> &s) {
        return compare(s) <= 0;
    }

    operator const T *() const { // 便于格式转换
        return get_data();
    }

    T &operator[](int index) { // 允许修改
        return data[index];
    }

    /**
     * 字符串比较
     * @param s 要比较的字符串
     * @return 相等为0，小于为-1，大于为1
     */
    int compare(const String<T> &s) const {
        auto a_len = get_length();
        auto b_len = s.get_length();
        auto min_len = a_len > b_len ? b_len : a_len;
        for (auto i = 0; i < min_len; ++i) {
            if ((*this)[i] < s[i]) {
                return -1;
            } else if ((*this)[i] > s[i]) {
                return 1;
            }
        }
        if (a_len < b_len) {
            return -1;
        } else if (a_len > b_len) {
            return 1;
        }
        return 0;
    }

    /**
     * 生成子串
     * @param start 起始位置
     * @param length 子串长度
     * @return 子串
     */
    String<T> substr(int start, int length = -1) const {
        /* (GCC) 注意，这里编译器进行NRVO优化
         * 如果我这么做：
         *   auto a = String<char>("hello world");
         *   auto b = a.substr(4,4);
         * 则自始至终，只有a和b分别调用一次默认构造函数
         * 不会调用拷贝构造以及赋值构造
         * 因此一般情况下没有必要涉及移动语义
         */
        String<T> s;
        if (length == -1) length = get_length();
        auto end = (start + length) > get_length() ? get_length() : (start + length);
        for (auto i = start; i < end; ++i) {
            s += (*this)[i];
        }
        s += '\0';
        s.remove(s.get_length() - 1); // 末尾置零
        return s; // 优化掉了，不会调用构造函数
    }

    /**
     * 生成从左开始的子串
     * @param length 子串长度
     * @return 子串
     */
    String<T> left(int length) const {
        return substr(0, length);
    }

    /**
     * 生成中间的子串
     * @param start 起始位置
     * @param end 结束位置
     * @return 子串
     */
    String<T> mid(int start, int end) const {
        return substr(start, end - start + 1);
    }

    /**
     * 生成从右开始的子串
     * @param length 子串长度
     * @return 子串
     */
    String<T> right(int length) const {
        return substr(get_length() - length);
    }

    /**
     * 查询字符
     * @param index 位置
     * @return 指定位置的字符
     */
    const T &get(int index) const { // 禁止修改
        return data.get(index);
    }

    /**
     * 添加字符
     * @param t 字符
     */
    void add(const T &t) {
        (*this) += t;
        set_end();
    }

    /**
     * 插入字符
     * @param index 位置
     * @param t 插入到指定位置的字符
     */
    void insert(int index, const T &t) {
        data.insert(index, t);
    }

    /**
     * 删除指定位置的字符
     * @param index 要删除的位置
     */
    void remove(int index) {
        data.remove(index);
        set_end();
    }

    /**
     * 删除指定位置的连续字符
     * @param index 要删除的位置
     * @param len 删除的字符串长度
     */
    void remove(int index, int len) {
        data.remove(index, len);
        set_end();
    }

    /**
     * 清除指定字符
     * @param t 字符
     */
    void erase(const T &t) {
        for (auto i = get_length() - 1; i >= 0; --i) {
            if ((*this)[i] == t) {
                remove(i);
            }
        }
    }

    /**
     * 清空字符串
     */
    void clear() {
        if (!is_empty()) {
            data.clear();
            set_end();
        }
    }

    /**
     * 事先预留一定空间来存放字符数组
     * @param size 字符数组大小
     */
    void reserve(int size) {
        data.resize(size);
    }

    /**
     * 字符串翻转
     * @return 翻转后的字符串
     */
    String<T> reverse() {
        auto s(*this);
        auto length = get_length();
        for (int i = 0; i < (length + 1) / 2; ++i) {
            s[i] = s[length - i - 1];
        }
        return s;
    }

    /**
     * 字符串分割
     * @param t 分隔符
     * @return 字符串数组
     */
    Vector<String<T>> split(const T &t) const {
        Vector<String<T>> v;
        auto start = 0;
        auto length = 0;
        for (auto i = 0; i < get_length(); ++i) {
            if ((*this)[i] == t) {
                if (length > 0) {
                    v += substr(start, length);
                    length = 0;
                }
                start = i + 1;
            } else {
                length++;
            }
        }
        if (length > 0)
            v += substr(start, length);
        return v;
    }

    // -------------------------------------------------

    // PS: 更加高级的find和replace用正则表达式实现

    /**
     * 查找第一次出现的指定字符位置
     * @param t 字符
     * @return 位置，-1则不存在
     */
    int find(const T &t) const {
        for (auto i = 0; i < get_length(); ++i) {
            if ((*this)[i] == t)
                return i;
        }
        return -1;
    }

    /**
     * 查找最后一次出现的指定字符位置
     * @param t 字符
     * @return 位置，-1则不存在
     */
    int find_last(const T &t) const {
        auto index = -1;
        for (auto i = 0; i < get_length(); ++i) {
            if ((*this)[i] == t)
                index = i;
        }
        return index;
    }

    /**
     * 查找除指定字符以外的第一次出现的字符位置
     * @param t 字符
     * @return 位置，-1则不存在
     */
    int find_not(const T &t) const {
        for (auto i = 0; i < get_length(); ++i) {
            if ((*this)[i] != t)
                return i;
        }
        return -1;
    }

    /**
     * 查找除指定字符以外的最后一次出现的字符位置
     * @param t 字符
     * @return 位置，-1则不存在
     */
    int find_last_not(const T &t) const {
        auto index = -1;
        for (auto i = 0; i < get_length(); ++i) {
            if ((*this)[i] != t)
                index = i;
        }
        return index;
    }

    /**
     * 查找第一次出现的指定字符位置（从右往左）
     * @param t 字符
     * @return 位置，-1则不存在
     */
    int findr(const T &t) const {
        for (auto i = get_length() - 1; i >= 0; --i) {
            if ((*this)[i] == t)
                return i;
        }
        return -1;
    }

    /**
     * 查找最后一次出现的指定字符位置（从右往左）
     * @param t 字符
     * @return 位置，-1则不存在
     */
    int findr_last(const T &t) const {
        auto index = -1;
        for (auto i = get_length() - 1; i >= 0; --i) {
            if ((*this)[i] == t)
                index = i;
        }
        return index;
    }

    /**
     * 查找最后一次出现的指定字符位置（从右往左）
     * @param t 字符
     * @return 位置，-1则不存在
     */
    int findr_not(const T &t) const {
        for (auto i = get_length() - 1; i >= 0; --i) {
            if ((*this)[i] == t)
                return i;
        }
        return -1;
    }

    /**
     * 查找除指定字符以外的最后一次出现的字符位置（从右往左）
     * @param t 字符
     * @return 位置，-1则不存在
     */
    int findr_last_not(const T &t) const {
        auto index = -1;
        for (auto i = get_length() - 1; i >= 0; --i) {
            if ((*this)[i] != t)
                index = i;
        }
        return index;
    }

    // -------------------------------------------------

    /**
     * 查找第一次出现的指定字符位置
     * @param t 字符
     * @param start 起始查找位置
     * @return 位置，-1则不存在
     */
    int find(const T &t, int start) const {
        for (auto i = start; i < get_length(); ++i) {
            if ((*this)[i] == t)
                return i;
        }
        return -1;
    }

    /**
     * 查找最后一次出现的指定字符位置
     * @param t 字符
     * @param start 起始查找位置
     * @return 位置，-1则不存在
     */
    int find_last(const T &t, int start) const {
        auto index = -1;
        for (auto i = start; i < get_length(); ++i) {
            if ((*this)[i] == t)
                index = i;
        }
        return index;
    }

    /**
     * 查找除指定字符以外的第一次出现的字符位置
     * @param t 字符
     * @param start 起始查找位置
     * @return 位置，-1则不存在
     */
    int find_not(const T &t, int start) const {
        for (auto i = start; i < get_length(); ++i) {
            if ((*this)[i] != t)
                return i;
        }
        return -1;
    }

    /**
     * 查找除指定字符以外的最后一次出现的字符位置
     * @param t 字符
     * @param start 起始查找位置
     * @return 位置，-1则不存在
     */
    int find_last_not(const T &t, int start) const {
        auto index = -1;
        for (auto i = start; i < get_length(); ++i) {
            if ((*this)[i] != t)
                index = i;
        }
        return index;
    }

    /**
     * 查找第一次出现的指定字符位置（从右往左）
     * @param t 字符
     * @param start 起始查找位置（从右往左数）
     * @return 位置，-1则不存在
     */
    int findr(const T &t, int start) const {
        for (auto i = get_length() - 1 - start; i >= 0; --i) {
            if ((*this)[i] == t)
                return i;
        }
        return -1;
    }

    /**
     * 查找最后一次出现的指定字符位置（从右往左）
     * @param t 字符
     * @param start 起始查找位置（从右往左数）
     * @return 位置，-1则不存在
     */
    int findr_last(const T &t, int start) const {
        auto index = -1;
        for (auto i = get_length() - 1 - start; i >= 0; --i) {
            if ((*this)[i] == t)
                index = i;
        }
        return index;
    }

    /**
     * 查找最后一次出现的指定字符位置（从右往左）
     * @param t 字符
     * @param start 起始查找位置（从右往左数）
     * @return 位置，-1则不存在
     */
    int findr_not(const T &t, int start) const {
        for (auto i = get_length() - 1 - start; i >= 0; --i) {
            if ((*this)[i] == t)
                return i;
        }
        return -1;
    }

    /**
     * 查找除指定字符以外的最后一次出现的字符位置（从右往左）
     * @param t 字符
     * @param start 起始查找位置（从右往左数）
     * @return 位置，-1则不存在
     */
    int findr_last_not(const T &t, int start) const {
        auto index = -1;
        for (auto i = get_length() - 1 - start; i >= 0; --i) {
            if ((*this)[i] != t)
                index = i;
        }
        return index;
    }

    // -------------------------------------------------
};

using StringA = String<char>;
using StringW = String<wchar_t>;

#endif
