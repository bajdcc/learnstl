#include "RefStringBase.h"


namespace cc_ref_string_base
{
    Object::Object()
    {
    }

    Object::~Object()
    {
    }

    string Object::toString()
    {
        return "None";
    }

    cc_exception::cc_exception(string reason): reason(reason)
    {
    }

    cc_exception::cc_exception(const cc_exception& e) : reason(e.reason)
    {
    }

    string cc_exception::toString()
    {
        return reason;
    }
}
