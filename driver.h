#ifndef DRIVER_HH
#define DRIVER_HH

#include "parser.h"
#include "options.h"
#ifdef GNUPLOT
#include "gnuplot.h"
#endif
#include "config.h"

#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <map>

// Tell Flex the lexer's prototype ...
# define YY_DECL \
	yy::parser::symbol_type yylex (driver& drv)
// ... and declare it for the parser's sake.
YY_DECL;

class driver
{
public:
	driver (options opt);

	// Run the parser on file f.  Return 0 on success.
	int parse_file(const std::string& f);
	int parse_string(const std::string& line);

	// The name of the file being parsed.
	std::string file;
	// Whether to generate parser debug traces.
	bool trace_parsing;

	// Handling the scanner.
	void scan_begin();
	void scan_file_begin();
	void scan_file_end();
	// Whether to generate scanner debug traces.
	bool trace_scanning;
	// The token's location used by the scanner.
	yy::location location;

	void result(std::shared_ptr<MathOps::MathOp<number>> op);
	std::shared_ptr<MathOps::MathOp<number>> solve(std::shared_ptr<MathOps::MathOp<number>> lhs,
    	std::shared_ptr<MathOps::MathOp<number>> rhs, std::string variable);
	std::vector<number> get_all_results(std::vector<std::shared_ptr<MathOps::MathOp<number>>> ops);
	void plot(std::string variable,
		std::vector<std::shared_ptr<MathOps::MathOp<number>>> equations,
		std::vector<number> args);
	void replot();
	std::shared_ptr<MathOps::MathOp<number>> find_identifier(std::string variable);
	std::shared_ptr<MathOps::MathOp<number>> assign(std::string variable, std::shared_ptr<MathOps::MathOp<number>> op);
	std::shared_ptr<MathOps::MathOp<number>> assign_lambda(std::string variable, std::shared_ptr<MathOps::MathOp<number>> op);
	void remove(std::string name);
	void make_var(std::string variable);

	void check_function(std::string func_name);
	std::shared_ptr<MathOps::MathOp<number>> function(std::string func_name,
		std::vector<std::shared_ptr<MathOps::MathOp<number>>> ops);
	std::shared_ptr<MathOps::MathOp<number>> get_constant(std::string id);
	void command(std::string cmd);

	bool input_is_file() const { return is_file; }

private:
	void show_variables();
	void clear_variables();
	void help();
	void warranty();

	void check_reserved(std::string variable);
	number print_result(std::shared_ptr<MathOps::MathOp<number>> op);
	std::shared_ptr<MathOps::Variable<number>> get_var(std::string variable);
	std::shared_ptr<MathOps::Container<number>> get_lambda(std::string variable);
	
#ifdef GNUPLOT
	void delete_plot_using(std::string name);

	GnuPlot<number> gp;
	std::string plot_variable;
	std::vector<std::shared_ptr<MathOps::MathOp<number>>> plot_equations;
	std::vector<number> plot_args;
#endif

	options opt;
	bool is_file;
	int var_id = 0;

	std::shared_ptr<MathOps::Variable<number>> digits;
	std::shared_ptr<MathOps::Variable<number>> ans;

#ifdef ARBIT_PREC
	std::shared_ptr<MathOps::Variable<number>> precision;
#endif
	std::vector<std::shared_ptr<MathOps::Variable<number>>> variables;
	std::vector<std::shared_ptr<MathOps::Container<number>>> lambdas;
};
#endif // ! DRIVER_HH
