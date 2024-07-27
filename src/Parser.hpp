#pragma once
#include "Common.hpp"
#include "Tokenizer.hpp"

enum class SymbolType : uint8_t
{
	Invalid,

	Identifier, // identifier name is the `name` field
	Assign,
	Value,
};

struct Symbol
{
	SymbolType type = SymbolType::Invalid;

	crypt::Variable value;
	CryptString name;

	std::vector<Symbol> children;

	static Symbol Parse(const Token *tokens, size_t count);
};
