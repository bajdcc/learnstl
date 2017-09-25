#include "RefStringIterator.h"


namespace cc_ref_string_iterator
{
    RefStringIteratorDecorator::RefStringIteratorDecorator()
    {
    }
    
    shared_ptr<LookAheadOneIterator> RefStringIteratorDecorator::lookAhead()
    {
        return make_shared<LookAheadOneIterator>(shared_from_this());
    }

    RefString::RefString(string ref): ref(ref), start(0), end(ref.length())
    {
    }

    int RefString::getStart() const
    {
        return start;
    }

    void RefString::setStart(int start)
    {
        this->start = start;
    }

    int RefString::getEnd() const
    {
        return end;
    }

    void RefString::setEnd(int end)
    {
        this->end = end;
    }

    void RefString::normalize()
    {
        if (start > end)
        {
            int tmp = start;
            start = end - 1;
            end = tmp + 1;
        }
    }

    char RefString::charAt(int index) const
    {
        if (index < length())
        {
            return ref[start + index];
        }
        return 0;
    }

    int RefString::length() const
    {
        return end - start;
    }

    shared_ptr<IRefStringIterator> RefString::iterator()
    {
        return make_shared<RefStringIterator>(shared_from_this());
    }

    string RefString::toString()
    {
        return ref.substr(start, end - start + 1);
    }

    RefStringIterator::RefStringIterator(shared_ptr<RefString> ref): ref(ref), length(ref->length())
    {
    }

    int RefStringIterator::index()
    {
        return ptr;
    }

    char RefStringIterator::current()
    {
        return ref->charAt(ptr);
    }

    char RefStringIterator::ahead()
    {
        return 0;
    }

    bool RefStringIterator::available()
    {
        return ptr < length;
    }

    void RefStringIterator::next()
    {
        if (ptr < length)
        {
            ptr++;
        }
    }

    RefStringIteratorBase::RefStringIteratorBase(shared_ptr<IRefStringIterator> iterator): iter(iterator)
    {
    }
    
    LookAheadOneIterator::LookAheadOneIterator(shared_ptr<IRefStringIterator> iterator): RefStringIteratorBase(iterator)
    {
        idx = this->iter->index();
        chCurrent = this->iter->current();
        iter->next();
        chNext = this->iter->current();
    }

    int LookAheadOneIterator::index()
    {
        return idx;
    }

    char LookAheadOneIterator::current()
    {
        return chCurrent;
    }

    char LookAheadOneIterator::ahead()
    {
        return chNext;
    }

    void LookAheadOneIterator::next()
    {
        idx = iter->index();
        chCurrent = chNext;
        iter->next();
        chNext = iter->current();
    }

    bool LookAheadOneIterator::available()
    {
        return chCurrent != 0 || chNext != 0;
    }
}
