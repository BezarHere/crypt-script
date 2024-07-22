#include "Tokenizer.hpp"
#include "Tools.hpp"
#include "ArrayString.hpp"

#include <iostream>
#include <limits>

static inline bool IsNewline(CryptChar value);
static inline bool IsWhiteSpaceNonNewline(CryptChar value);
static inline bool IsWhiteSpace(CryptChar value);
static inline bool IsDigit(CryptChar value);
static inline bool IsAlpha(CryptChar value);
static inline bool IsAlphaOrDigit(CryptChar value);
static inline bool IsPrintable(CryptChar value);

static inline bool IsIdentifierStart(CryptChar value);
static inline bool IsIdentifier(CryptChar value);

class Tokenizer
{
public:
	Tokenizer(const CryptChar *source, size_t length);

	void parse();

	inline size_t get_source_length() const { return m_source_length; }
	inline const CryptChar *get_source() const { return m_source; }

	inline const CryptChar *get_current_string() const { return &m_source[m_position]; }
	inline size_t get_space_left() const { return m_source_length - m_position; }

	inline bool empty_read() const { return m_position >= m_source_length; }

	inline size_t get_read_position() const { return m_position; }

	std::vector<Token> storage;
private:
	void _advance(offset_t offset = 1);

	errno_t _parse_token();
	Token _read_token();

	Token _read_number();
	template <typename Pred>
	Token _read_continues(TokenType type, Pred &&predicate);

	void _add_token(const Token &token);

private:
	size_t m_position = 0;

	size_t m_lines_read = 0;
	size_t m_last_line_pos = 0;

	const CryptChar *m_source;
	const size_t m_source_length;
};

void Token::Parse(const CryptChar *source, size_t length, std::vector<Token> &out_tokens) {
	if (length == 0)
	{
		length = strlen(source);
	}

	Tokenizer tokenizer = {source, length};
	tokenizer.parse();

	for (const Token &token : tokenizer.storage)
	{
		out_tokens.push_back(token);
	}
}

Tokenizer::Tokenizer(const CryptChar *source, size_t length)
	: m_source{source}, m_source_length{length} {
}

void Tokenizer::parse() {

	while (true)
	{
		errno_t error = _parse_token();

		if (error != EOK)
		{
			break;
		}
	}
}

errno_t Tokenizer::_parse_token() {
	const size_t pre_read_pos = get_read_position();
	Token token = this->_read_token();
	token.pos.line = m_lines_read;
	token.pos.column = pre_read_pos - m_last_line_pos;

	if (token.type == TokenType::EndOfFile)
	{
		return EOF;
	}

	if (token.content_length == 0)
	{
		return EBADF;
	}

	if (token.type == TokenType::Newline)
	{
		m_last_line_pos = get_read_position();
		m_lines_read += token.content_length;
	}

	this->storage.push_back(token);
	return EOK;
}

Token Tokenizer::_read_token() {
	if (empty_read())
	{
		return {TokenType::EndOfFile};
	}

	const CryptChar current_char = *get_current_string();
	const size_t space_left = get_space_left();

	std::cout << "current char: " << current_char << '\n';

	//* whitespace and newlines

	if (IsNewline(current_char))
	{
		return _read_continues(
			TokenType::Newline,
			IsNewline
		);
	}

	if (IsWhiteSpaceNonNewline(current_char))
	{
		return _read_continues(
			TokenType::Whitespace,
			IsWhiteSpaceNonNewline
		);
	}

	//* basic symbols

	constexpr std::pair<CryptChar, TokenType> SimpleCharTokenMap[] = {
		{ ',', TokenType::Comma },
		{ '{', TokenType::BraceOpen },
		{ '}', TokenType::BraceClose },
		{ '(', TokenType::ParenthesisOpen },
		{ ')', TokenType::ParenthesisClose },
		{ '#', TokenType::CommentPrefix }
	};

	for (size_t i = 0; i < std::size(SimpleCharTokenMap); i++)
	{
		if (current_char == SimpleCharTokenMap[i].first)
		{
			Token token;
			token.type = SimpleCharTokenMap[i].second;
			token.content = get_current_string();
			token.content_length = 1;

			this->_advance();

			return token;
		}
	}

	//* numbers (floats and integers)

	if (space_left > 1)
	{
		if (current_char == '-' && IsDigit(get_current_string()[1]))
		{
			return _read_number();
		}
	}

	if (IsDigit(current_char))
	{
		return _read_number();
	}

	//* operators

	struct OperatorMatch
	{
		CryptChar character;
		TokenType type;
		// set to unknow to mark a non-combined operator
		TokenType comp_type = TokenType::Unknown;
		CryptChar comp_char = '=';
	};

	constexpr OperatorMatch Operators[] = {
		{ '+', TokenType::AddOp, TokenType::AddEqOp },
		{ '-', TokenType::SubOp, TokenType::SubEqOp },
		{ '*', TokenType::MulOp, TokenType::MulEqOp },
		{ '/', TokenType::DivOp, TokenType::DivEqOp },

		{ '=', TokenType::AssignOp, TokenType::EqualityOp },
		{ '!', TokenType::NotOp, TokenType::InEqualityOp },

		// logic operators can only be compound ('&&' and '||') 
		{ '&', TokenType::Unknown, TokenType::AndOp, '&' },
		{ '|', TokenType::Unknown, TokenType::OrOp, '|' },

		// make sure the bit and/or ops come after the logic ops to not shadow them 
		{ '&', TokenType::BitAndOp, TokenType::BitAndEqOp },
		{ '|', TokenType::BitOrOp, TokenType::BitOrEqOp },
		{ '~', TokenType::BitNotOp, TokenType::BitNotEqOp },
	};

	for (size_t i = 0; i < std::size(Operators); i++)
	{
		const auto &current_op = Operators[i];

		if (current_char != current_op.character)
		{
			continue;
		}

		if (current_op.comp_type != TokenType::Unknown && space_left > 1)
		{
			if (get_current_string()[1] == current_op.comp_char)
			{
				Token token;
				token.type = current_op.comp_type;
				token.content = get_current_string();
				token.content_length = 2;
				return token;
			}
		}

		if (current_op.type == TokenType::Unknown)
		{
			continue;
		}

		Token token;
		token.type = current_op.type;
		token.content = get_current_string();
		token.content_length = 1;
		return token;
	}

	if (IsIdentifierStart(current_char))
	{
		return _read_continues(
			TokenType::Identifier,
			IsIdentifier
		);
	}

	return {TokenType::Unknown, get_current_string(), 1};
}

Token Tokenizer::_read_number() {
	return Token();
}

void Tokenizer::_add_token(const Token &token) {
	storage.push_back(token);
}

void Tokenizer::_advance(offset_t offset) {
	if (-offset > m_position)
	{
		m_position = 0;
		return;
	}

	if (m_position + offset > m_source_length)
	{
		m_position = m_source_length;
		return;
	}

	m_position += offset;
}

template<typename Pred>
Token Tokenizer::_read_continues(TokenType type, Pred &&predicate) {
	size_t count = tools::count(get_current_string(), get_space_left(), predicate);
	if (count >= std::numeric_limits<offset_t>::max())
	{
		throw std::out_of_range("count");
	}

	std::cout << count << '\n';

	Token token;
	token.content = get_current_string();
	token.content_length = count;
	token.type = type;

	_advance(count);

	return token;
}

inline bool IsNewline(CryptChar value) {
	return value == '\n'; // \r? nah
}

inline bool IsWhiteSpaceNonNewline(CryptChar value) {
	return value == '\v' || value == '\f' || value == '\r' || value == '\t' || value == ' ';
}

inline bool IsWhiteSpace(CryptChar value) {
	return IsWhiteSpaceNonNewline(value) || IsNewline(value);
}

inline bool IsDigit(CryptChar value) {
	return value >= '0' && value <= '9';
}

inline bool IsAlpha(CryptChar value) {
	return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z');
}

inline bool IsAlphaOrDigit(CryptChar value) {
	return IsDigit(value) || IsAlpha(value);
}

inline bool IsPrintable(CryptChar value) {
	return value > ' ' && value < '\x7F';
}

inline bool IsIdentifierStart(CryptChar value) {
	return IsAlpha(value) || value == '_' || value == '@';
}

inline bool IsIdentifier(CryptChar value) {
	return IsIdentifierStart(value) || IsDigit(value);
}
