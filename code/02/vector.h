#ifndef __STD_VECTOR_H
#define __STD_VECTOR_H

#include "memory.h"

namespace clib
{
    namespace collections
    {
        using namespace type;

        namespace vector_config
        {
            // 最大空间
            static const size_t FULL_SIZE = 0x100000;
            // 共用内存池
            static memory::memory_pool<FULL_SIZE> mem;

            // 默认总容量
            static const size_t DEF_SIZE = 0x10;
            // 默认递增容量
            static const size_t ACC_SIZE = 0x10;
        }

        // 向量（变长数组）
        template<class T>
        class vector
        {
            using data_t = T;

            size_t capacity; // 所有空间
            size_t used; // 已用空间
            size_t acc; // 每次递增大小

            data_t *data; // 数据

            void extend()
            {
                capacity += acc;
                // 注意：扩充容量时，原有数据失效！
                data = vector_config::mem.realloc(data, capacity);
            }

        public:

            vector()
                : capacity(vector_config::DEF_SIZE)
                , used(0)
                , acc(vector_config::ACC_SIZE)
            {
                data = vector_config::mem.alloc_array<data_t>(capacity);
            }

            // 添加新元素至末尾
            void push(T&& obj)
            {
                if (used >= capacity)
                {
                    extend();
                }
                data[used++] = obj; // T类型的赋值拷贝
            }

            // 弹出末尾的元素
            T&& pop()
            {
                if (used == 0)
                    throw "Empty vector";
                return std::forward<T>(data[--used]); // 返回右值引用
            }

            // 获取元素
            T&& get(size_t index) const
            {
                if (index >= used)
                    throw "Invalid index";
                return std::forward<T>(data[index]); // 返回右值引用
            }

            // 获取最末尾元素
            T&& top() const
            {
                return get(used - 1);
            }

            // 得到大小
            size_t size() const
            {
                return used;
            }
        };
    }
}

#endif
