#include <iostream>
#include <algorithm>

#include "driver.h"
#include "parser.h"
#include "findvariabetransformer.h"
#include "rearrangetransformer.h"

using namespace std;

driver::driver()
    : trace_parsing(false), trace_scanning(false)
{
}

int driver::parse_file(const std::string &f)
{
    expressions.clear();
    file = f;
    location.initialize(&file);
    scan_file_begin();
    yy::parser parser(*this);
    parser.set_debug_level(trace_parsing);
    int res = parser.parse();
    scan_file_end();
    return res;
}

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern YY_BUFFER_STATE yy_scan_string(const char *yy_str);

int driver::parse_string(const std::string &line)
{
    expressions.clear();
    yy_scan_string(line.c_str());
    scan_begin();
    std::string file("string input");
    location.initialize(&file);
    yy::parser parser(*this);
    parser.set_debug_level(trace_parsing);
    return parser.parse();
}

void driver::add_var(std::shared_ptr<MathOpVariable<number>> variable)
{
    variables.push_back(variable);
}

void driver::make_var(std::string variable)
{
    if (!get_var(variable))
    {
        variables.push_back(MathFactory::Variable<number>(variable));
    }
}

void driver::show_variables()
{
    for (auto variable: variables)
    {
        expressions.push_back(variable);
    }
}

void driver::help()
{
    std::cout << "Syntax:\n"
                 "  Assignments                 : <variable name> = <expression>\n"
                 "                                 Example: c = sqrt(a^2 + b^2)\n"
                 "  Solve for a variable        : solve <variable name>: <expression> == <expession>\n"
                 "                                 Example: solve a: a^2 + b^2 == c^2\n"
                 "  Show all assigned variables : show\n"
                 "  Help                        : help, ?\n"
                 "  Constants                   : %pi, %e\n"
                 "  Math functions              : pow(), log(), sqrt(), sin(), asin(), cos(), acos(), tan(), atan()\n"
                 "\n"
                 "Default variables:\n"
                 "  digits                      : The number of significant digits to display (default: 5)\n"
                 "  precision                   : The number of significant digits used internally (default: 50)\n"
                 "\n"
                 "Exit                          : Control-D\n"
                 "\n";
}

void driver::warranty()
{
    std::cout << "Algebla: An equation solving, arbitrary precision calculator\n"
                 "Copyright (C) 2022 Tom Wimmenhove\n"
                 "\n"
                 "This program is free software; you can redistribute it and/or\n"
                 "modify it under the terms of the GNU General Public License\n"
                 "as published by the Free Software Foundation; either version 2\n"
                 "of the License, or (at your option) any later version.\n"
                 "\n"
                 "This program is distributed in the hope that it will be useful,\n"
                 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                 "GNU General Public License for more details.\n"
                 "\n"
                 "You should have received a copy of the GNU General Public License\n"
                 "along with this program; if not, write to the Free Software\n"
                 "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"
                 "\n";
}

std::shared_ptr<MathOp<number>> driver::solve(std::shared_ptr<MathOp<number>> lhs,
                                              std::shared_ptr<MathOp<number>> rhs, std::string variable)
{
    auto solve_for = std::static_pointer_cast<MathOpVariableBase<number>>(
        lhs->transform(MathOpFindVariableTransformer<number>(variable)));

    if (solve_for)
    {
        auto solved = lhs->transform(MathOpRearrangeTransformer<number>(solve_for, rhs));

        solve_for->set(solved->result());

        return solved;
    }

    solve_for = std::static_pointer_cast<MathOpVariableBase<number>>(
        rhs->transform(MathOpFindVariableTransformer<number>(variable)));

    if (!solve_for)
    {
        throw yy::parser::syntax_error(location, "variable " + variable + " appears on neither left or right side");        
    }

    auto solved = rhs->transform(MathOpRearrangeTransformer<number>(solve_for, lhs));

    solve_for->set(solved->result());

    return solved;
}

std::shared_ptr<MathOp<number>> driver::assign(std::string variable, std::shared_ptr<MathOp<number>> op)
{
    auto result = op->result();

    auto v = get_var(variable);
    if (!v)
    {
        auto new_variable = MathFactory::Variable(variable, result);
        variables.push_back(new_variable);

        return new_variable;
    }

    v->set(result);

    return v;
}

std::shared_ptr<MathOpVariable<number>> driver::find_var(std::string variable)
{
    auto v = get_var(variable);
    if (!v)
    {
        throw yy::parser::syntax_error(location, "variable " + variable + " has not been declared");
    }

    return v;
}

std::shared_ptr<MathOpVariable<number>> driver::get_var(std::string variable)
{
    auto it = std::find_if(variables.begin(), variables.end(),
        [&variable](std::shared_ptr<MathOpVariable<number>> v) { return v->get_symbol() == variable; });

    return it == variables.end()
        ? nullptr
        : *it;
}

void driver::add_exp(std::shared_ptr<MathOp<number>> exp)
{
    expressions.push_back(exp);
}
