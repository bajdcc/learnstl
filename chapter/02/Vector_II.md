# 第二章：变长数组（二）

## 前言

先前的vector是建立在memory pool的基础上的，带有点实验的性质。那么我们将重写vector，就用calloc等来实现。

## 介绍

**<u>私有数据</u>**

- 数据指针：data
- 有效长度：length
- 当前容量：capacity

**<u>公有方法</u>**

- 构造函数
  - 无参：初始化长度为4的数组
  - 带指定长度：按照指定长度初始化
  - 带指定长度和初始化值：按照指定长度以指定值初始化
- Get方法
  - 对应三个私有数据的Get方法
  - 数组寻址：返回index所在位置的元素
- Set方法
  - 设置index所在位置上的元素
- 常用方法
  - resize 重新设置大小：如果新值依然小于容量，则pass；如果不小于容量，则扩张。扩张的方法：设当前大小为size，<u>找到大于size的最小的2的幂</u>，设成capacity。由于data是calloc来的，所以先用realloc尝试一下，如果不行，再重新calloc并搬运数据。
  - add 在末尾添加
  - insert 在指定位置插入：插入前要进行部分数组移动的操作
  - remove 删除指定位置元素，可以设定删除的那批元素的长度

## 代码

```c++
#ifndef _VECTOR_H
#define _VECTOR_H

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <iostream>

/**
 * 变长数组
 * @tparam T 数据类型
 */
template<class T>
class Vector {
private:
    T *data; // 数据
    int length; // 使用的长度
    int capacity; // 容量

    static const int INIT_SIZE = 4; // 一开始的大小

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
    Vector() : data((T *) calloc(INIT_SIZE, sizeof(T))), capacity(INIT_SIZE), length(0) {}

    Vector(int size) : Vector() { resize(size); }

    Vector(int size, const T &t) : Vector(size) { fill(t); }

    ~Vector() { free(data); }

    int get_size() const { return length; }

    int get_capacity() const { return capacity; }

    T *get_data() const { return data; }

    /**
     * 更改数组大小
     * @param size 新的大小
     */
    void resize(int size) {
        if (size >= capacity) { // 容量不够 就进行扩张
            auto n = size / capacity; // 算几倍
            n <<= 1; // 增长一倍
            capacity *= n;
            auto new_size = sizeof(T) * capacity;
            auto new_data = (T *) realloc(data, new_size); // 重新申请
            if (!new_data) {
                new_data = (T *) calloc(capacity, sizeof(T)); // realloc失败 重新申请 手动拷贝
                assert(new_data); // 再次失败则gameover
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
        memmove(&data[index + 1], &data[index], sizeof(T) * (length - index)); // 移动部分数组，腾出空间
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
        memmove(&data[index], &data[index + len], sizeof(T) * (length - index - len)); // 搬运数组
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
};

#endif
```

## 代码中涉及的知识

掌握等级：

1. 基础
2. 进阶
3. 高阶

列表：

- C
   1. (Lv.1)ifndef宏
   2. (Lv.1)include
   3. (Lv.1)指针
   4. (Lv.1)static
   5. (Lv.1)sizeof
   6. (Lv.1)移位 <<=
   7. (Lv.2)calloc, free, realloc
   8. (Lv.2)memcpy, memmove
   9. (Lv.2)assert断言
- C++
   1. (Lv.1)数据与方法
   2. (Lv.1)访问范围 public private
   3. (Lv.1)const修饰
   4. (Lv.2)引用
   5. (Lv.2)[C++11]委托构造函数
   6. (Lv.3)类模版

<u>PS：只讲究实用，没必要把C++一大堆特性塞进去。</u>

如果要完善一下代码的话，可以再加些：
      1. 拷贝构造
      2. 初始化列表构造
      3. delete修饰
      4. 运算符重载
      5. ...

## 后续目标

1. 用vector来实现string
2. 用string来实现stream
3. 实现LINQ类似的数组查询方式