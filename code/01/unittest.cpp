#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Learncpp/memory.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{		
    TEST_CLASS(UnitTest_Memory)
    {
    public:
        
        TEST_METHOD(TestAllocAndFree1)
        {
            clib::memory::memory_pool<10> m;
            auto i = m.alloc<int>();
            *i = 100;
            Assert::IsTrue(m.free<int>(i), L"Success free");
            Assert::IsFalse(m.free<int>(i), L"Dup free");
            Assert::IsFalse(m.free<int>(nullptr), L"Free null");
            Assert::IsNull(m.alloc_array<int>(0x100000), L"Alloc much");
            Assert::AreEqual(i, m.alloc<int>(), L"Alloc equal");
            Assert::AreEqual(100, *i, L"Alloc equal");
            auto j = m.alloc<int>();
            Assert::AreEqual(8, j - i, L"Alloc equal");
            auto k = m.alloc<int>();
            Assert::AreEqual(8, k - j, L"Alloc equal");
            Assert::IsNull(m.alloc<int>(), L"No enough memory");
        }

        TEST_METHOD(TestAllocAndFree2)
        {
            clib::memory::memory_pool<16> m;
            int* i[] =
            {
                m.alloc<int>(),
                m.alloc_array<int>(5),
                m.alloc<int>(),
                m.alloc_array<int>(5),
                m.alloc<int>(),
            };
            *(i[0]) = 1;
            *(i[1]) = 2;
            *(i[2]) = 3;
            *(i[3]) = 4;
            *(i[4]) = 5;
            Assert::IsNull(m.alloc_array<int>(8));
            Assert::AreEqual(true, m.free(i[1]));
            Assert::AreEqual(true, m.free(i[3]));
            Assert::AreEqual(true, m.free(i[2]));
            i[1] = m.alloc_array<int>(5);
            Assert::IsNotNull(i[1]);
            Assert::AreEqual(2, *(i[1]));
            i[2] = m.alloc_array<int>(5);
            Assert::IsNotNull(i[2]);
            Assert::AreEqual(3, *(i[2]));
            i[3] = m.alloc<int>();
            Assert::IsNotNull(i[3]);
        }
    };
}