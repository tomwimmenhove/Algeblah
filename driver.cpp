#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <functional>
#include <limits>

#include "driver.h"
#include "parser.h"
#include "mathop/containercounter.h"
#include "mathop/rearrangemultitransformer.h"
#include "mathop/expandtransformer.h"
#include "mathop/namedvaluecounter.h"
#include "mathop/defaultformatter.h"
#include "mathop/finder.h"
#include "mathop/texformatter.h"
#include "mathop/constants.h"
#include "usefulfraction.h"

driver::driver(options opt)
    : trace_parsing(false), trace_scanning(false),
      opt(opt),
      digits(MathOps::Variable<number>::create("digits", opt.digits)),
      ans(MathOps::Variable<number>::create("ans", 0)),
#ifdef ARBIT_PREC
      precision(MathOps::Variable<number>::create("precision", opt.precision)),
      variables({precision, digits, ans})
#else
      variables({digits, ans})
#endif
{
#ifdef ARBIT_PREC
    boost::multiprecision::mpfr_float::default_precision((int) precision->result());
#endif
}

int driver::parse_file(const std::string &f)
{
    is_file = true;
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
    is_file = false;
    yy_scan_string(line.c_str());
    scan_begin();
    std::string file({});
    location.initialize(&file);
    yy::parser parser(*this);
    parser.set_debug_level(trace_parsing);
    return parser.parse();
}

void driver::make_var(const std::string& variable)
{
    if (get_lambda(variable))
    {
        throw yy::parser::syntax_error(location, variable + " is a lambda");
    }

    if (!get_var(variable))
    {
        variables.push_back(MathOps::Variable<number>::create(variable));
    }
}

void driver::show_variables()
{
    for (auto variable: variables)
    {
        print_result(variable);
    }

    for (auto lambda: lambdas)
    {
        print_result(lambda);
    }
}

void driver::clear_variables()
{
    variables.clear();
    lambdas.clear();
    plot_equations.clear();
#ifdef ARBIT_PREC
    variables.insert(variables.end(), { precision, digits, ans });
#else
    variables.insert(variables.end(), { digits, ans });
#endif
}

void driver::help()
{
    std::cout << "Syntax:\n"
                 "  Assignments                  : <variable name> = <expression>\n"
                 "                                  Example: c = sqrt(a^2 + b^2)\n"
                 "  Lambda assignments           : <lambda name> => <expression>\n"
                 "                                  Example: c => a + b\n"
                 "  Expanding a lambda:          : expand(<lambda name>)\n"
                 "                                  Example: c => expand(c)\n"
                 "  Solve for a variable         : solve <variable name>: <expression> = <expession>\n"
                 "                                  Example: solve a: a^2 + b^2 = c^2\n"
                 "  Convert expression to value  : value(<expression>)\n"
                 "                                  Example: some_lambda => 2 * value(anoter_lambda)\n"
#ifdef GNUPLOT
                 "  Plot                         : plot <variable name> [, <from>, <to>, <step>]: <expression>, <expression>, ...\n"
                 "                                  Example: plot x, 0, 2 * %pi: sin(x), cos(x)\n"
                 "  Close plot (kill gnuplot)    : unplot\n"
#endif
                 "  Delete a vairable or lambda  : <variable name> =\n"
                 "                                 : a =\n"
                 "  Show all assigned variables  : :show\n"
                 "  Clear all assigned variables : :clear\n"
                 "  Help                         : :help\n"
                 "  Constants                    : %pi, %e\n"
                 "  Math functions               : pow(), log(), log10(), sqrt(),\n"
                 "                               : sin(), asin(), cos(), acos(), tan(), atan()\n"
                 "                               : sinh(), asinh(), cosh(), acosh(), tanh(), atanh()\n"
                 "\n"
                 "Default variables:\n"
                 "  ans                          : The result of the last calculation\n"
                 "  digits                       : The number of significant digits to display (default: 5)\n"
#ifdef ARBIT_PREC
                 "  precision                    : The number of internal significant digits (default: 50)\n"
#endif
                 "\n";
    if (isatty(fileno(stdin)))
    {
        std::cout << "Exit                           : Control-D, :exit, :quit, :q\n\n";
    }
}

void driver::warranty()
{
#ifdef ARBIT_PREC
    std::cout << "Algebla: An equation solving, arbitrary precision calculator\n"
#else
    std::cout << "Algebla: An equation solving calculator\n"
#endif
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

std::vector<std::shared_ptr<MathOps::MathOp<number>>> driver::find_solutions(
    std::shared_ptr<MathOps::MathOp<number>> lhs,
    std::shared_ptr<MathOps::MathOp<number>> rhs,
    const std::shared_ptr<MathOps::Value<number>> &solve_for,
    bool solve_from_left)
{
    // XXX: Should we always expand the entire tree before solving?
    auto solve_side  = (solve_from_left ? lhs : rhs)->transform(MathOps::ExpandTransformer<number>());
    auto result_side = (solve_from_left ? rhs : lhs)->transform(MathOps::ExpandTransformer<number>());
    auto solutions = solve_side->multi_transform(MathOps::RearrangeMultiTransformer<number>(solve_for, result_side));

    /* Remove non-viable solutions */
    solutions.erase(std::remove_if(solutions.begin(), solutions.end(), [](auto& x) { 
        return isnan(x->result());
    }), solutions.end());

    return solutions;
}

std::shared_ptr<MathOps::MathOp<number>> driver::solve(std::shared_ptr<MathOps::MathOp<number>> lhs,
                                              std::shared_ptr<MathOps::MathOp<number>> rhs,
                                              const std::string& variable, number index)
{
    MathOps::NamedValueCounter<number> counter(variable);
    int left_count = lhs->count(counter);
    rhs->count(counter);
    auto& variables = counter.get_results();

    if ((int) index != index)
    {
        throw yy::parser::syntax_error(location, "Index should be an integer value");        
    }

    if (variables.size() == 0)
    {
        throw yy::parser::syntax_error(location, "variable " + variable + " appears on neither left or right side");        
    }

    if (variables.size() > 1)
    {
        throw yy::parser::syntax_error(location, "variable " + variable + " appears more than once");        
    }

    auto solutions = find_solutions(lhs, rhs, variables[0], left_count > 0);
    if (solutions.size() == 0)
    {
        throw yy::parser::syntax_error(location, "No solutions found");
    }

    int idx = index != -1 ? (int) index : 0;
    if (index != -1)
    {
        if (idx >= (int) solutions.size())
        {
            throw yy::parser::syntax_error(location, "Solution " + std::to_string(idx) + " does not exist");
        }
    }
    else if (solutions.size() > 1)
    {
        MathOps::DefaultFormatter<number> formatter((int) digits->result());
        std::cout << "WARNING: Multiple solutions for " << variable << ": "
                  << lhs->format(formatter) << " = "
                  << rhs->format(formatter) << ":\n";
        for (size_t i = 0; i < solutions.size(); i++)
        {
            std::cout << "         " << i << ": " << variable << " = " << solutions[i]->format(formatter) << '\n';
        }
        std::cout << "         Selecting solution 0. (Use \"solve " << variable << ", <index>: ...\" to override)\n";
    }

    variables[0]->set(solutions[idx]->result());

    return solutions[idx];
}

void driver::plot(const std::string& variable,
		std::vector<std::shared_ptr<MathOps::MathOp<number>>> equations,
		std::vector<std::shared_ptr<MathOps::MathOp<number>>> args)
{
#ifdef GNUPLOT
    if (equations.size() < 1)
    {
        throw yy::parser::syntax_error(location, "No expressions to plot");
    }

    if (args.size() > 3)
    {
        throw yy::parser::syntax_error(location, "Too may arguments for plot command");        
    }

    if (std::count(equations.begin(), equations.end(), nullptr))
    {
        throw yy::parser::syntax_error(location, "One of more empty expressions found");
    }
    
    number from = args.size() >= 1 && args[0] ? args[0]->result() : 0;
    number to =   args.size() >= 2 && args[1] ? args[1]->result() : from + 10;
    number step = args.size() >= 3 && args[2] ? args[2]->result() : (to - from) / 100;

    if (to <= from)
    {
        std::stringstream ss;
        ss << "Range of " << from << " to "  << to <<  " is invalid";

        throw yy::parser::syntax_error(location, ss.str());
    }

    if (!gp.is_open())
    {
        gp.open();
    }

    if (!gp.is_open())
    {
        throw yy::parser::syntax_error(location, "Failed to launch gnuplot");
    }

    auto v = get_var(variable);

    gp.plot(equations, v, from, to, step, (int) digits->result());
    plot_variable = variable;
    plot_equations = equations;
    plot_args = args;
#else
    throw yy::parser::syntax_error(location, "Not compiled with support for plotting");
#endif
}

void driver::replot()
{
#ifdef GNUPLOT
    if (plot_equations.size() == 0)
    {
        throw yy::parser::syntax_error(location, "Nothing to plot");
    }

    make_var(plot_variable);

    plot(plot_variable, plot_equations, plot_args);
#else
    throw yy::parser::syntax_error(location, "Not compiled with support for plotting");
#endif
}

void driver::unplot()
{
#ifdef GNUPLOT
    if (!gp.is_open())
    {
        throw yy::parser::syntax_error(location, "Gnuplot not running");
    }

    gp.close();
#else
    throw yy::parser::syntax_error(location, "Not compiled with support for plotting");
#endif
}

std::shared_ptr<MathOps::MathOp<number>> driver::assign(const std::string& variable, std::shared_ptr<MathOps::MathOp<number>> op)
{
    auto result = op->result();

    /* Special variables */
    if (variable == digits->get_name())
    {
#ifdef ARBIT_PREC
        if ((int) result > precision->result())
        {
            std::cerr << "Value can not be greater than precision.\n";

            return digits;
        }
#endif

        digits->set((int) result);

        return digits;
    }
#ifdef ARBIT_PREC
    else if (variable == precision->get_name())
    {
        if (opt.max_precision > 0 && (int) result > opt.max_precision)
        {
            std::cerr << "Value exceeds maximum precision.\n";

            return precision;
        }

        if ((int) result < digits->result())
        {
            std::cerr << "Value can not be less than the number of visible digits.\n";

            return precision;
        }

        boost::multiprecision::mpfr_float::default_precision((int) result);
        precision->set((int) result);

        return precision;
    }
#endif

    auto l = get_lambda(variable);
    if (l)
    {
        for (auto lambda : lambdas)
        {
            if (lambda != l && lambda->count(MathOps::Finder<number>(l)))
            {
                throw yy::parser::syntax_error(location, variable + " is in use by lambda " + lambda->get_name() + " as a lambda\n");
            }
        }

        remove(lambdas, l);
    }

    auto v = get_var(variable);
    if (!v)
    {
        v = MathOps::Variable<number>::create(variable, result);
        variables.push_back(v);

        return v;
    }

    v->set(result);

    return v;
}

std::shared_ptr<MathOps::MathOp<number>> driver::assign_lambda(const std::string& variable, std::shared_ptr<MathOps::MathOp<number>> op)
{
    check_reserved(variable);

    if (MathOps::NamedValueCounter<number>::find_first(op, variable))
    {
        throw yy::parser::syntax_error(location, "Lambda may not reference a variable with the same name");
    }

    for(auto lambda: lambdas)
    {
        if (MathOps::NamedValueCounter<number>::find_first(lambda, variable))
        {
            throw yy::parser::syntax_error(location, variable + " is in use by lambda " + lambda->get_name() + " as a variale\n");
        }
    }

    auto l = get_lambda(variable);
    if (l && op->count(MathOps::Finder<number>(l)))
    {
        throw yy::parser::syntax_error(location, "Infinite recursion detected");
    }

    auto v = get_var(variable);
    if (v)
    {
        remove(variables, v);
    }
    
    if (!l)
    {
        l = MathOps::Container<number>::create(op, variable);
        lambdas.push_back(l);

        return l;
    }

    l->set_inner(op);

    return l;
}

void driver::unassign(const std::string& name)
{
    check_reserved(name);

    auto op = find_identifier(name);

    /* Check if variable is in use */
    for(auto lambda: lambdas)
    {
        if (lambda->get_inner()->count(MathOps::Finder<number>(op)))
        {
            throw yy::parser::syntax_error(location, name + " is in use by lambda " + lambda->get_name() + "\n");
        }
    }

    variables.erase(std::remove(variables.begin(), variables.end(), op), variables.end());
    lambdas.erase(std::remove(lambdas.begin(), lambdas.end(), op), lambdas.end());

#ifdef GNUPLOT
    delete_plot_using(op);
#endif
}

#ifdef GNUPLOT
void driver::delete_plot_using(std::shared_ptr<MathOps::MathOp<number>> op)
{
    auto plot_it = plot_equations.begin();
    while (plot_it != plot_equations.end())
    {
        if ((*plot_it)->count(MathOps::Finder<number>(op)))
        {
            plot_it = plot_equations.erase(plot_it);
        }
        else
        {
            ++plot_it;
        }
    }

    for (auto& arg: plot_args)
    {
        if (arg->count(MathOps::Finder<number>(op)))
        {
            plot_args.clear();
            plot_equations.clear();
            return;
        }
    }
}
#endif

std::shared_ptr<MathOps::MathOp<number>> driver::find_identifier(const std::string& variable)
{
    std::shared_ptr<MathOps::MathOp<number>> v = get_var(variable);
    if (!v)
    {
        v = get_lambda(variable);
    }

    if (!v)
    {
        throw yy::parser::syntax_error(location, variable + " has not been declared");
    }

    return v;
}

std::shared_ptr<MathOps::Container<number>> driver::get_lambda(const std::string& name)
{
    return get(lambdas, name);
}

std::shared_ptr<MathOps::Variable<number>> driver::get_var(const std::string& name)
{
    return get(variables, name);
}

void driver::check_reserved(const std::string& variable)
{
    if (variable == ans->get_name()
#ifdef ARBIT_PREC
        || variable == precision->get_name()
#endif
        || variable == digits->get_name())
    {
        throw yy::parser::syntax_error(location, variable + " is reserved");
    }
}

struct FunctionOptions
{
    size_t num_args;
    std::function<std::shared_ptr<MathOps::MathOp<number>>(std::vector<std::shared_ptr<MathOps::MathOp<number>>>)> handler;
};

static std::map<std::string, FunctionOptions> function_map = {
    { "sqrt",   FunctionOptions { 1, [](auto ops) { return MathOps::sqrt(ops[0]);  } } },
    { "log",    FunctionOptions { 1, [](auto ops) { return MathOps::log(ops[0]);   } } },
    { "log10",  FunctionOptions { 1, [](auto ops) { return MathOps::log10(ops[0]); } } },
    { "sin",    FunctionOptions { 1, [](auto ops) { return MathOps::sin(ops[0]);   } } },
    { "cos",    FunctionOptions { 1, [](auto ops) { return MathOps::cos(ops[0]);   } } },
    { "tan",    FunctionOptions { 1, [](auto ops) { return MathOps::tan(ops[0]);   } } },
    { "asin",   FunctionOptions { 1, [](auto ops) { return MathOps::asin(ops[0]);  } } },
    { "acos",   FunctionOptions { 1, [](auto ops) { return MathOps::acos(ops[0]);  } } },
    { "atan",   FunctionOptions { 1, [](auto ops) { return MathOps::atan(ops[0]);  } } },
    { "sinh",   FunctionOptions { 1, [](auto ops) { return MathOps::sinh(ops[0]);  } } },
    { "cosh",   FunctionOptions { 1, [](auto ops) { return MathOps::cosh(ops[0]);  } } },
    { "tanh",   FunctionOptions { 1, [](auto ops) { return MathOps::tanh(ops[0]);  } } },
    { "asinh",  FunctionOptions { 1, [](auto ops) { return MathOps::asinh(ops[0]); } } },
    { "acosh",  FunctionOptions { 1, [](auto ops) { return MathOps::acosh(ops[0]); } } },
    { "atanh",  FunctionOptions { 1, [](auto ops) { return MathOps::atanh(ops[0]); } } },
    { "expand", FunctionOptions { 1, [](auto ops) { return ops[0]->transform(MathOps::ExpandTransformer<number>()); } } },
    { "value",  FunctionOptions { 1, [](auto ops) { return MathOps::ConstantValue<number>::create(ops[0]->result()); } } },
};

void driver::check_function(const std::string& func_name)
{
    if (function_map.find(func_name) == function_map.end())
    {
        throw yy::parser::syntax_error(location, "Uknown function: " + func_name);
    }
}

std::shared_ptr<MathOps::MathOp<number>> driver::function(const std::string& func_name,
    std::vector<std::shared_ptr<MathOps::MathOp<number>>> ops)
{
    auto it = function_map.find(func_name);
    if (it->second.num_args != ops.size())
    {
        throw yy::parser::syntax_error(location,
            func_name + " takes " + std::to_string(it->second.num_args) + " arguments, "
                                  + std::to_string(ops.size()) + " given");
    }

    return it->second.handler(ops);
}

std::shared_ptr<MathOps::MathOp<number>> driver::get_constant(const std::string& id)
{
    if (id == "e")  return MathOps::Constants::e<number>();
    if (id == "pi") return MathOps::Constants::pi<number>();
    else throw yy::parser::syntax_error(location, "Uknown constant: " + id);
}

void driver::command(const std::string& cmd)
{
    if      (cmd == "exit" ||
             cmd == "quit" ||
             cmd == "q")        exit(0);
    else if (cmd == "help")     help();
    else if (cmd == "warranty") warranty();
    else if (cmd == "show")     show_variables();
    else if (cmd == "clear")    clear_variables();
    else throw yy::parser::syntax_error(location, "Uknown command: " + cmd);
}

void driver::result(std::shared_ptr<MathOps::MathOp<number>> op)
{
    number result = print_result(op);
    
    ans->set(result);
}

std::string driver::format(std::shared_ptr<MathOps::MathOp<number>> op)
{
    int int_digits = (int) digits->result();

    if (opt.use_tex)
    {
        return op->format(MathOps::TexFormatter<number>(int_digits));
    }

    return op->format(MathOps::DefaultFormatter<number>(int_digits));    
}

std::string driver::result_string(std::shared_ptr<MathOps::MathOp<number>> op, number result)
{
    std::stringstream ss;

    int int_digits = (int) digits->result();
    ss << std::setprecision(int_digits);
    ss << "  ";

    if (opt.answer_only)
    {
        ss << result << '\n';
        return ss.str();
    }

    /* Expand containers? */
    auto container = MathOps::ContainerCounter<number>::find_first(op, "");
    if (container)
    {
        if (container == op)
        {
            /* Print the lambda name, followed by it's expression */
            ss << format(op) << " => " << format(container->get_inner()) << " = ";

            /* If the lambda contains more lambdas, print the lambda expression */
            if (MathOps::ContainerCounter<number>::find_first(container->get_inner(), ""))
            {
                ss << format(op->transform(MathOps::ExpandTransformer<number>())) << " = ";
            }
        }
        else
        {
            /* Print the lambda name, followed by it's expression */
            ss << format(op) << " = " << format(op->transform(MathOps::ExpandTransformer<number>())) << " = ";
        }
    }
    else
    {
        ss << format(op) << " = ";
    }

    std::string uf = useful_fraction<number>(result, int_digits);
    if (opt.use_tex || uf.empty())
    {
        ss << result;
    }
    else
    {
        ss << result << " (~" << uf << ")";
    }

    return ss.str();
}

number driver::print_result(std::shared_ptr<MathOps::MathOp<number>> op)
{
    number result = op->result();

    std::string s = result_string(op, result);

    if (opt.external.empty())
    {
        std::cout << s << '\n';

        return result;
    }
    else
    {
        int pid = fork();
        if (pid != 0)
        {
            int status;
            waitpid(pid, &status, 0);

            return result;
        }

        execlp(opt.external.c_str(), opt.external.c_str(), s.c_str(), nullptr);

        perror("exec");

        exit(100);
    }
}