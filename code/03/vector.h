#ifndef _VECTOR_H
#define _VECTOR_H

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <iostream>

#ifndef INTSIZEOF
#define INTSIZEOF(n) ((sizeof(n)+sizeof(int)-1)&~(sizeof(int) - 1))
#endif

/**
 * 变长数组
 * @tparam T 数据类型
 */
template<class T>
class Vector {
private:
    T *data; // 数据
    int length; // 使用的长度
    size_t capacity; // 容量

    static const int INIT_SIZE = 4; // 一开始的大小
    static const size_t N_SIZE = INTSIZEOF(T); // 对齐的T结构大小

private:
    /**
     * 用t填充初始化后的数组
     * @param t 填充值
     */
    void fill(const T &t) {
        for (int i = 0; i < length; ++i) {
            data[i] = t;
        }
    }

    void print_info() const {
        std::cerr << "[VECTOR INFO] capacity: " << capacity << ", length: " << length << std::endl;
    }

    void error_index(const char *str, int index) const {
        std::cerr << "[VECTOR ERROR] " << str << ": invalid index " << index << std::endl;
        print_info();
        assert(!"ERROR");
        exit(-1);
    }

public:
    Vector() : data((T *) calloc(INIT_SIZE, N_SIZE)), capacity(INIT_SIZE), length(0) {}

    Vector(int size) : Vector() { resize(size); }

    Vector(int size, const T &t) : Vector(size) { fill(t); }

    Vector(const Vector &v) : data((T *) calloc(v.capacity, N_SIZE)), capacity(v.capacity), length(v.length) {
        memcpy(data, v.data, N_SIZE * length);
    }

    ~Vector() { free(data); }

    int get_size() const { return length; }

    size_t get_capacity() const { return capacity; }

    const T *get_data() const { return data; }

    Vector<T> &operator+=(const T &t) {
        return (add(t), *this);
    }

    Vector<T> &operator=(const Vector<T> &v) {
        free(data);
        data = ((T *) calloc(v.capacity, N_SIZE));
        capacity = v.capacity;
        length = v.length;
        memcpy(data, v.data, N_SIZE * length);
        return *this;
    }

    T &operator[](int index) {
        if (index < 0 || index >= length) {
            error_index("get", index);
        }
        return data[index];
    }

    /**
     * 更改数组大小
     * @param size 新的大小
     */
    void resize(int size) {
        if (size >= capacity) { // 容量不够 就进行扩张
            auto n = size / capacity; // 算几倍
            n <<= 1; // 增长一倍
            capacity *= n;
            auto new_size = N_SIZE * capacity;
            auto new_data = (T *) realloc(data, new_size); // 重新申请
            if (!new_data) {
                new_data = (T *) calloc(capacity, N_SIZE); // realloc失败 重新申请 手动拷贝
                assert(new_data);
                memcpy(new_data, data, new_size); // 拷贝数据
                free(data); // 释放原来的数组
            }
            data = new_data;
        }
        length = size;
    }

    /**
     * 添加数据
     * @param t 新增在数组末尾的数据
     */
    void add(const T &t) {
        data[length] = t;
        length++;
        resize(length); // 确保容量够用
    }

    /**
     * 插入数据
     * @param index 位置
     * @param t 插入到指定位置的数据
     */
    void insert(int index, const T &t) {
        if (index < 0 || index > length) {
            error_index("insert(index)", index);
        }
        if (index == length) { // 相当于add
            add(t);
            return;
        }
        memmove(&data[index + 1], &data[index], N_SIZE * (length - index)); // 移动部分数组，腾出空间
        data[index] = t; // 放置数据
        length++;
        resize(length); // 确保容量够用
    }

    /**
     * 删除数据
     * @param index 要删除的位置
     */
    void remove(int index) {
        remove(index, 1);
    }

    /**
     * 删除数据
     * @param index 要删除的位置
     * @param len 删除的片段长度
     */
    void remove(int index, int len) {
        if (index < 0 || index >= length) {
            error_index("remove(index)", index);
        }
        if (len <= 0 || index + len > length) { // 判断len合法
            error_index("remove(len)", len);
        }
        for (auto i = 0; i < len; ++i) {
            data[index + i].~T();
        }
        memmove(&data[index], &data[index + len], N_SIZE * (length - index - len)); // 搬运数组
        length -= len;
    }

    /**
     * 获取数据
     * @param index 位置
     * @return 数据
     */
    const T &get(int index) const {
        if (index < 0 || index >= length) {
            error_index("get", index);
        }
        return data[index];
    }

    /**
     * 设置数据
     * @param index 位置
     * @param t 数据
     */
    void set(int index, const T &t) const {
        if (index < 0 || index >= length) {
            error_index("set", index);
        }
        data[index] = t;
    }

    /**
     * 清空所有内容
     */
    void clear() {
        for (auto i = 0; i < length; ++i) {
            data[i].~T();
        }
        length = 0;
    }
};

#endif
