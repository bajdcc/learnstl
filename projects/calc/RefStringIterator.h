#pragma once

#include "RefStringBase.h"
#include <memory>

using namespace std;
using namespace cc_ref_string_base;

namespace cc_ref_string_iterator
{
    class IRefStringIterator;
    class IRefStringFindIterator;
    class RefString;
    class RefStringIterator;
    class RefStringIteratorBase;
    class RefStringIteratorDecorator;

    class LookAheadOneIterator;

    /**
    * 引用字串的迭代器
    * <p><i>不向前看</i></p>
    * @author bajdcc
    */
    class IRefStringIterator : public Object
    {
    public:
        /**
        * @return 当前位置
        */
        virtual int index() = 0;

        /**
        * @return 当前字符
        */
        virtual char current() = 0;

        /**
        * @return 向前看一个字符
        */
        virtual char ahead() = 0;

        /**
        * @return 是否可以继续
        */
        virtual bool available() = 0;

        /**
        * 指针移动至下一字符
        */
        virtual void next() = 0;

        ///////////////////////////////////////////////

        /**
        * 提供向前看一个字符的功能
        * @return 迭代器
        */
        virtual shared_ptr<LookAheadOneIterator> lookAhead() = 0;
    };

    /**
    * 迭代器基类，实现装饰方法
    * @author bajdcc
    */
    class RefStringIteratorDecorator : public IRefStringIterator, public enable_shared_from_this<RefStringIteratorDecorator>
    {
    public:
        RefStringIteratorDecorator();

        shared_ptr<LookAheadOneIterator> lookAhead() override;
    };

    /**
    * 引用类型的字符串
    * @author bajdcc
    */
    class RefString : public Object, public enable_shared_from_this<RefString>
    {
    private:
        string ref;
        int start, end;

    public:
        RefString(string ref);

        int getStart() const;
        void setStart(int start);
        int getEnd() const;
        void setEnd(int end);
        void normalize();
        char charAt(int index) const;
        int length() const;

        shared_ptr<IRefStringIterator> iterator();
        shared_ptr<IRefStringIterator> reverse();
        string toString() override;
    };

    /**
    * 简易迭代器
    * @author bajdcc
    */
    class RefStringIterator : public RefStringIteratorDecorator
    {
    private:
        int ptr{0};
        shared_ptr<RefString> ref;
        int length;

    public:
        RefStringIterator(shared_ptr<RefString> ref);

        int index() override;
        char current() override;
        char ahead() override;
        bool available() override;
        void next() override;
    };

    /**
    * 迭代器基类，拥有外部装饰器
    * @author bajdcc
    */
    class RefStringIteratorBase : public RefStringIteratorDecorator
    {
    protected:
        shared_ptr<IRefStringIterator> iter;

    public:
        RefStringIteratorBase(shared_ptr<IRefStringIterator> iterator);
    };

    /**
    * 字符串的迭代器
    * <p>向前看一位</p>
    * @author bajdcc
    */
    class LookAheadOneIterator : public RefStringIteratorBase
    {
    private:
        char chCurrent;
        char chNext;
        int idx;

    public:
        LookAheadOneIterator(shared_ptr<IRefStringIterator> iterator);

        int index() override;
        char current() override;
        char ahead() override;
        void next() override;
        bool available() override;
    };
}
