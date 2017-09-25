#include <cassert>
#include <iostream>

#include "RefStringBase.h"
#include "CEval.h"

#define TEST_PARSER
#define TEST_DOUBLE
#define TEST_OTHER

using namespace cc_eval;

int main()
{
    try
    {
#ifdef TEST_PARSER
        {
            CEval eval;
            assert(eval.eval_output("1") == "1");
            assert(eval.eval_output("1.") == "1");
            assert(eval.eval_output("1 + 2") == "3");
            assert(eval.eval_output("1 -2") == "Error");
            assert(eval.eval_output("1 - 2") == "-1");
            assert(eval.eval_output("1 * -2") == "-2");
            assert(eval.eval_output("5 / 2") == "2");
            assert(eval.eval_output("5 / 0") == "Error");
            assert(eval.eval_output("1 +") == "Error");
            assert(eval.eval_output("e") == "Error");
            assert(eval.eval_output("()") == "Error");
            assert(eval.eval_output("\"") == "Error");
            assert(eval.eval_output("\"aa\"") == "aa");
            assert(eval.eval_output("\"aa\" + 123") == "aa123");
            assert(eval.eval_output("\"aa\" + 123.4") == "aa123.4");
            assert(eval.eval_output("1+ 123.4") == "124.4");
            assert(eval.eval_output("1+ 4* 7") == "29");
            assert(eval.eval_output("1+ 4* 7 -6*0") == "Error");
            assert(eval.eval_output("1+ 4* 7 - 6*0") == "29");
            assert(eval.eval_output("1+ 4* 7 - 6*") == "Error");
            assert(eval.eval_output("1+ 4* 7 * -2 / 2 - -3* -3 *-3") == "0");
            assert(eval.eval_output("25 - 6* 1 + 3") == "22");
            assert(eval.eval_output("(1 + 2)") == "3");
            assert(eval.eval_output("(1)") == "1");
            assert(eval.eval_output("(1 + )") == "Error");
            assert(eval.eval_output("2 * (1 + 2)") == "6");
            assert(eval.eval_output("(1 + 2) * 2") == "6");
            assert(eval.eval_output("(1 + 2) * 2 - (3 - 1)") == "4");
            assert(eval.eval_output("1+ 4* (7 * 1 - 1)") == "25");
            assert(eval.eval_output("1+ 4* (7 * 1 - 1) - 6* (0 + 1) /2 + 3") == "25"); 
            assert(eval.eval_output("(3 + 6)*(8 - -1)") == "81");
            //TODO: need more test!!!
        }
#endif
#ifdef TEST_DOUBLE
        { CEvalLexer lexer("123"); assert(lexer.next() == v_int); assert(lexer.getInt() == 123); }
        { CEvalLexer lexer("123."); assert(lexer.next() == v_double); assert(lexer.getDouble() == 123.0); }
        { CEvalLexer lexer("123.4"); assert(lexer.next() == v_double); assert(lexer.getDouble() == 123.4); }
        { CEvalLexer lexer("-123"); assert(lexer.next() == v_int); assert(lexer.getInt() == -123); }
        { CEvalLexer lexer("-123."); assert(lexer.next() == v_double); assert(lexer.getDouble() == -123.0); }
        { CEvalLexer lexer("-123.4"); assert(lexer.next() == v_double); assert(lexer.getDouble() == -123.4); }
        { CEvalLexer lexer("123e1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == 1230.0); }
        { CEvalLexer lexer("123.e1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == 1230.0); }
        { CEvalLexer lexer("123.4e1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == 1234.0); }
        { CEvalLexer lexer("-123e1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == -1230.0); }
        { CEvalLexer lexer("-123.e1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == -1230.0); }
        { CEvalLexer lexer("-123.4e1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == -1234.0); }
        { CEvalLexer lexer("123e-1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == 12.3); }
        { CEvalLexer lexer("123.e-1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == 12.3); }
        { CEvalLexer lexer("123.4e-1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == 12.34); }
        { CEvalLexer lexer("-123e-1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == -12.3); }
        { CEvalLexer lexer("-123.e-1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == -12.3); }
        { CEvalLexer lexer("-123.4e-1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == -12.34); }
        { CEvalLexer lexer("12345678987654321"); assert(lexer.next() == v_double); assert(lexer.getDouble() == 12345678987654321.0); }
        { CEvalLexer lexer("-"); assert(lexer.next() == v_oper); }
        { CEvalLexer lexer(".1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == .1); }
        { CEvalLexer lexer("-.1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == -.1); }
        { CEvalLexer lexer(".e1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == 0.0); }
        { CEvalLexer lexer("-.e-1"); assert(lexer.next() == v_double); assert(lexer.getDouble() == 0.0); }
        { CEvalLexer lexer("-123e"); assert(lexer.next() == v_error); }
        { CEvalLexer lexer("-123e-"); assert(lexer.next() == v_error); }
#endif
#ifdef TEST_OTHER
        { CEvalLexer lexer("+"); assert(lexer.next() == v_oper); assert(lexer.getOper() == op_add); }
        { CEvalLexer lexer("_a"); assert(lexer.next() == v_id); assert(lexer.getId() == "_a"); }
        { CEvalLexer lexer("\""); assert(lexer.next() == v_error); }
        { CEvalLexer lexer("\"\""); assert(lexer.next() == v_string); assert(lexer.getString() == ""); }
        { CEvalLexer lexer("\"aaa\""); assert(lexer.next() == v_string); assert(lexer.getString() == "aaa"); }
        { CEvalLexer lexer("\"\n\""); assert(lexer.next() == v_string); assert(lexer.getString() == "\n"); }
        { CEvalLexer lexer("\"\\n\""); assert(lexer.next() == v_string); assert(lexer.getString() == "\n"); }
        { CEvalLexer lexer("\"\\0\""); assert(lexer.next() == v_string); assert(lexer.getString() == ""); }
        { CEvalLexer lexer("\"\\x30\\x41\\x61\""); assert(lexer.next() == v_string); assert(lexer.getString() == "0Aa"); }
#endif
    }
    catch (cc_exception& e) {
        cerr << "Error: " << e.toString() << endl;
    }
    return 0;
}

