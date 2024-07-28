#include "Parser.hpp"
#include <stdexcept>

#include "Error.hpp"

enum ObjectType
{
	eObjType_None,
	eObjType_List,
	eObjType_Table,
};

struct TokenReadout
{
	const Token *tokens;
	size_t count;
};

struct ParseResult
{
	size_t read_count = 0;
	errno_t error = 0;
};

struct Block
{
	bool root;
	Symbol &parent;
};

static CryptString PreprocessTokenStr(const CryptChar *content, size_t size);
static ParseResult ParseValue(const Token *tokens, size_t count, crypt::Variable &out);

static CryptInt ParseInt(const CryptChar *content, size_t length);
static CryptReal ParseReal(const CryptChar *content, size_t length);
static CryptBool ParseBoolean(const CryptChar *content, size_t length);

static ParseResult ParseExpression(const TokenReadout &tokens, Symbol &out);
static ParseResult ParseBlock(const TokenReadout &tokens, const Block &block);

static ParseResult _ParseIdentifierExpr(const TokenReadout &tokens, Symbol &out);

/// @param tokens start of the object (the '{' token)
/// @param count in/out. in is the tokens count; out is the read count
/// @param count in/out. in is the tokens count; out is the read count
static ParseResult ParseObject(const Token *tokens, size_t count, crypt::Variable &out);
static ParseResult _ParseTable(const Token *tokens, size_t count, CryptTable &out);
static ParseResult _ParseList(const Token *tokens, size_t count, CryptList &out);

static ObjectType GetObjectType(const Token *tokens, size_t count);
static const Token *SkipUselessTokens(const Token *tokens, size_t count);
static size_t NextUsefulTokenIndex(const Token *tokens, size_t count);

static inline bool IsExpectedTokenTypeForTableKey(TokenType type);
static inline CryptChar UnescapeChar(CryptChar value);
static inline constexpr bool IsUselessTokenType(TokenType type);




Symbol Symbol::Parse(const Token *tokens, size_t count) {
	Symbol base;

	for (size_t i = 0; i < count; i++)
	{
		i += NextUsefulTokenIndex(&tokens[i], count - i);



	}

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

ParseResult ParseValue(const Token *tokens, size_t count, crypt::Variable &out) {
	const Token &head = tokens[0];

	const size_t tokens_count = count;
	count = 1;

	ParseResult result;


	switch (head.type)
	{
	case TokenType::Null:
		{
			break;
		}
	case TokenType::String:
		{
			out = PreprocessTokenStr(head.content, head.content_length);
			break;
		}
	case TokenType::Integer:
		{
			out = ParseInt(head.content, head.content_length);
			break;
		}
	case TokenType::Real:
		{
			out = ParseReal(head.content, head.content_length);
			break;
		}
	case TokenType::Boolean:
		{
			out = ParseBoolean(head.content, head.content_length);
			break;
		}
	case TokenType::BraceOpen:
		{
			// overriden to 1 above to suite most cases 
			count = tokens_count;
			const errno_t error = ParseObject(tokens, count, out);

			if (error != EOK)
			{
				throw std::runtime_error("parse object error");
			}

			return error;
		}
	default:
		throw std::runtime_error("invalid token list to value");
	}

	return EOK;
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

ParseResult ParseExpression(const TokenReadout &tokens, Symbol &out) {
	ParseResult result;

	size_t &index = result.read_count;
	for (;index < tokens.count; index++)
	{
		index += NextUsefulTokenIndex(&tokens.tokens[index], tokens.count - index);
		if (index > tokens.count)
		{
			break;
		}

		const Token &token = tokens.tokens[index];

		if (token.type == TokenType::Identifier)
		{
			out.



		}
	}

	return result;
}

errno_t ParseBlock(const TokenReadout &tokens, size_t &read_count, const Block &block) {
	read_count = 0;

	size_t index = 0;
	for (; index < tokens.count; index++)
	{
		index += NextUsefulTokenIndex(&tokens.tokens[index], tokens.count - index);
		if (index > tokens.count)
		{
			break;
		}

		Symbol output = {};
		ParseResult result = ParseExpression({&tokens.tokens[index], tokens.count - index}, output);
		block.parent.children.push_back(output);

		if (result.error != EOK)
		{
			//! TEMP
			throw std::runtime_error("parse error!");
		}

		if (result.read_count > 0)
		{
			index += result.read_count - 1;
		}
	}

	return errno_t();
}

ParseResult _ParseIdentifierExpr(const TokenReadout &tokens, Symbol &out) {
	if (tokens.tokens[0].type != TokenType::Identifier)
	{
		throw std::logic_error("!");
	}
	ParseResult result{};

	const auto identifier_name = \
		CryptString(tokens.tokens[0].content, tokens.tokens[0].content_length);

	size_t &index = result.read_count;
	index++;
	index += NextUsefulTokenIndex(&tokens.tokens[index], tokens.count - index);

	switch (tokens.tokens[index].type)
	{
	case TokenType::AssignOp:
		{
			out.type = SymbolType::Assign;
			out.name = identifier_name;

			index++;
			index += NextUsefulTokenIndex(&tokens.tokens[index], tokens.count - index);

			Symbol &expr = out.children.emplace_back();
			ParseResult expr_result = ParseExpression(
				{&tokens.tokens[index], tokens.count - index},
				expr
			);

			result.error = expr_result.error;
			index += expr_result.read_count;
		}
		break;
	default:
		result.error = EINVAL;
	}

	return result;
}

errno_t ParseObject(const Token *tokens, size_t &count, crypt::Variable &out) {
	if (tokens[0].type != TokenType::BraceOpen)
	{
		LOG_ERR("Expected '{' at object start %p", tokens);
		return EINVAL;
	}

	if (count < 2)
	{
		LOG_ERR("too little tokens, min is 2, given is %llu", count);
		return ERANGE;
	}

	ObjectType obj_type = GetObjectType(tokens, count);

	if (obj_type == eObjType_None)
	{
		LOG_ERR("object type returned as none, overwriting to table type, tokens=%p", tokens);
		obj_type = eObjType_Table;
	}

	if (obj_type == eObjType_List)
	{
		out = crypt::Variable(crypt::VariableType::List);
		return _ParseList(tokens, count, out.get_list());
	}

	out = crypt::Variable(crypt::VariableType::Table);
	return _ParseTable(tokens, count, out.get_table());
}

errno_t _ParseTable(const Token *tokens, size_t &count, CryptTable &out) {
	const size_t tokens_count = count;
	size_t index = 1;
	bool expecting_separator = false;
	bool found_value = false;

	for (; index < count; index++)
	{
		// skip useless
		index += SkipUselessTokens(&tokens[index], tokens_count - index) - &tokens[index];

		if (index >= count)
		{
			break;
		}

		const Token &token = tokens[index];

		if (expecting_separator)
		{
			if (token.type == TokenType::Comma)
			{
				expecting_separator = false;
				continue;
			}

			//! ERROR?
			continue;
		}

		// expecting a value
		size_t read_tokens_inout = tokens_count - index;

		out.emplace();
		const errno_t error = ParseValue(&tokens[index], read_tokens_inout, out.back());

		if (error != EOK)
		{
			LOG_ERR("error parsing value at %llu:%llu", token.pos.line, token.pos.column);
			return error;
		}

		if (read_tokens_inout > 0)
		{// out the tokens read
			index += read_tokens_inout - 1;
		}
	}

	count = index;

	return EOK;
}

errno_t _ParseList(const Token *tokens, size_t &count, CryptList &out) {
	const size_t tokens_count = count;
	size_t index = 1;
	bool expecting_separator = false;

	for (; index < count; index++)
	{
		// skip useless
		index += SkipUselessTokens(&tokens[index], tokens_count - index) - &tokens[index];

		if (index >= count)
		{
			break;
		}

		const Token &token = tokens[index];

		if (expecting_separator)
		{
			if (token.type == TokenType::Comma)
			{
				expecting_separator = false;
				continue;
			}

			//! ERROR?
			continue;
		}

		// expecting a value
		size_t read_tokens_inout = tokens_count - index;

		out.emplace_back();
		const errno_t error = ParseValue(&tokens[index], read_tokens_inout, out.back());

		if (error != EOK)
		{
			LOG_ERR("error parsing value at %llu:%llu", token.pos.line, token.pos.column);
			return error;
		}

		if (read_tokens_inout > 0)
		{// out the tokens read
			index += read_tokens_inout - 1;
		}
	}

	count = index;

	return EOK;
}

ObjectType GetObjectType(const Token *tokens, size_t count) {
	if (tokens[0].type == TokenType::BraceOpen)
	{
		tokens++;
		count--;
	}
	size_t index = 0;

	index += NextUsefulTokenIndex(&tokens[index], count - index);

	// no more tokens or we overflowed (no more tokens too)
	if (index >= count)
	{
		// it's not a list... or anything valid in-fact
		return eObjType_None;
	}

	// ex: '{}'
	// - this can be a list -> { {} }
	// - this can NOT be a table -> { {} = "hello" }
	if (!IsExpectedTokenTypeForTableKey(tokens[index].type))
	{
		return eObjType_List;
	}

	// skip current token
	index++;
	// go to the next useful token
	index += NextUsefulTokenIndex(&tokens[index], count - index);

	// no more tokens or we overflowed (no more tokens too)
	if (index >= count)
	{
		return eObjType_None;
	}

	// assign op after a value ('name = '), must be a table; not a list
	if (tokens[index].type == TokenType::AssignOp)
	{
		return eObjType_Table;
	}

	// comma after a value ('name ,'), can't be a table; must be a list
	if (tokens[index].type == TokenType::Comma)
	{
		return eObjType_List;
	}

	return eObjType_None;
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

size_t NextUsefulTokenIndex(const Token *tokens, size_t count) {
	const Token *useful_token = SkipUselessTokens(tokens, count);
	return useful_token - tokens;
}

inline bool IsExpectedTokenTypeForTableKey(TokenType type) {
	return type == TokenType::Identifier || type == TokenType::String;
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
