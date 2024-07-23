#include "Crypt.hpp"

struct ConstructDefault
{
	template <typename T>
	inline void operator()(T &value) const {
		new (&value) T();
	}

	inline void operator()() const {
	}
};

struct Deconstruct
{
	template <typename T>
	inline void operator()(T &value) const {
		value.~T();
	}

	inline void operator()() const {
	}
};

struct CopyConstruct
{
	inline CopyConstruct(const void *_source) : source{_source} {}

	template <typename T>
	inline void operator()(T &value) const {
		new (&value) T(*((const T *)this->source));
	}

	inline void operator()() const {
	}

	const void *source;
};

struct MoveConstruct
{
	inline MoveConstruct(void *_source) : source{_source} {}

	template <typename T>
	inline void operator()(T &value) const {
		new (&value) T(std::forward<T>(*((T *)this->source)));
	}

	inline void operator()() const {
	}

	void *source;
};

struct CopyAssign
{
	inline CopyAssign(const void *_source) : source{_source} {}

	template <typename T>
	inline void operator()(T &value) const {
		value = *((const T *)this->source);
	}

	inline void operator()() const {
	}

	const void *source;
};

struct MoveAssign
{
	inline MoveAssign(void *_source) : source{_source} {}

	template <typename T>
	inline void operator()(T &value) const {
		value = std::forward<T>(*((T *)this->source));
	}

	inline void operator()() const {
	}

	void *source;
};

namespace crypt
{
	template<typename _Proc>
	decltype(auto) Variable::__apply(_Proc &&proc) {
		switch (m_type)
		{
		case VariableType::Bool:
			return proc(m_boolean);
		case VariableType::Int:
			return proc(m_integer);
		case VariableType::Real:
			return proc(m_real);
		case VariableType::Str:
			return proc(m_string);
		case VariableType::List:
			return proc(m_list);
		case VariableType::Table:
			return proc(m_table);

		case VariableType::Null:
		default:
			return proc();
		}
	}

	Variable::Variable(VariableType type) : m_type{type} {
		this->__apply(ConstructDefault());
	}

	Variable::Variable(boolean_type value)
		: m_type{VariableType::Bool}, m_boolean{value} {
	}

	Variable::Variable(int_type value)
		: m_type{VariableType::Int}, m_integer{value} {
	}

	Variable::Variable(real_type value)
		: m_type{VariableType::Real}, m_real{value} {
	}

	Variable::Variable(const string_type &value)
		: m_type{VariableType::Str}, m_string{value} {
	}

	Variable::Variable(const list_type &value)
		: m_type{VariableType::List}, m_list{value} {
	}

	Variable::Variable(const table_type &value)
		: m_type{VariableType::Table}, m_table{value} {
	}

	Variable::Variable(const Variable &copy) : m_type{copy.m_type} {
		this->__apply(CopyConstruct(&copy.m_boolean));
	}

	Variable::Variable(Variable &&move) noexcept : m_type{move.m_type} {
		this->__apply(MoveConstruct(&move.m_boolean));
	}

	Variable &Variable::operator=(const Variable &copy) {
		if (std::addressof(copy) == this)
		{
			return *this;
		}

		this->__apply(Deconstruct());
		m_type = copy.m_type;
		this->__apply(CopyAssign(&copy.m_boolean));
		return *this;
	}

	Variable &Variable::operator=(Variable &&move) noexcept {
		this->__apply(Deconstruct());
		m_type = move.m_type;
		this->__apply(MoveAssign(&move.m_boolean));
		return *this;
	}

	Variable::~Variable() {
		this->__apply(Deconstruct());
	}

	boolean_type Variable::get_bool() const {
		switch (m_type)
		{
		case VariableType::Null:
			return false;
		case VariableType::Bool:
			return m_boolean;
		case VariableType::Int:
			return m_integer != 0;
		case VariableType::Real:
			return m_real != 0;
		default:
			throw VariableAccessError("boolean");
		}
	}

	int_type Variable::get_int() const {
		switch (m_type)
		{
		case VariableType::Null:
			return 0;
		case VariableType::Bool:
			return m_boolean ? 1 : 0;
		case VariableType::Int:
			return m_integer;
		case VariableType::Real:
			return static_cast<int_type>(m_real);
		default:
			throw VariableAccessError("int");
		}
	}

	real_type Variable::get_real() const {
		switch (m_type)
		{
		case VariableType::Null:
			return 0;
		case VariableType::Bool:
			return static_cast<real_type>(m_boolean ? 1 : 0);
		case VariableType::Int:
			return static_cast<real_type>(m_integer);
		case VariableType::Real:
			return m_real;
		default:
			throw VariableAccessError("real");
		}
	}

	const string_type &Variable::get_string() const {
		if (m_type != VariableType::Str)
		{
			throw VariableAccessError("string");
		}

		return m_string;
	}

	const list_type &Variable::get_list() const {
		if (m_type != VariableType::List)
		{
			throw VariableAccessError("list");
		}

		return m_list;
	}

	const table_type &Variable::get_table() const {
		if (m_type != VariableType::Table)
		{
			throw VariableAccessError("table");
		}

		return m_table;
	}
}
