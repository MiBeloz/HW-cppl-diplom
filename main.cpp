#include <iostream>

#if WIN32 
#include <Windows.h>
#endif

#include "ini_parser.h"


template<typename T>
void print_result(ini_parser& parser, T, std::string section, std::string name);

int main() {
#if WIN32
	setlocale(LC_ALL, "ru");
	std::cout << "\tПарсер INI файлов.\n" << std::endl;
	setlocale(LC_ALL, "C");
	SetConsoleOutputCP(CP_UTF8);
#else
	std::cout << "\tПарсер INI файлов.\n" << std::endl;
#endif // WIN32

	ini_parser parser("ini_file.txt");

	std::cout << "[Section1]" << std::endl;
	print_result(parser, double(), "Section1", "var1");
	print_result(parser, double(), "Section1", "var2");
	print_result(parser, std::string(), "Section1", "var3");
	print_result(parser, std::string(), "Section1", "var4");
	std::cout << std::endl;

	std::cout << "[Section2]" << std::endl;
	print_result(parser, int(), "Section2", "var1");
	print_result(parser, std::string(), "Section2", "var2");
	print_result(parser, std::string(), "Section2", "var3");
	std::cout << std::endl;

	std::cout << "[Section3]" << std::endl;
	std::cout << std::endl;

	std::cout << "[Section4]" << std::endl;
	print_result(parser, std::string(), "Section4", "Mode");
	print_result(parser, std::string(), "Section4", "Vid");
	std::cout << std::endl;

	std::cout << "[Section5]" << std::endl;
	print_result(parser, std::string(), "Section5", "Mode");
	std::cout << std::endl;

	std::cout << "[Section6]" << std::endl;
	print_result(parser, std::string(), "Section6", "Mode");
	std::cout << std::endl;
	
	std::cin.get();
	return 0;
}

template<typename T>
void print_result(ini_parser& parser, T, std::string section, std::string name) {
	try {
		auto value = parser.get_value<T>(section + '.' + name);
		std::cout << name << " = " << value << std::endl;
	}
	catch (ParserThrows parser_throws) {
		switch (parser_throws) {
		case ParserThrows::file_read_error:
			std::cerr << "FILE READ ERROR" << std::endl;
			std::cin.get();
			exit(0);
		case ParserThrows::section_not_found:
			std::cerr << "Incorrect section. Section not found" << std::endl;
			break;
		case ParserThrows::value_error:
			std::cerr << "Incorrect value on line " << parser.get_line() << ". ERROR FILE" << std::endl;
			std::cin.get();
			exit(0);
		case ParserThrows::no_value:
			std::cout << name << " = " << "no value" << std::endl;
			break;
		case ParserThrows::possble_variables:
			std::cout << "'" << name << "' does not exist. Maybe you were looking for: ";
			for (const auto& el : parser.get_possible_variables()) {
				std::cout << el << "   ";
			}
			std::cout << std::endl;
			break;
		case ParserThrows::invalid_type:
			std::cout << "Invalid type '" << name << "' on line " << parser.get_line() << std::endl;
			break;
		}
	}
}
