#ifndef STRING_CLASS
#define STRING_CLASS

#include <stdlib.h>
#include <stdbool.h>
#include "GenericList.h"
#include "IntList.h"

typedef struct StringObj String;

struct StringClass_t {
	String* (*new)(void);
	void (*delete)(String **obj);
	void (*print)(const String *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct StringMethods {
	void (*print)(const String *self);
	void (*toUpperCase)(String *self);
	void (*toLowerCase)(String *self);
	void (*trimWhitespace)(String *self);
	void (*setSize)(String *self, const int len);
	void (*set)(String *self, const char *newStr);
	bool (*equals)(const String *self, const char *other);
	void (*concat)(String *self, const char *newStr);
	bool (*contains)(const String *self, const char *token);
	void (*concatChar)(String *self, const char newChar);
	IntList* (*getIndexesOf)(const String *self, const char token);
	int (*getLastIndexOf)(const String *self, const char searchChar);
	int (*getFirstIndexOf)(const String *self, const char searchChar);
	void (*removeChars)(String *self, const char *unwantedChars);
	void (*trimSubString)(String *self, const int startIndex, const int endIndex);
	int (*getCharOccurences)(const String *self, const char *searchChars);
	void (*copyStrBetween)(String *self, const String *other, const int startIndex, const int endIndex);
};

struct StringObj {
	GenericList *genericList;
	char *str;
	int length;
	const struct StringMethods *methods;
};


extern const struct StringClass_t StringClass;

#endif
