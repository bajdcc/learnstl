#pragma once

#include <string>
#include <map>
#include "RefStringBase.h"
#include "RefStringIterator.h"

using namespace std;
using namespace cc_ref_string_base;
using namespace cc_ref_string_iterator;

namespace cc_eval
{
    enum value_t
    {
        v_error,   // 出错
        v_id,      // 标识
        v_int,     // 整型
        v_double,  // 浮点
        v_string,  // 字符串
        v_space,   // 空白字符
        v_oper,    // 操作符
        v_end,     // 末尾
    };

    // 引用自CParser中的操作符类型
    enum operator_t
    {
        op_nil,
        op_equ,
        op_bin_begin, // 二元符号标记开始，从上往下优先级增高
        op_sub,
        op_add,
        op_mul,
        op_div,
        op_mod,
        op_bin_end,   // 二元符号标记结束
        op_esc,
        op_ask,
        op_and,
        op_bar,
        op_til,
        op_xor,
        op_exc,
        op_lpa,
        op_rpa,
        op_lsq,
        op_rsq,
        op_lbr,
        op_rbr,
        op_com,
        op_dot,
        op_sem,
        op_col,
    };

    /**
     * 词法分析
     */
    class CEvalLexer
    {
    private:
        shared_ptr<RefString> ref;
        shared_ptr<IRefStringIterator> iter;
    public:
        explicit CEvalLexer(const string& s);

        value_t current() const;
        value_t next();

        double getDouble() const;
        int getInt() const;
        string getString() const;
        string getId() const;
        operator_t getOper() const;

        string getValueString() const;

    private:
        value_t next_digit();
        value_t next_alpha();
        value_t next_string();
        value_t next_operator();

    private:
        value_t type{ v_error };
        double vDouble{ 0.0 };
        int vInt{ 0 };
        string vString;
        string vId;
        operator_t vOper;
        int nStart{ 0 }, nEnd{ 0 };
    };

    // -------------------------
    
    // AST结点

    class Node : public Object
    {
    public:
        virtual value_t type() const = 0;
        weak_ptr<Node> parent;
    };

    class IntegerNode : public Node
    {
    private:
        int n{ 0 };
    public:
        using value_type = int;
        explicit IntegerNode(int n);
        int value() const;
        value_t type() const override;
        string toString() override;
    };

    class DoubleNode : public Node
    {
    private:
        double d{ 0 };
    public:
        using value_type = double;
        explicit DoubleNode(double d);
        double value() const;
        value_t type() const override;
        string toString() override;
    };

    class StringNode : public Node
    {
    private:
        string s;
    public:
        using value_type = string;
        explicit StringNode(string s);
        string value() const;
        value_t type() const override;
        string toString() override;
    };

    class IdNode : public Node
    {
    private:
        string id;
    public:
        explicit IdNode(string id);
        string value() const;
        value_t type() const override;
        string toString() override;
    };

    class OperNode : public Node, public enable_shared_from_this<OperNode>
    {
    private:
        operator_t oper;
    public:
        explicit OperNode(operator_t oper);
        operator_t value() const;
        value_t type() const override;
        string toString() override;
        shared_ptr<Node> left, right;
        void addLeft(shared_ptr<Node> child);
        void addRight(shared_ptr<Node> child);
        shared_ptr<OperNode> assertLeftNull(string msg);
        shared_ptr<OperNode> assertRightNull(string msg);
    };

    /**
     * 计算器类，接收输入，返回结果
     */
    class CEval
    {
    private:
        shared_ptr<CEvalLexer> lexer;
        map<string, shared_ptr<Node>> env;

    public:
        CEval();
        ~CEval();

        string eval(const string &s);
        string eval_output(const string &s);
    private:
        shared_ptr<Node> parse();
        shared_ptr<Node> eval_node(shared_ptr<Node> node);
        string eval_string(shared_ptr<Node> node);
        shared_ptr<Node> eval_binop(operator_t op, value_t value, shared_ptr<Node> left, shared_ptr<Node> right);
        shared_ptr<OperNode> op_node(shared_ptr<Node> node);
        shared_ptr<Node> promote_node(shared_ptr<Node> node, value_t value);
        shared_ptr<Node> simplify_node(shared_ptr<Node> node);
        value_t nextValue();
    private:
        static int calc_binop_prior(operator_t op);
    };
}
