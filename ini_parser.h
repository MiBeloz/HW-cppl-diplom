#pragma once

#include <string>
#include <fstream>
#include <algorithm>
#include <set>
#include <regex>
#include <any>

enum class ParserThrows {
	file_read_error,
	section_not_found,
	value_error,
	no_value,
	possble_variables,
	invalid_type
};


class ini_parser final {
public:
	explicit ini_parser(const std::string& filename = "ini_file.ini") : m_filename(filename) {}
	ini_parser(const ini_parser&) = delete;
	ini_parser(ini_parser&& other) = delete;
	ini_parser& operator=(const ini_parser& other) = delete;
	ini_parser& operator=(ini_parser&& other) = delete;
	~ini_parser() {};

	template<typename T>
	T get_value(const std::string& section_value);

	std::set<std::string>& get_possible_variables() noexcept;
	size_t get_line() const noexcept;

private:
	std::string m_filename;
	std::set<std::string> m_possible_variables;
	size_t m_number_of_line = 0;

	std::any return_value(const std::string& result_value);

	std::string get_str_section(const std::string& section_value) const noexcept;
	std::string get_str_value(const std::string& section_value) const noexcept;

	bool file_section_check(std::string& file_section) noexcept;
	bool file_value_check(std::string& file_value);

	void delete_spaces(std::string& str) noexcept;
	void delete_comment(std::string& str) noexcept;
	void delete_r(std::string& str) noexcept;
};


template<typename T>
T ini_parser::get_value(const std::string& section_value) {
	m_possible_variables.clear();
	std::string result_value;
	m_number_of_line = 0;
	size_t temp_number_of_line = 0;

	bool value_found = false;
	std::ifstream file;
	file.open(m_filename, std::ifstream::in);

	if (file.is_open()) {
		std::string file_section;
		std::string file_value;

		while (!file.eof()) {
			std::getline(file, file_section, '\n');
			m_number_of_line++;

			if (file_section_check(file_section)) {
				if (file_section == '[' + get_str_section(section_value) + ']') {
					while (!file.eof()) {
						std::getline(file, file_value);
						m_number_of_line++;

						if (file_value_check(file_value)) {
							if ((file_value.size() > 1) && (file_value[0] != '[' && file_value[file_value.size() - 1] != ']')) {
								std::string temp_file_value = file_value;

								auto it = std::find(temp_file_value.begin(), temp_file_value.end(), '=');
								if (it != temp_file_value.end()) {
									temp_file_value.erase(it, temp_file_value.end());
								}

								if (temp_file_value == get_str_value(section_value)) {
									result_value = file_value;

									auto it_result_value = std::find(result_value.begin(), result_value.end(), '=');
									if (it_result_value != result_value.end()) {
										result_value.erase(result_value.begin(), ++it_result_value);
									}

									value_found = true;
									temp_number_of_line = m_number_of_line;
								}

								m_possible_variables.emplace(temp_file_value);
							}
							else {
								break;
							}
						}
					}
				}
			}
		}
	}
	else {
		throw ParserThrows::file_read_error;
	}

	file.close();

	m_number_of_line = temp_number_of_line;

	if (m_possible_variables.empty() && !value_found) {
		throw ParserThrows::section_not_found;
	}

	if (value_found) {
		m_possible_variables.clear();
	}
	else {
		throw ParserThrows::possble_variables;
	}

	if (result_value.empty()) {
		throw ParserThrows::no_value;
	}

	return std::any_cast<T>(return_value(result_value));
}

inline std::any ini_parser::return_value(const std::string& result_value) {
	const std::regex rgx_int{ "^[0-9]*$" };
	const std::regex rgx_double{ "^(0\\.[0-9]{1,9})||([1-9]{1,}[0-9]*\\.[0-9]{1,9})$" };
	std::any result_any;

	if (std::regex_match(result_value, rgx_int)) {
		result_any = std::stoi(result_value);
	}
	else if (std::regex_match(result_value, rgx_double)) {
		result_any = stod(result_value);
	}
	else {
		result_any = result_value;
	}

	return result_any;
}

inline std::set<std::string>& ini_parser::get_possible_variables() noexcept {
	return m_possible_variables;
}

inline size_t ini_parser::get_line() const noexcept {
	return m_number_of_line;
}

inline std::string ini_parser::get_str_section(const std::string& section_value) const noexcept {
	std::string section = section_value;
	auto it = find(section.begin(), section.end(), '.');
	if (it != section.end()) {
		section.erase(it, section.end());
	}

	return section;
}

inline std::string ini_parser::get_str_value(const std::string& section_value) const noexcept {
	std::string value = section_value;
	auto it = find(value.begin(), value.end(), '.');
	if (it != value.end()) {
		value.erase(value.begin(), ++it);
	}
	return value;
}

bool ini_parser::file_section_check(std::string& file_section) noexcept {
	if (file_section.size() > 0  && file_section[0] != ';') {
		delete_spaces(file_section);
		delete_comment(file_section);
		delete_r(file_section);

		if (file_section.size() > 0) {
			const std::regex rgx_section{ R"(([\[]{1})([\w\d\.\-+@]*)([\]]{1}))" };
			if (std::regex_match(file_section, rgx_section)) {
				return true;
			}
		}
	}

	return false;
}

bool ini_parser::file_value_check(std::string& file_value) {
	if (file_value.size() > 0 && file_value[0] != ';') {
		delete_spaces(file_value);
		delete_comment(file_value);
		delete_r(file_value);

		if (file_value.size() > 0) {
			const std::regex rgx_value{ R"(([\w]+)(=)([^\s]*))" };
			const std::regex rgx_section{ R"(([\[]{1})([\w\d\.\-+@]*)([\]]{1}))" };

			if (std::regex_match(file_value, rgx_value) || std::regex_match(file_value, rgx_section)) {
				return true;
			}
			else {
				throw ParserThrows::value_error;
			}
		}
	}

	return false;
}

inline void ini_parser::delete_spaces(std::string& str) noexcept {
	auto it = str.begin();
	while (it != str.end()) {
		it = std::find(str.begin(), str.end(), ' ');
		if (it != str.end()) {
			str.erase(it);
		}
	}
}

inline void ini_parser::delete_comment(std::string& str) noexcept {
	auto it = std::find(str.begin(), str.end(), ';');
	if (it != str.end()) {
		str.erase(it, str.end());
	}
}

inline void ini_parser::delete_r(std::string& str) noexcept {
	if (str.size() > 0 && str[str.size() - 1] == '\r') {
		str.erase(str.size() - 1);
	}
}
