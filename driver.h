#ifndef DRIVER_HH
#define DRIVER_HH

#include "parser.h"

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
	driver ();

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

	void print_result(std::shared_ptr<MathOp<number>> op);
	std::shared_ptr<MathOp<number>> solve(std::shared_ptr<MathOp<number>> lhs,
    	std::shared_ptr<MathOp<number>> rhs, std::string variable);
	std::shared_ptr<MathOpVariable<number>> find_var(std::string variable);
	std::shared_ptr<MathOpVariable<number>> get_var(std::string variable);
	std::shared_ptr<MathOp<number>> assign(std::string variable, std::shared_ptr<MathOp<number>> op);
	void add_var(std::shared_ptr<MathOpVariable<number>> variable);
	void make_var(std::string variable);

	void show_variables();
	void clear_variables();
	void help();
	void warranty();

	bool input_is_file() const { return is_file; }

private:
	bool is_file;
	std::vector<std::shared_ptr<MathOpVariable<number>>> variables;
	int var_id = 0;

	std::shared_ptr<MathOpVariable<number>> precision;
	std::shared_ptr<MathOpVariable<number>> digits;
};
#endif // ! DRIVER_HH
