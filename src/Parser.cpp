#include "Parser.hpp"
#include <stdexcept>


static CryptString PreprocessTokenStr(const CryptChar *content, size_t size);
static Symbol ParseValue(const Token *tokens, size_t &count);

static CryptInt ParseInt(const CryptChar *content, size_t length);
static CryptReal ParseReal(const CryptChar *content, size_t length);
static CryptBool ParseBoolean(const CryptChar *content, size_t length);

/// @param tokens start of the object (the '{' token)
/// @param count in/out. in is the tokens count; out is the read count
/// @param count in/out. in is the tokens count; out is the read count
static errno_t ParseObject(const Token *tokens, size_t &count, Symbol &out);

static bool IsObjectAnArray(const Token *tokens, size_t count);
static const Token *SkipUselessTokens(const Token *tokens, size_t count);

static inline CryptChar UnescapeChar(CryptChar value);
static inline constexpr bool IsUselessTokenType(TokenType type);

Symbol Symbol::Parse(const Token *tokens, size_t count) {
	Symbol base;

	return base;
}

CryptString PreprocessTokenStr(const CryptChar *content, const size_t size) {
	CryptString result{};
	result.resize(size);

	offset_t result_head = 0;
	for (size_t i = 0; i < size; i++)
	{
		if (content[i] == '\\')
		{
			// skip escape
			i++;

			result[result_head++] = UnescapeChar(content[i]);
			continue;
		}

		result[result_head++] = content[i];
	}

	return result;
}

Symbol ParseValue(const Token *tokens, size_t &count) {
	const Token &head = tokens[0];
	Symbol result;

	const size_t tokens_count = count;
	count = 1;

	result.type = SymbolType::Value;

	switch (head.type)
	{
	case TokenType::Null:
		{
			break;
		}
	case TokenType::String:
		{
			result.value = PreprocessTokenStr(head.content, head.content_length);
			break;
		}
	case TokenType::Integer:
		{
			result.value = ParseInt(head.content, head.content_length);
			break;
		}
	case TokenType::Real:
		{
			result.value = ParseReal(head.content, head.content_length);
			break;
		}
	case TokenType::Boolean:
		{
			result.value = ParseBoolean(head.content, head.content_length);
			break;
		}
	case TokenType::BraceOpen:
		{
			// overriden to 1 above to suite most cases 
			count = tokens_count;
			const errno_t error = ParseObject(tokens, count, result);
			if (error != EOK)
			{
				throw std::runtime_error("parse object error");
			}

			break;
		}
	default:
		throw std::runtime_error("invalid token list to value");
	}

	return result;
}

CryptInt ParseInt(const CryptChar *content, size_t length) {
	if (length == 0 || content == nullptr)
	{
		//! temp
		throw std::runtime_error("invalid args");
	}

	constexpr CryptInt base = 10;
	CryptInt mul = 1;
	CryptInt value = 0;

	if (content[0] == '-')
	{
		mul = -1;
		content++;
		length--;
	}

	for (size_t i = 0; i < length; i++)
	{
		if (content[i] > '9' || content[i] < '0')
		{
			//! temp
			throw std::runtime_error("unexpected char in int parsing");
		}

		value += mul * (content[i] - '0');
		mul *= base;
	}

	return value;
}

CryptReal ParseReal(const CryptChar *content, size_t length) {
	if (length == 0 || content == nullptr)
	{
		//! temp
		throw std::runtime_error("invalid args");
	}

	constexpr CryptReal PreDecimalBase = 10;
	constexpr CryptReal DecimalBase = 0.1;

	CryptReal base = PreDecimalBase;
	CryptReal mul = 1;
	CryptReal value = 0;

	if (content[0] == '-')
	{
		mul = -1;
		content++;
		length--;
	}

	const CryptChar *decimal_dot = nullptr;
	for (size_t i = 0; i < length; i++)
	{
		if (decimal_dot == nullptr && content[i] == '.')
		{
			decimal_dot = &content[i];
			base = DecimalBase;

			if (mul > 0)
			{
				mul = DecimalBase;
			}
			else
			{
				mul = -DecimalBase;
			}
		}

		if (content[i] > '9' || content[i] < '0')
		{
			//! temp
			throw std::runtime_error("unexpected char in int parsing");
		}

		value += mul * CryptReal(content[i] - '0');
		mul *= base;
	}

	return value;
}

CryptBool ParseBoolean(const CryptChar *content, size_t length) {
	if (StringEqual(content, BooleanNames[false], length))
	{
		return false;
	}

	if (StringEqual(content, BooleanNames[true], length))
	{
		return true;
	}

	//! temp
	throw std::runtime_error("invalid boolean");
	// return false;
}

errno_t ParseObject(const Token *tokens, size_t &count, Symbol &out) {
	bool expecting_separator = false;


	for (size_t i = 0; i < count; i++)
	{
	}

	return errno_t();
}

bool IsObjectAnArray(const Token *tokens, size_t count) {

	return false;
}

const Token *SkipUselessTokens(const Token *tokens, size_t count) {
	size_t index = 0;
	for (; index < count; index++)
	{
		// comment, skip it
		if (tokens[index].type == TokenType::CommentPrefix)
		{
			// skip to the next newline
			index = tools::find(
				tokens,
				index + 1,
				count,
				[](const Token &token) { return token.type == TokenType::Newline; }
			);
			continue;
		}

		// useless tokens, skip it
		if (IsUselessTokenType(tokens[index].type))
		{
			continue;
		}

		// not useless? stop skipping
		break;
	}

	return &tokens[index];
}

inline CryptChar UnescapeChar(CryptChar value) {
	switch (value)
	{
	case 'n':
		return '\n';
	case 'r':
		return '\r';
	case 'v':
		return '\v';
	case 't':
		return '\t';
	case 'f':
		return '\f';
	default:
		return value;
	}
}

inline constexpr bool IsUselessTokenType(TokenType type) {
	return type == TokenType::Newline || type == TokenType::Whitespace;
}
