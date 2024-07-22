#pragma once
#include <string>
#include <stdexcept>

template <size_t _MaxLen, typename _T>
class BasicArrayString
{
public:
	static constexpr size_t max_length = _MaxLen;

	typedef _T value_type;
	typedef _T char_type;
	typedef std::basic_string<value_type> string_type;
	typedef typename string_type::traits_type traits_type;

	constexpr void _copy(char_type *dst, const char_type *src, size_t length = max_length);
	constexpr size_t _length(const char_type *src, size_t max_length = max_length);

	constexpr BasicArrayString() = default;

	constexpr BasicArrayString(std::nullptr_t)
		: BasicArrayString() {
	}

	constexpr BasicArrayString(const char_type *cstr, const size_t length)
		: m_length{std::min(max_length, length)} {
		(void)_copy(m_data, cstr, m_length);
		m_data[m_length] = char_type();
	}

	constexpr BasicArrayString(const char_type *cstr)
		: BasicArrayString(cstr, _length(cstr)) {
	}

	constexpr explicit BasicArrayString(const char_type _char)
		: m_length{_char != char_type()} {
		// filling the string with null chars
		if (m_length)
		{
			m_data[m_length - 1] = _char;
		}

		m_data[m_length] = char_type();
	}

	constexpr explicit BasicArrayString(const size_t count, const char_type _char)
		: m_length{std::min(max_length, count)} {
		// filling the string with null chars
		if (_char == char_type())
		{
			m_length = 0;
		}
		else
		{
			for (size_t i = 0; i < m_length; i++)
			{
				m_data[i] = _char;
			}
		}

		m_data[m_length] = char_type();
	}

	template <size_t N>
	constexpr BasicArrayString(const char_type(&_arr)[N])
		: m_length{std::min(max_length, N)} {
		// filling the string with null chars
		for (size_t i = 0; i < std::min(max_length, N); i++)
		{
			if (_arr[i] == char_type())
			{
				m_length = i;
				break;
			}

			m_data[i] = _arr[i];
		}

		m_data[m_length] = char_type();
	}

	constexpr BasicArrayString(const char_type *cstr)
		: BasicArrayString(cstr, _length(cstr)) {
	}

	constexpr BasicArrayString(const string_type &str)
		: BasicArrayString(str.c_str(), str.length()) {
	}

	inline operator string_type() const {
		return string_type(m_data, m_length);
	}

	inline constexpr char_type &operator[](size_t pos) {
		if (pos > m_length)
		{
			throw std::out_of_range("pos");
		}

		return m_data[pos];
	}

	inline constexpr char_type operator[](size_t pos) const {
		if (pos > m_length)
		{
			throw std::out_of_range("pos");
		}

		return m_data[pos];
	}

	inline constexpr size_t length() const {
		return m_length;
	}
	inline constexpr size_t size() const {
		return m_length;
	}

	inline constexpr bool empty() const {
		return m_length == 0;
	}

	inline constexpr value_type *data() {
		return m_data;
	}
	inline constexpr const value_type *data() const {
		return m_data;
	}
	inline constexpr const value_type *c_str() const {
		return m_data;
	}

	inline constexpr value_type *begin() {
		return m_data;
	}
	inline constexpr value_type *end() {
		return m_data + m_length;
	}

	inline constexpr const value_type *begin() const {
		return m_data;
	}
	inline constexpr const value_type *end() const {
		return m_data + m_length;
	}

private:
	size_t m_length = 0;
	char_type m_data[max_length + 1] = {0};
};

template <size_t MaxLen>
using ArrayString = BasicArrayString<MaxLen, char>;

template <size_t MaxLen>
using WArrayString = BasicArrayString<MaxLen, wchar_t>;

template<size_t _MaxLen, typename _T>
inline constexpr void BasicArrayString<_MaxLen, _T>::_copy(char_type *dst, const char_type *src, size_t length) {
	for (size_t i = 0; i < length && src[i]; i++)
	{
		dst[i] = src[i];
	}
}

template<size_t _MaxLen, typename _T>
inline constexpr size_t BasicArrayString<_MaxLen, _T>::_length(const char_type *src, size_t max_length) {
	for (size_t i = 0; i < max_length; i++)
	{
		if (src[i] == char_type())
		{
			return i;
		}
	}

	return max_length;
}
