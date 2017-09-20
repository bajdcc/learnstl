#pragma once

#include <string>

using namespace std;

/**
 * 计算器类，接收输入，返回结果
 */
class CEval
{
public:
    CEval();
    ~CEval();

    enum value_t
    {
        v_error, // 表达式出错
        v_double, // 浮点
        v_int, // 整型
        v_string, // 字符串
    };

    value_t eval(const string &s);

private:
    double vDouble{ 0.0 };
    int vInt{ 0 };
    string vString;
};

