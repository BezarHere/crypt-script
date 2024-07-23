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

// specializes the token to a keyword token or boolean token or ...
static TokenType IdentifierTokenSpecialtyType(const Token &token);

constexpr CryptChar StringChar = '"';

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
	void _advance(size_t amount = 1);

	errno_t _parse_token();
	Token _read_token();

	Token _read_number();
	Token _read_string();
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

	// debug
	if (m_position - pre_read_pos == 0)
	{
		std::cout << "ERROR! at token " << pre_read_pos << "\n";
		return EFAULT;
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

	//* strings

	if (current_char == StringChar)
	{
		return this->_read_string();
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

				this->_advance(2);

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

		this->_advance();

		return token;
	}

	if (IsIdentifierStart(current_char))
	{
		Token token = _read_continues(
			TokenType::Identifier,
			IsIdentifier
		);

		token.type = IdentifierTokenSpecialtyType(token);
		return token;
	}

	// unknown token
	this->_advance();
	return {TokenType::Unknown, get_current_string(), 1};
}

Token Tokenizer::_read_number() {
	const CryptChar *decimal_dot = nullptr;
	size_t index = 0;

	// skip negation
	if (get_current_string()[0] == '-')
	{
		index = 1;
	}

	for (; index < get_source_length(); index++)
	{
		const CryptChar cur_chr = get_current_string()[index];
		if (IsDigit(cur_chr))
		{
			continue;
		}

		if (decimal_dot == nullptr && cur_chr == '.')
		{
			decimal_dot = get_current_string() + index;
			continue;
		}

		break;
	}

	const CryptChar *current_str = get_current_string();

	this->_advance(index);

	return {
		decimal_dot == nullptr ? TokenType::Integer : TokenType::Real,
		current_str, index
	};
}

Token Tokenizer::_read_string() {
	size_t index = 1;

	const size_t space_left = get_space_left();
	const CryptChar *const current_str = get_current_string();

	for (; index < space_left; index++)
	{
		// skip the escape and the char after it
		if (current_str[index] == '\\')
		{
			index++;
			continue;
		}

		if (current_str[index] == StringChar)
		{
			break;
		}
	}

	this->_advance(index + 1);
	return {
		TokenType::String,
		current_str + 1, index - 1
	};
}

void Tokenizer::_add_token(const Token &token) {
	storage.push_back(token);
}

void Tokenizer::_advance(size_t amount) {
	if (m_position + amount > m_source_length)
	{
		m_position = m_source_length;
		return;
	}

	m_position += amount;
}

template<typename Pred>
Token Tokenizer::_read_continues(TokenType type, Pred &&predicate) {
	size_t count = tools::count(get_current_string(), get_space_left(), predicate);
	if (count >= std::numeric_limits<offset_t>::max())
	{
		throw std::out_of_range("count");
	}

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

TokenType IdentifierTokenSpecialtyType(const Token &token) {
	if (StringEqual(CryptNull, token.content, token.content_length))
	{
		return TokenType::Null;
	}

	for (size_t i = 0; i < std::size(BooleanNames); i++)
	{
		if (StringEqual(BooleanNames[i], token.content, token.content_length))
		{
			return TokenType::Boolean;
		}
	}

	for (size_t i = 0; i < std::size(CryptKeywords); i++)
	{
		if (StringEqual(CryptKeywords[i], token.content, token.content_length))
		{
			return TokenType::Keyword;
		}
	}

	return token.type;
}
