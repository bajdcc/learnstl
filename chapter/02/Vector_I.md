# 第二章：变长数组（一）

## 地位

变长数组是比较基础的数据结构，对初学者而言，是它是它就是它！

假如没有循环语句去遍历，那么数组也就不会那么广为人知。数组在生活中也有体现，如报表、清单等等，它意味着连续的空间。大家知道内存其实就是一片连续的空间，所以内存也是一个巨大的数组，不过由于某种原因，你只能访问有限的空间。

静态分配的定长数组，那没问题，它长度永远不变，在编译期就可以安排好，没毛病。不过定长数组功能有限，假如要读取全班成绩，没读到尾巴之前，谁也不知道有多少个学生。除非知道了大致的学生总数，先分配空间；否则定长数组就无用武之地。

为了解决定长数组的局限性，变长数组应运而生。说是变长，其实是“假”变长。先分配好空间，结果空间不够了，这时候就尴尬了：“老大，留还是撤？”。留的话，不知道后面还有没有空间，结果生死未卜；战略转移的话，很有可能找到新家，就这样决定了！

怎么个“战略转移”法？就是找处新地方，将原来的数据搬过去。

## 基础

### 向量

在这里，类名是*vector*即向量。

了解一下向量的基本操作：

- 增：插入至头，插入至中间，添加至末尾，插入连续的数据
- 删：删除头、尾、中间、连续区域
- 改：修改数据，修改预设大小
- 查：查询某个位置的数据，获取数组的有效长度

本节是第一节，主要介绍vector的基本功能。后续“增删改查”的环节会添加迭代器。

## 思路

在先前的内存池基础上实现简单的vector。

## 详解

### 一、再分配

补上先前缺少的重新分配内存方法。

思路：在内存池中查找指定大小的块，如果没有就报错，如果有的话，就搬运这些数据，最后把原有内存释放。

```c++
// 重新分配内存
void* _realloc(void* p, uint newSize, uint clsSize)
{
	block *blk = static_cast<block*>(p);
	--blk; // 自减得到块的元信息头
	if (!verify_address(blk))
		return nullptr;
	auto size = block_align(newSize * clsSize); // 计算新的内存大小
	auto _new = _alloc(size);
	if (!_new)
	{
		// 空间不足
		_free(blk);
		return nullptr;
	}
	auto oldSize = blk->size;
	memmove(_new, p, sizeof(block) * __min(oldSize, size)); // 移动内存
	_free(p);
	return _new;
}
```

### 二、设置参数

设置一些参数。

```c++
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
```

### 三、向量

目前只实现了向量的核心部分。

往向量中添加数据的时候，如果可用空间不够了，就去向内存池申请更大的一片空间。

```c++
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
```

### 六、单元测试

测试了添加和寻址功能，目前没啥问题。

## 源码

文章：https://github.com/bajdcc/learnstl/blob/master/chapter/02/Vector_I.md

源码：https://github.com/bajdcc/learnstl/blob/master/code/02/vector.h

## 总结

`vector`的编写过程还算轻松~