# 第一章：内存管理

## 思路

1. 建立原始类型type
2. 构建原始内存池LegacyMemoryPool，无块大小限制，双向链表
3. 在LegacyMemoryPool基础上构建基于指针地址的哈希表LegacyHashset，用于验证地址的合法性
4. 借助LegacyHashset，向LegacyMemoryPool申请足够的内存作为内存池MemoryPool的初始内存，申请的内存分小块内存、中块内存和大块内存
5. 在MemoryPool的基础上实现哈希表Hashset，以用作对地址的合法性检查，实现块的开辟算法和合并算法，块是双向链表，以数组形式存储
6. 在MemoryPool的基础上实现可变长数组Vector

## 依赖图

尚未制作

## 注意点

- 最原始的类型具有Legacy前缀，含义为其输入永远是合法的，输入来自库而非用户
- LegacyMemoryPool不直接面向用户，因此它的使用频率较低；MemoryPool面向用户，使用效率较高
- 哈希表采用求模运算的方式进行Hash
- 整个过程中只申请一块内存，以后所有的默认内存管理操作都基于原始内存，换句话说，替换了全局的`new`和`delete`，方便日后对内存的使用统计及可视化