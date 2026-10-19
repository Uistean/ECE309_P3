// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"
#include  <stack>

static bool do_the_operation(std::stack<double>& values,std::stack<CalcToken::Kind>& ops,std::string* error) {

    if (ops.empty()||values.size()<2) {
        *error = "empty values or ops thus bad";

        return false;
    }


    CalcToken::Kind op = ops.top();
    ops.pop();
    double right = values.top();
    values.pop();
    double left = values.top();
    values.pop();
    double result = 0;

    if (op == CalcToken::Kind::Plus ) {
        result = left + right;
    }
    if (op == CalcToken::Kind::Minus ) {
        result = left - right;
    }
    if (op == CalcToken::Kind::Times ) {
        result = left * right;
    }
    if (op == CalcToken::Kind::Divide ) {
        if (right == 0) {
            *error = "division by zero";
            return  false;
        }

            result = left / right;

    }
    values.push(result);
    return  true;
}

static int precedence(CalcToken::Kind op) {
    if ((op == CalcToken::Kind::Plus)||(op == CalcToken::Kind::Minus) ) {
        return 1;
    }
     if((op == CalcToken::Kind::Times)||(op == CalcToken::Kind::Divide) ){ return 2;}
    return 0;
}

bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {

    std::stack<double> inputs;
    std::stack<CalcToken::Kind> ops;


    for (const CalcToken& tok : tokens) {

        if (tok.kind == CalcToken::Kind::Number ) {
            inputs.push(tok.value);
        }
        else if (tok.kind == CalcToken::Kind::LParen) {
            ops.push(tok.kind);
        }
        else if (tok.kind == CalcToken::Kind::RParen) {
            while ( !ops.empty()&&(ops.top() != CalcToken::Kind::LParen)) {
                if (!do_the_operation(inputs, ops, error)) {
                    return false;
                }
            }
            if (!ops.empty())ops.pop()
;        }
        else {
            while (!ops.empty() && precedence(ops.top()) >= precedence(tok.kind)) {
                if (!do_the_operation(inputs,ops,error)){return false;}
            }
            ops.push(tok.kind);
        }



    }
    while (!ops.empty()) {
        if (!do_the_operation(inputs, ops, error)) {
            return false;
        }
    }
    if (inputs.empty()) {
        *error = "no inputs thus bad";
        return false;
    }

        *out = inputs.top();
        return true;


}
