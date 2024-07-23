#pragma once
#include "Crypt.hpp"

#ifndef EOK
#define EOK 0
#endif

typedef intptr_t offset_t;

typedef crypt::char_type CryptChar;
typedef crypt::string_type CryptString;
typedef crypt::boolean_type CryptBool;
typedef crypt::int_type CryptInt;
typedef crypt::real_type CryptReal;
typedef crypt::list_type CryptList;
typedef crypt::table_type CryptTable;

static constexpr const CryptChar *CryptNull = "null"; 

static constexpr const CryptChar *BooleanNames[] = {
	"false",
	"true"
};

static constexpr const CryptChar *CryptKeywords[] = {
	"func"
};
