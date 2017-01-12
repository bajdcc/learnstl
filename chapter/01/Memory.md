# 第一章：内存管理

## 思路

1. 建立原始类型`type`
2. 构建原始内存池`legacy_memory_pool<default_allocator>`，无块大小限制，双向链表
3. 完善`legacy_memory_pool`，实现`allocator`接口，实现`legacy_memory_pool_allocator`
4. 构建类型`using memory_pool = legacy_memory_pool<legacy_memory_pool_allocator>`
5. 单元测试

## 依赖图

尚未制作

## 注意点

- 最原始的类型具有**legacy**前缀，含义为其输入永远是合法的，输入来自库而非用户
- 整个过程中只申请一块内存，以后所有的默认内存管理操作都基于原始内存，换句话说，替换了全局的`new`和`delete`，方便日后对内存的使用统计及可视化
- 完善单元测试