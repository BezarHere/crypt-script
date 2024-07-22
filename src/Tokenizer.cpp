#include "Tokenizer.hpp"
#include "Tools.hpp"
#include "ArrayString.hpp"


static inline bool IsNewline(CryptChar value);
static inline bool IsWhiteSpaceNonNewline(CryptChar value);
static inline bool IsWhiteSpace(CryptChar value);
static inline bool IsDigit(CryptChar value);
static inline bool IsAlpha(CryptChar value);
static inline bool IsAlphaOrDigit(CryptChar value);
static inline bool IsPrintable(CryptChar value);

class Tokenizer
{
public:
	Tokenizer(const CryptChar *source, size_t length);

	void parse();

	inline size_t get_source_length() const { return m_source_length; }
	inline const CryptChar *get_source() const { return m_source; }

	errno_t _parse_token();
	Token _read_token();

	template <typename Pred>
	Token _read_continues(TokenType type, TextPosition pos, Pred &&predicate);

	void _add_token(const Token &token);

	inline const CryptChar *get_current_string() const { return &m_source[m_position]; }
	inline size_t get_space_left() const { return m_source_length - m_position; }

	inline bool empty_read() const { return m_position >= m_source_length; }

	void advance(int offset = 1);

	std::vector<Token> storage;
private:
	size_t m_position = 0;

	TextPosition m_text_pos = {0, 0};

	const size_t m_source_length;
	const CryptChar *m_source;
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
	Token token = this->_read_token();

	if (token.type == TokenType::EndOfFile)
	{
		return EOF;
	}

	if (token.content_length == 0)
	{
		return EBADF;
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

	if (IsNewline(current_char))
	{
		return _read_continues(
			TokenType::Newline,
			m_text_pos,
			IsNewline
		);
	}

	if (IsWhiteSpaceNonNewline(current_char))
	{
		return _read_continues(
			TokenType::Whitespace,
			m_text_pos,
			IsWhiteSpaceNonNewline
		);
	}

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
			token.pos = m_text_pos;

			this->advance();

			return token;
		}
	}

	return {TokenType::Unknown, get_current_string(), 1, m_text_pos};
}

void Tokenizer::_add_token(const Token &token) {
	storage.push_back(token);
}

void Tokenizer::advance(int offset) {
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
Token Tokenizer::_read_continues(TokenType type, TextPosition pos, Pred &&predicate) {
	size_t count = tools::count(get_current_string(), get_space_left(), predicate);
	Token tk;
	tk.content = get_current_string();
	tk.content_length = count;
	tk.type = type;
	tk.pos = pos;

	advance(count);

	return tk;
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
