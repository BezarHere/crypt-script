#include "src/Tokenizer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

int main() {
	const std::string file_path = "test.txt";
	std::ifstream file{file_path.c_str()};
	if (!file.good())
	{
		std::cout << "ERROR: " << file_path << '\n';
	}

	std::stringstream ss;
	ss << file.rdbuf();

	std::string str = ss.str();

	std::vector<Token> tks{};
	Token::Parse(str.c_str(), str.length(), tks);

	for (const auto &token : tks)
	{
		std::cout << (int)token.type << " \"" << std::string(token.content, token.content_length) << "\" " << token.pos.line << ':' << token.pos.column << '\n';
	}

}
