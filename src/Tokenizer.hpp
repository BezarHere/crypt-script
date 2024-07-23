#pragma once
#include "CryptString.hpp"
#include "Tools.hpp"

#include <vector>
#include <inttypes.h>

enum class TokenType : uint8_t
{
	Unknown,
	EndOfFile,

	CommentPrefix,

	Whitespace,
	Newline,

	Null,
	Boolean,
	Keyword,

	String,
	Identifier,

	Integer,
	Real,

	AddOp,
	SubOp,
	MulOp,
	DivOp,
	
	AddEqOp,
	SubEqOp,
	MulEqOp,
	DivEqOp,

	AssignOp,
	EqualityOp,

	NotOp,
	InEqualityOp,

	AndOp,
	OrOp,

	BitAndOp,
	BitOrOp,
	BitNotOp,

	BitAndEqOp,
	BitOrEqOp,
	BitNotEqOp,

	ParenthesisOpen,
	ParenthesisClose,

	BraceOpen,
	BraceClose,

	Comma
};

struct TextPosition
{
	uint32_t line = 0;
	uint32_t column = 0;
};

struct Token
{
	TokenType type;

	const CryptChar *content = nullptr;
	size_t content_length = 0;

	TextPosition pos = {};

	static void Parse(const CryptChar *source, size_t length, std::vector<Token> &out_tokens);
};

