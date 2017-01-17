#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Learncpp/vector.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
    TEST_CLASS(UnitTest_Vector)
    {
    public:

        TEST_METHOD(Vector1)
        {
            clib::collections::vector<int> vec;
            vec.push(100);
            Assert::AreEqual(100, vec.top());
            vec.push(200);
            Assert::AreEqual(200, vec.get(1));
            Assert::AreEqual(2U, vec.size());
            vec.pop();
            Assert::AreEqual(100, vec.top());
            Assert::AreEqual(1U, vec.size());
            vec.pop();
            Assert::AreEqual(0U, vec.size());
            for (auto i = 0; i < 0x20; i++)
            {
                vec.push(std::forward<int>(i));
            }
            for (auto i = 0; i < 0x20; i++)
            {
                Assert::AreEqual(i, vec.get(i));
            }
        }
    };
}