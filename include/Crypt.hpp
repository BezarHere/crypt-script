#ifndef _CRYPT_H_
#define _CRYPT_H_
#include <inttypes.h>

#include <map>
#include <vector>
#include <string>
#include <stdexcept>

#ifndef EOK
#define EOK 0
#endif

namespace crypt
{
	class Variable;

	typedef char char_type;
	typedef std::basic_string<char_type> string_type;

	typedef bool boolean_type;
	typedef intptr_t int_type;
	typedef float real_type;

	typedef std::vector<Variable> list_type;
	typedef std::map<string_type, Variable> table_type;

	enum class VariableType : uint8_t
	{
		Null,
		Bool,
		Int,
		Real,
		Str,
		List, // array
		Table // dict/map
	};

	class VariableAccessError : std::runtime_error
	{
	public:
		inline VariableAccessError(const std::string &msg) : std::runtime_error(msg) {}
		inline VariableAccessError(const char *msg) : std::runtime_error(msg) {}
	};

	class Variable
	{
	public:
		static constexpr auto _null = VariableType::Null;

		Variable(VariableType type = _null);
		Variable(boolean_type value);
		Variable(int_type value);
		Variable(real_type value);
		Variable(const string_type &value);
		Variable(const list_type &value);
		Variable(const table_type &value);

		Variable(const Variable &copy);
		Variable(Variable &&move) noexcept;

		Variable &operator=(const Variable &copy);
		Variable &operator=(Variable &&move) noexcept;

		~Variable();

		inline bool is_null() const noexcept { return m_type == _null; }

		boolean_type get_bool() const;
		int_type get_int() const;
		real_type get_real() const;

		string_type &get_string();
		list_type &get_list();
		table_type &get_table();

		const string_type &get_string() const;
		const list_type &get_list() const;
		const table_type &get_table() const;

	private:
		template <typename _Proc>
		decltype(auto) __apply(_Proc &&proc);

	private:
		VariableType m_type;
		union
		{
			boolean_type m_boolean;
			int_type m_integer;
			real_type m_real;
			string_type m_string;
			list_type m_list;
			table_type m_table;
		};
	};

}

#endif