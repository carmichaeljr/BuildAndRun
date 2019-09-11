#ifndef GENERIC_DICTIONARY_CLASS
#define GENERIC_DICTIONARY_CLASS

#include <stdlib.h>
#include <stdbool.h>
#include "GenericLinkedList.h"

typedef struct GenericDictionaryObj GenericDictionary;

struct GenericDictionaryClass_t {
	GenericDictionary* (*new)(void);
	void (*delete)(GenericDictionary **obj);
	void (*print)(GenericDictionary *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct GenericDictionaryMethods {
	void (*setElementSize)(GenericDictionary *self,const size_t keySize, const size_t valSize);
	void (*set)(GenericDictionary *self, const void *keys, const void *vals, const int numElements);
	void (*setVal)(GenericDictionary *self, const void *key, const void *value);
	void (*add)(GenericDictionary *self, const void *key, const void *value);
	void* (*get)(const GenericDictionary *self, const void *token);
	bool (*keysContain)(const GenericDictionary *self, const void *token);
	bool (*valuesContain)(const GenericDictionary *self, const void *token);
	void (*remove)(GenericDictionary *self, const void *token);
	void (*clear)(GenericDictionary *self);
	bool (*equals)(const GenericDictionary *self, const GenericDictionary *other);
	bool (*isEmpty)(const GenericDictionary *self);
};

typedef struct GenericDictionaryObj {
	GenericLinkedList *keys;
	GenericLinkedList *values;
	int numElements;
	const struct GenericDictionaryMethods *methods;
} GenericDictionary;

extern const struct GenericDictionaryClass_t GenericDictionaryClass;

#endif
