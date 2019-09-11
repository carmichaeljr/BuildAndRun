#ifndef XML_TAG_CLASS
#define XML_TAG_CLASS

#include <stdbool.h>
#include "String.h"
#include "StrStrDictionary.h"

typedef struct XMLTagObj XMLTag;

struct XMLTagClass_t {
	XMLTag* (*new)(void);
	void (*delete)(XMLTag **obj);
	void (*print)(XMLTag *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct XMLTagMethods {
	void (*parseFromRaw)(XMLTag *self, const String *raw);
	String* (*getRawOpeningTag)(const XMLTag *self);
	String* (*getRawClosingTag)(const XMLTag *self);
	void (*setName)(XMLTag *self, const String *newName);
	void (*setData)(XMLTag *self, const String *newData);
	void (*setAttribute)(XMLTag *self, const String *key, const String *val);
	void (*addAttribute)(XMLTag *self, const String *key, const String *val);
	String* (*getAttribute)(const XMLTag *self, const String *key);
	void (*removeAttribute)(XMLTag *self, const String *key);
	void (*clearAttributes)(XMLTag *self);
	void (*clear)(XMLTag *self);
	bool (*equals)(const XMLTag *self, const XMLTag *other);
};

typedef struct XMLTagObj {
	String *name;
	String *data;
	StrStrDictionary *attributes;
	const struct XMLTagMethods *methods;
} XMLTag;

extern const struct XMLTagClass_t XMLTagClass;

#endif
