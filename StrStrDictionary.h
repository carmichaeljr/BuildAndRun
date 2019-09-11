#ifndef STR_STR_DICTIONARY_CLASS
#define STR_STR_DICTIONARY_CLASS

#include <stdlib.h>
#include <stdbool.h>
#include "GenericDictionary.h"
#include "StringList.h"
#include "String.h"

typedef struct StrStrDictionaryObj StrStrDictionary;

struct StrStrDictionaryClass_t {
	StrStrDictionary* (*new)(void);
	void (*delete)(StrStrDictionary **obj);
	void (*print)(StrStrDictionary *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct StrStrDictionaryMethods {
	void (*set)(StrStrDictionary *self, const String **keys, const String **vals, const int numElements);
	void (*setVal)(StrStrDictionary *self, const String *key, const String *val);
	void (*add)(StrStrDictionary *self, const String **keys, const String **vals, const int numElements);
	String* (*get)(const StrStrDictionary *self, const String *token);
	StringList* (*getKeys)(const StrStrDictionary *self);
	bool (*keysContain)(const StrStrDictionary *self, const String *token);
	bool (*valuesContain)(const StrStrDictionary *self, const String *token);
	void (*remove)(StrStrDictionary *self, const String *token);
	void (*clear)(StrStrDictionary *self);
	bool (*equals)(const StrStrDictionary *self, const StrStrDictionary *other);
	bool (*isEmpty)(const StrStrDictionary *self);
};

struct StrStrDictKeyValuePair {
	String *key;
	String *value;
};

typedef struct StrStrDictionaryObj {
	GenericDictionary *gd;
	int size;
	bool ownsData;
	const struct StrStrDictionaryMethods *methods;
} StrStrDictionary;

extern const struct StrStrDictionaryClass_t StrStrDictionaryClass;

#endif
