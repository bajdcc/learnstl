#include "CEval.h"
#include <iostream>
#include <sstream>
#include <vector>

// 实现，一开始都是这么简陋，不是吗？

namespace cc_eval
{
    CEvalLexer::CEvalLexer(const string& s)
    {
        ref = make_shared<RefString>(s);
        iter = ref->iterator()->lookAhead();
    }

    value_t CEvalLexer::current() const
    {
        return type;
    }

    value_t CEvalLexer::next()
    {
        // 词法分析参考：https://github.com/bajdcc/CParser/blob/master/CParser/Lexer.cpp
        auto c = iter->current();
        nStart = iter->index();
        if (!c) // 末尾
        {
            nEnd = iter->index();
            return type = v_end;
        }
        try
        {
            if (isalpha(c) || c == '_') // 变量名或关键字
            {
                type = next_alpha();
            }
            else if (isdigit(c) || c == '-' || c == '.') // 数字
            {
                type = next_digit();
            }
            else if (isspace(c)) // 空白字符
            {
                type = v_space;
            }
            else if (c == '\"') // 字符串
            {
                type = next_string();
            }
            else // 操作符
            {
                type = next_operator();
            }
        }
        catch (cc_exception& e)
        {
            cerr << "Lexer error: " << e.toString() << endl;
            type = v_error;
        }
        nEnd = iter->index();
        iter->next();
        return type;
    }

    double CEvalLexer::getDouble() const
    {
        return vDouble;
    }

    int CEvalLexer::getInt() const
    {
        return vInt;
    }

    string CEvalLexer::getString() const
    {
        return vString;
    }

    string CEvalLexer::getId() const
    {
        return vId;
    }

    operator_t CEvalLexer::getOper() const
    {
        return vOper;
    }

    string CEvalLexer::getValueString() const
    {
        return ref->toString().substr(nStart, nEnd - nStart + 1);
    }

    static double calc_exp(double d, int e)
    {
        if (e == 0)
            return d;
        else if (e > 0)
            for (int i = 0; i < e; i++)
                d *= 10;
        else
            for (int i = e; i < 0; i++)
                d /= 10;
        return d;
    }

    value_t CEvalLexer::next_digit()
    {
        // 手动实现atof/atoi，并类型转换
        // ** 测试已通过（在main.cpp）

        // 词法规则不表示了，见main.cpp的测试用例
        // 其他功能：int溢出转double，e科学记数法
        // PS：这可能是最复杂的一个parser了
        //     实现atod，至少需要向前看一字符

        type = v_int; // 假设是整型
        bool neg = iter->current() == '-';
        bool has_exp = false;
        bool neg_exp = false;
        bool has_dot = false;
        if (neg) iter->next();
        int c = iter->current();
        int n = 0, l = 0, e = 0;
        double d = 0.0;
        if (isdigit(c))
            n = iter->current() - '0';
        else if (c == '.')
        {
            type = v_double;
            has_dot = true;
        }
        else
        {
            vOper = op_sub;
            return v_oper;
        }
        for (;;)
        {
            c = iter->ahead();
            if (isdigit(c))
            {
                if (type == v_int)
                {
                    auto _n = n;
                    n *= 10;
                    n += c - '0';
                    if (n < 0)
                    {
                        n = 0;
                        type = v_double;
                        d = double(_n) * 10 + double(c - '0');
                    }
                }
                else if (has_exp)
                {
                    e *= 10;
                    e += c - '0';
                }
                else
                {
                    if (has_dot)
                        l++;
                    d *= 10.0;
                    d += c - '0';
                }
                iter->next();
            }
            else if (c == '.')
            {
                type = v_double;
                has_dot = true;
                iter->next();
            }
            else if (c == 'e' || c == 'E')
            {
                if (type == v_int)
                {
                    d = double(n);
                    type = v_double;
                }
                else
                    d = double(n) + calc_exp(d, -l);
                has_exp = true;
                iter->next();
                if (iter->ahead() == '-')
                {
                    neg_exp = true;
                    iter->next();
                    if (!isdigit(iter->ahead()))
                        throw cc_exception("invalid negative number of exp");
                }
                if (!isdigit(iter->ahead()))
                    throw cc_exception("invalid exp");
            }
            else
            {
                if (type == v_int)
                    vInt = neg ? -n : n;
                else
                {
                    if (has_exp)
                        vDouble = calc_exp(d, neg_exp ? -e : e);
                    else
                        vDouble = double(n) + calc_exp(d, -l);
                    if (neg) vDouble = -vDouble;
                }
                return type;
            }
        }
    }

    value_t CEvalLexer::next_alpha()
    {
        ostringstream oss;
        oss << iter->current();
        for (;;)
        {
            auto c = iter->ahead();
            if (!(isalnum(c) || c == '_'))
                break;
            oss << c;
            iter->next();
        }
        vId = oss.str();
        return v_id;
    }

    value_t CEvalLexer::next_string()
    {
        iter->next();
        ostringstream oss;
        for (;;)
        {
            auto c = iter->current();
            if (!c)
                throw cc_exception("incomplete string");
            if (c == '"')
            {
                vString = oss.str();
                break;
            }
            if (c == '\\')
            {
                iter->next();
                switch (iter->current())
                {
                case 'b':
                    oss << '\b';
                    break;
                case 'f':
                    oss << '\f';
                    break;
                case 'n':
                    oss << '\n';
                    break;
                case 'r':
                    oss << '\r';
                    break;
                case 't':
                    oss << '\t';
                    break;
                case 'v':
                    oss << '\v';
                    break;
                case '\'':
                    oss << '\'';
                    break;
                case '\"':
                    oss << '\"';
                    break;
                case '\\':
                    oss << '\\';
                    break;
                case '0': // skip
                    break;
                case 'x':
                case 'X':
                {
                    auto xx = iter->ahead(); // current = \\x, ahead = X1
                    if (!isxdigit(xx))
                        throw cc_exception("incomplete string escape \\xXX");
                    auto n1 = xx <= '9' ? xx - '0' : ((xx | 0x20) - 'a');
                    iter->next();
                    xx = iter->ahead(); // current = X1, ahead = X2
                    if (isxdigit(xx))
                    {
                        auto n2 = xx <= '9' ? xx - '0' : ((xx | 0x20) - 'a');
                        iter->next();
                        oss << char((n1 << 4) | n2);
                    }
                    else
                    {
                        oss << char(n1);
                    }
                }
                    break;
                default:
                    throw cc_exception("incomplete string escape");
                }
            }
            else
            {
                oss << c;
            }
            iter->next();
        }
        return v_string;
    }

    value_t CEvalLexer::next_operator()
    {
        static map<char, operator_t> mapOper =
        {
            { '=', op_equ },
            { '+', op_add },
            { '-', op_sub },
            { '*', op_mul },
            { '/', op_div },
            { '%', op_mod },
            { '\\', op_esc },
            { '?', op_ask },
            { '%', op_mod },
            { '&', op_and },
            { '|', op_bar },
            { '~', op_til },
            { '^', op_xor },
            { '!', op_exc },
            { '(', op_lpa },
            { ')', op_rpa },
            { '[', op_lsq },
            { ']', op_rsq },
            { '{', op_lbr },
            { '}', op_rbr },
            { ',', op_com },
            { '.', op_dot },
            { ';', op_sem },
            { ':', op_col },
        };
        auto op1 = mapOper.find(iter->current());
        if (op1 != mapOper.end())
        {
            vOper = op1->second;
            return v_oper;
        }
        return v_error;
    }

    IntegerNode::IntegerNode(int n): n(n)
    {
    }

    int IntegerNode::value() const
    {
        return n;
    }

    value_t IntegerNode::type() const
    {
        return v_int;
    }

    string IntegerNode::toString()
    {
        ostringstream oss;
        oss << n;
        return oss.str();
    }

    DoubleNode::DoubleNode(double d): d(d)
    {
    }

    double DoubleNode::value() const
    {
        return d;
    }

    value_t DoubleNode::type() const
    {
        return v_double;
    }

    string DoubleNode::toString()
    {
        ostringstream oss;
        oss << d;
        return oss.str();
    }

    StringNode::StringNode(string s): s(s)
    {
    }

    string StringNode::value() const
    {
        return s;
    }

    value_t StringNode::type() const
    {
        return v_string;
    }

    string StringNode::toString()
    {
        return s;
    }

    IdNode::IdNode(string id): id(id)
    {
    }

    string IdNode::value() const
    {
        return id;
    }

    value_t IdNode::type() const
    {
        return v_id;
    }

    string IdNode::toString()
    {
        return id;
    }

    OperNode::OperNode(operator_t oper): oper(oper)
    {
    }

    operator_t OperNode::value() const
    {
        return oper;
    }

    value_t OperNode::type() const
    {
        return v_oper;
    }

    string OperNode::toString()
    {
        ostringstream oss;
        oss << oper;
        return oss.str();
    }

    void OperNode::addLeft(shared_ptr<Node> child)
    {
        left = child;
        child->parent = shared_from_this();
    }

    void OperNode::addRight(shared_ptr<Node> child)
    {
        right = child;
        child->parent = shared_from_this();
    }

    shared_ptr<OperNode> OperNode::assertLeftNull(string msg)
    {
        if (left) throw cc_exception(msg);
        return shared_from_this();
    }

    shared_ptr<OperNode> OperNode::assertRightNull(string msg)
    {
        if (right) throw cc_exception(msg);
        return shared_from_this();
    }

    CEval::CEval()
    {
    }


    CEval::~CEval()
    {
    }

    string CEval::eval(const string& s)
    {
        try
        {
            lexer = make_shared<CEvalLexer>(s);
            return eval_string(parse());
        }
        catch (cc_exception& e)
        {
            cerr << "Parser error: " << e.toString() << endl;
        }
        return "Error";
    }

    string CEval::eval_output(const string& s)
    {
        cout << "Input: " << s << " ==> ";
        auto r = eval(s);
        if (r != "Error")
            cout << r << endl;
        return r;
    }

    shared_ptr<Node> CEval::parse()
    {
        // 准备实现括号

        // 实话说，只用循环做LL分析还是头一次
        // 没有用所谓的递归，好处就是不会爆栈
        // 一共只用了一个线索结点，哈哈
        vector<shared_ptr<OperNode>> rootStack; // 根结点栈
        vector<shared_ptr<Node>> nodeStack; // 当前结点栈

        auto root = make_shared<OperNode>(op_nil); // 根结点父亲
        shared_ptr<Node> node; // 当前结点
        auto value = v_error;
        bool fetch_next = true;
        for (;;) // 我就不用递归
        {
            if (fetch_next) // 当需要归约时，暂停输入
                value = nextValue();
            if (value == v_end) // 求值
            {
                return simplify_node(node); // 处理尚未化简的表达式（层数<=2）
            }
            if (value == v_int) // 叶子
            {
                if (!node) // 替换根
                    root->addLeft(node = make_shared<IntegerNode>(lexer->getInt()));
                else
                    op_node(node)->assertRightNull(string("required op but found int: ") + lexer->getValueString())
                    ->addRight(make_shared<IntegerNode>(lexer->getInt()));
            }
            else if (value == v_double) // 叶子
            {
                if (!node) // 替换根
                    root->addLeft(node = make_shared<DoubleNode>(lexer->getDouble()));
                else
                    op_node(node)->assertRightNull(string("required op but found double: ") + lexer->getValueString())
                    ->addRight(make_shared<DoubleNode>(lexer->getDouble()));
            }
            else if (value == v_string) // 叶子
            {
                if (!node) // 替换根
                    root->addLeft(node = make_shared<StringNode>(lexer->getString()));
                else
                    op_node(node)->assertRightNull(string("required op but found string: ") + lexer->getValueString())
                    ->addRight(make_shared<StringNode>(lexer->getString()));
            }
            else if (value == v_id) // 叶子
            {
                if (!node) // 替换根
                    root->addLeft(node = make_shared<IdNode>(lexer->getId()));
                else
                    op_node(node)->assertRightNull(string("required op but found id: ") + lexer->getValueString())
                    ->addRight(make_shared<IdNode>(lexer->getId()));
            }
            else if (value == v_oper) // 根
            {
                if (!root) // 替换根
                    root->addLeft(node = make_shared<OperNode>(lexer->getOper()));
                else
                {
                    auto op = lexer->getOper();
                    // 此时node是子树或是叶子
                    // 若是加减乘除等二元运算，将其移至操作符左子树
                    // 因为二元运算的前后必是两根左右子树
                    if (op > op_bin_begin && op < op_bin_end)
                    {
                        auto parent_op = op_node(node->parent.lock());
                        // 当前结点是操作符，意味着进行到3+4的结尾
                        // 但是要考虑优先级的情况
                        // 如果op的优先级大于当前node的优先级，
                        // 那么就要让node的右孩子拱手让给op
                        // 由op构建子树成为node的右孩子
                        if (node->type() == v_oper)
                        {
                            auto old_op = op_node(node);
                            if (calc_binop_prior(op) > calc_binop_prior(old_op->value())) //  如果当前优先级大于原先优先级
                            {
                                // 抢走原树的右孩子！
                                // 就要进行右孩子置换
                                // old_op->right = new_op(left: old_op->right)
                                auto new_op = make_shared<OperNode>(op);
                                new_op->addLeft(old_op->right);
                                old_op->addRight(new_op);
                                node = new_op;
                                // 置换好后，树变成右斜树，导致暂不可求值
                                if (!fetch_next) fetch_next = true;
                            }
                            else // 否则就可以先结合并求值了，哈哈
                            {
                                // 如果不马上简化树，这个树就变复杂了哈~
                                // 计算二元表达式的值
                                auto tmp_node = eval_node(old_op);
                                // 新建new_op的结点，其将置换原表达式
                                //auto new_op = make_shared<OperNode>(op);
                                // 构建op的新结点，左孩子为tmp_node，等待右孩子

                                // 已改进，启用归约阶段，此时暂停输入
                                if (parent_op->value() == op_nil) // 根了
                                {
                                    parent_op->addLeft(tmp_node);
                                    node = tmp_node;
                                }
                                else
                                {
                                    parent_op->addRight(tmp_node);
                                    node = parent_op;
                                }
                                // 这样的好处：不会生成很长的(斜)树
                                // 如果不简化，如3-4*5*6+7时，
                                // 处理"+"号就要从node向上回溯，直到遇到大于等于"-"优先级的结点
                                // 还要将4*5*6从树上摘下，挂到"+7"的下面，太繁琐
                                // 由此证明简化的思路还是可取的

                                // 归约时不输入，设置flag
                                fetch_next = false;
                            }
                        }
                        else
                        {
                            // 当前结点是叶子，那么一定是还没构建树
                            auto new_op = make_shared<OperNode>(op);
                            new_op->addLeft(node);
                            parent_op->addLeft(new_op);
                            node = new_op;
                            if (!fetch_next) fetch_next = true;
                        }
                    }
                    else if (op == op_lpa) // 左括号，入栈，保存现场
                    {
                        rootStack.push_back(root);
                        nodeStack.push_back(node);
                        root = make_shared<OperNode>(op_nil);
                        node.reset();
                    }
                    else if (op == op_rpa) // 右括号，出栈，恢复现场，计算表达式
                    {
                        if (rootStack.empty())
                            throw cc_exception(string("redundant op: ") + lexer->getValueString());
                        // 处理尚未化简的表达式（层数<=2）
                        auto exp = simplify_node(node); // 括号内的表达式一定会计算成值(叶子)
                        root = rootStack.back(); rootStack.pop_back(); // 本层root没用了
                        node = nodeStack.back(); nodeStack.pop_back();
                        if (!node)
                            root->addLeft(node = exp); // node是值而非操作符
                        else if (node->type() == v_oper)
                            op_node(node)->assertRightNull(string("required op but found evaluated value: ") + exp->toString())
                            ->addRight(exp);
                        else
                            throw cc_exception(string("required op but found evaluated value: ") + exp->toString());
                    }
                    else
                    {
                        throw cc_exception(string("undefined operator: ") + lexer->getValueString());
                    }
                }
            }
            else if (value == v_error)
            {
                throw cc_exception("lexer error");;
            }
        }
    }

    shared_ptr<Node> CEval::eval_node(shared_ptr<Node> node)
    {
        auto type = node->type();
        if (type == v_oper)
        {
            auto oper = op_node(node);
            auto op = oper->value();
            if (oper->left && oper->right) // 二元
            {
                auto left = eval_node(oper->left);
                auto right = eval_node(oper->right);
                auto max_value_type = __max(left->type(), right->type());
                left = promote_node(left, max_value_type);
                right = promote_node(right, max_value_type);
                return eval_binop(op, max_value_type, left, right);
            }
            else if (oper->left) // 一元
            {
                throw cc_exception("missing value");
            }
            else
            {
                throw cc_exception("missing value");
            }
        }
        if (type == v_id)
        {
            auto f = env.find(node->toString());
            if (f != env.end())
                return f->second;
            throw cc_exception(string("undefined symbol: ") + node->toString());
        }
        return node;
    }

    string CEval::eval_string(shared_ptr<Node> node)
    {
        if (node->type() == v_oper)
            throw cc_exception("cannot stringify op node");
        return node->toString();
    }

    // -----------------------------

    // 二元函数实现

    template<class T>
    struct eval_s
    {
        
    };

    template<>
    struct eval_s<int>
    {
        static int eval(operator_t op, const int& left, const int& right)
        {
            switch (op)
            {
            case op_sub: return left - right;
            case op_add: return left + right;
            case op_mul: return left * right;
            case op_div: return right == 0 ? throw cc_exception("divided by zero") : left / right;
            case op_mod: return left % right;
            default:
                throw cc_exception("not implemented for binop int");
            }
            return 0;
        }
    };

    template<>
    struct eval_s<double>
    {
        static double eval(operator_t op, const double& left, const double& right)
        {
            switch (op)
            {
            case op_sub: return left - right;
            case op_add: return left + right;
            case op_mul: return left * right;
            case op_div: return right == 0.0 ? throw cc_exception("divided by zero") : left / right;
            default:
                throw cc_exception("not implemented for binop double");
            }
            return 0.0;
        }
    };

    template<>
    struct eval_s<string>
    {
        static string eval(operator_t op, const string& left, const string& right)
        {
            switch (op)
            {
            case op_add: return left + right;
            default:
                throw cc_exception("not implemented for binop double");
            }
        }
    };

    template <class T> shared_ptr<Node> node_binop(operator_t op, shared_ptr<Node> __x, shared_ptr<Node> __y)
    {
        return make_shared<T>(eval_s<typename T::value_type>::eval(op, dynamic_pointer_cast<T>(__x)->value(), dynamic_pointer_cast<T>(__y)->value()));
    }

    // -----------------------------

    shared_ptr<Node> CEval::eval_binop(operator_t op, value_t value, shared_ptr<Node> left, shared_ptr<Node> right)
    {
        using bin_type = function<shared_ptr<Node>(operator_t, shared_ptr<Node>, shared_ptr<Node>)>;
        // map_binop = op_type -> value_type -> (Node -> Node -> Node)
        static map<value_t, bin_type> map_binop = {
            { v_int, node_binop<IntegerNode> },
            { v_double, node_binop<DoubleNode> },
            { v_string, node_binop<StringNode> }
        };
        auto f = map_binop.find(value);
        if (f != map_binop.end())
        {
            return (f->second)(op, left, right);
        }
        throw cc_exception("undefined value type for binop");
    }

    shared_ptr<OperNode> CEval::op_node(shared_ptr<Node> node)
    {
        if (node->type() == v_oper)
            return dynamic_pointer_cast<OperNode>(node);
        throw cc_exception("cannot cast op node: " + node->toString());
    }

    shared_ptr<Node> CEval::promote_node(shared_ptr<Node> node, value_t value)
    {
        if (value == node->type())
            return node;
        if (value == v_string) // to_string
            return make_shared<StringNode>(node->toString());
        if (value == v_double) // to_string
        {
            if (node->type() == v_int)
                return make_shared<DoubleNode>(double(dynamic_pointer_cast<IntegerNode>(node)->value()));
            throw cc_exception(string("cannot cast string to double: " + node->toString()));
        }
        throw cc_exception(string("cannot cast: " + node->toString()));
    }

    shared_ptr<Node> CEval::simplify_node(shared_ptr<Node> node)
    {
        if (!node)
            throw cc_exception("null expression");
        if (node->type() == v_oper)
        {
            auto op = node;
            for (;;) // 由于简化机制，一般循环只执行2次
            { // 最多就是3+5*5 [END]，再多输入的话，表达式依然会自动化简
                auto op2 = eval_node(op_node(op));
                auto parent = op_node(op->parent.lock());
                if (parent->value() == op_nil)
                    return op2;
                parent->addRight(op2);
                op = parent;
            }
        }
        return eval_node(node);
    }

    value_t CEval::nextValue()
    {
        lexer->next();
        while (lexer->current() == v_space)
            lexer->next();
        return lexer->current();
    }

    int CEval::calc_binop_prior(operator_t op)
    {
        vector<int> prior = { 1, 1, 2, 2, 2 };
        if (op > op_bin_begin && op < op_bin_end)
        {
            return prior.at(op - op_bin_begin - 1);
        }
        throw cc_exception("undefined binop priority");
    }
}
