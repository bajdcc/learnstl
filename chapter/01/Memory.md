# 第一章：内存管理

## 地位

程序=数据结构+算法，总体还是围绕内存进行各种操作。偌大的一块内存令人茫然，如同空白的棋盘一样，令人无从下手。然而，下棋有固定的套路，管理内存亦然。管理一块内存，粗看看是简单的活儿，但实际中体会一遍过后就感觉不那么简单。

接触C语言，那么对malloc和free一定不陌生，C++中的new和delete同样如此。有了这些方法，变长数组和对象的灵活创建也就成了可能。

想一想：假如没有了malloc、free，或是new和delete，哪些程序还能原模原样运行着？这个答案也就暗示着内存管理的地位如何。这部分内容和整个库的联系暗含其中，地位处于库的底层，依赖链条的起始端。因而，本部分是最先可以动手完成的内容。

## 基础

### 内存

所有的`struct`、`class`等内容都位于内存中。通过调用相关的方法可以向操作系统申请特定大小的内存。主要的内存管理操作这里就实现两种：申请和释放，另外还做了一些必要的安全检查。

### 内存池

这里其实也可以叫作对象池。内存池负责管理一块内存，完成日常的内存申请和释放操作，那么它的任务简不简单？并不简单！

申请和释放？一般人想到设个`flag`不就没事了。事实没有那么简单，因为会有一种叫作**内存碎片**的情况出现。

假设一开始全部是申请操作，所有内存都申请完了。接着要接二连三释放一半的内存。释放完后，出现一片片的空洞。可用空间还有一半，但是如何从一个个空洞中去找够用的内存块？值得深究。况且，申请的内存大小参差不齐，更加增加了难度。

### 实现

自己去实现一个内存池很有难度，好在有优秀的方法可以参考。

这里就采用**分块法**，即把内存分成一个个块，每个块有自己的属性：是否空闲、大小等。为了使块与块之间能够遍历，需要添加链表指针，这里就采用**双向链表**。内存的查找采用**初次匹配**的原则，而不是最佳匹配，后者需要花更多的时间。内存 的释放附有**合并空闲块**的任务。

## 思路

1. 建立原始类型`type`
2. 构建原始内存池`legacy_memory_pool<default_allocator>`，无块大小限制，双向链表
3. 完善`legacy_memory_pool`，实现`allocator`接口，实现`legacy_memory_pool_allocator`
4. 命名类型`using memory_pool = legacy_memory_pool<legacy_memory_pool_allocator>`
5. 编写单元测试，除bug
6. 思考需要改进的地方，这里没有实现`remalloc`和`calloc`

## 详解

### 一、原始类型

这里没有什么难点。需要注意32位和64位的区别。

简单起见，库的实现就先照顾32位系统。

```c++
namespace clib
{
    namespace type
    {
        using int8 = signed __int8;
        using uint8 = unsigned __int8;
        using int16 = signed __int16;
        using uint16 = unsigned __int16;
        using int32 = signed __int32;
        using uint32 = unsigned __int32;
        using int64 = signed __int64;
        using uint64 = unsigned __int64;

#ifdef WIN32
        using sint = int32;
        using uint = uint32;
#else
        using sint = int64;
        using uint = uint64;
#endif

        using byte = uint8;
        using size_t = uint;
    }
}
```

### 二、默认接口

这里调用了`new`和`delete`方法，没什么难点。

带`args`的方法意味着会调用构造函数。

```c++
template<size_t DefaultSize = 0x10000>
class default_allocator
{
public:
	static const size_t DEFAULT_ALLOC_BLOCK_SIZE = DefaultSize;

	template<class T>
	T* __alloc()
	{
		return new T;
	}

	template<class T>
	T* __alloc_array(uint size)
	{
		return new T[size];
	}

	template<class T, class ... TArgs>
	T* __alloc_args(TArgs ... args)
	{
		return new T(args);
	}

	template<class T, class ... TArgs>
	T* __alloc_array_args(uint size, TArgs ... args)
	{
		return new T[size];
	}

	template<class T>
	bool __free(T* t)
	{
		delete t;
		return true;
	}

	template<class T>
	bool __free_array(T* t)
	{
		delete[] t;
		return true;
	}
};
```

### 三、内存块

#### 块的属性

内存块包含四个数据成员：

- size：该块后面跟着的数据部分的大小（以块的大小为基本单位）
- flag：参数，比如要设置空闲位`BLOCK_USING`
- prev：前一节点
- next：后一节点

```c++
struct block
{
	size_t size; // 数据部分的大小
	uint flag;   // 参数
	block *prev; // 前指针
	block *next; // 后指针
};

enum block_flag
{
	BLOCK_USING = 0
};
```

#### 块的方法

方法有：

- block_align：块的大小对齐，单位是块的大小（这里是16byte，那么1=16byte）

- block_init：块的初始化

- block_connect：块的连接，将A和B连接起来，其中A是原本有的，B是新添加上去的

- block_merge：块的合并，合并后只有一个头部（块头~）

- block_set_flag和block_get_flag：设置标志位

### 四、内存池

#### 申请内存

构建一个循环遍历指针，不断遍历各个块，如果当前块是空闲的且空间足够，就开辟它！

将块分割后，将在当前块后面建立一个新的块。

```c++
// 申请内存
void* _alloc(size_t size)
{
	if (size == 0)
		return nullptr;
	auto old_size = size;
	size = block_align(size);
	if (size >= block_available_size)
		return nullptr;
	if (block_current == block_head)
		return alloc_free_block(size);
	auto blk = block_current;
	do
	{
		if (block_get_flag(blk, BLOCK_USING) == 0 && blk->size >= size)
		{
			block_current = blk;
			return alloc_free_block(size);
		}
		blk = blk->next;
	} while (blk != block_current);
	return nullptr;
}

// 查找空闲块
void* alloc_free_block(size_t size)
{
	if (block_current->size == size) // 申请的大小正好是空闲块大小
	{
		return alloc_cur_block(size + 1);
	}
	// 申请的空间小于空闲块大小，将空闲块分裂
	auto new_size = block_current->size - size - 1;
	if (new_size == 0)
		return alloc_cur_block(size); // 分裂后的新块空间过低，放弃分裂
	block *new_blk = block_current + size + 1;
	block_init(new_blk, new_size);
	block_connect(block_current, new_blk);
	return alloc_cur_block(size);
}

// 直接使用当前的空闲块
void* alloc_cur_block(size_t size)
{
	// 直接使用空闲块
	block_set_flag(block_current, BLOCK_USING, 1); // 设置标志为可用
	block_current->size = size;
	block_available_size -= size + 1;
	auto cur = static_cast<void*>(block_current + 1);
	block_current = block_current->next; // 指向后一个块
	return cur;
}
```

#### 释放内存

其实这里并不是真正意义上的“释放”，只是清除了标记而已。

释放前必须检查地址合法性；释放后要进行空闲块合并的操作。

```c++
// 释放内存
bool _free(void* p)
{
	block *blk = static_cast<block*>(p);
	--blk; // 自减得到块的元信息头
	if (!verify_address(blk))
		return false;
	if (blk->next == blk) // 只有一个块
	{
		block_set_flag(blk, BLOCK_USING, 0);
		return true;
	}
	if (blk->prev == blk->next && block_get_flag(blk->prev, BLOCK_USING) == 0) // 只有两个块
	{
		_init(); // 两个块都空闲，直接初始化
		return true;
	}
	auto is_prev_free = block_get_flag(blk->prev, BLOCK_USING) == 0 && blk->prev < blk;
	auto is_next_free = block_get_flag(blk->next, BLOCK_USING) == 0 && blk < blk->next;
	auto bit = (is_prev_free << 1) + is_next_free;
	switch (bit)
	{
	case 0:
		block_available_size += blk->size + 1;
		block_set_flag(blk, BLOCK_USING, 0);
		break;
	case 1:
		block_available_size += block_merge(blk, blk->next);
		break;
	case 2:
		block_available_size += block_merge(blk->prev, blk);
		break;
	case 3:
		block_available_size += block_merge(blk->prev, blk, blk->next);
		break;
	default:
		break;
	}
	return true;
}

// 验证地址是否合法
bool verify_address(block *blk)
{
	if (blk < block_head || blk > block_head + DEFAULT_ALLOC_MEMORY_SIZE - 1)
		return false;
	return (blk->next->prev == blk) && (blk->prev->next == blk) && (block_get_flag(blk, BLOCK_USING) == 1);
}
```

### 五、封装

到这里要大功告成了，封装一下！

```c++
// 基于原始内存池的内存分配策略
template<class Allocator = default_allocator<>, size_t DefaultSize = Allocator::DEFAULT_ALLOC_BLOCK_SIZE>
class legacy_memory_pool_allocator
{
	legacy_memory_pool<Allocator, DefaultSize> memory_pool;

public:
	static const size_t DEFAULT_ALLOC_BLOCK_SIZE = DefaultSize - 2;

	template<class T>
	T* __alloc()
	{
		return memory_pool.template alloc<T>();
	}

	template<class T>
	T* __alloc_array(uint count)
	{
		return memory_pool.template alloc_array<T>(count);
	}

	template<class T, class ... TArgs>
	T* __alloc_args(TArgs ... args)
	{
		return memory_pool.template alloc_args<T>(args...);
	}

	template<class T, class ... TArgs>
	T* __alloc_array_args(uint count, TArgs ... args)
	{
		return memory_pool.template alloc_array_args<T>(count, args...);
	}

	template<class T>
	bool __free(T* t)
	{
		return memory_pool.free(t);
	}

	template<class T>
	bool __free_array(T* t)
	{
		return memory_pool.free_array(t);
	}
};

template<size_t DefaultSize = default_allocator<>::DEFAULT_ALLOC_BLOCK_SIZE>
using memory_pool = legacy_memory_pool<legacy_memory_pool_allocator<default_allocator<>, DefaultSize>>;
```

在内存池基础上构建新的内存池，颇有递归的味道。

### 六、单元测试

进入最后的环节——测试环节。

运行测试的过程真是战战兢兢，因为一开始全是未通过。不过测试的过程中也发现了代码中的疏漏。

## 源码

文章：https://github.com/bajdcc/learnstl/blob/master/chapter/01/Memory.md

源码：https://github.com/bajdcc/learnstl/blob/master/code/01/memory.h

## 总结

万里长征只走了第一步，继续努力！下一章可以实现变长数组`vector`了。