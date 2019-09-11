#ifndef STRINGLIST_CLASS
#define STRINGLIST_CLASS

#include <stdlib.h>
#include <stdbool.h>
#include "GenericLinkedList.h"
#include "String.h"

typedef struct StringListObj StringList;

struct StringListClass_t {
	StringList* (*new)(void);
	void (*delete)(StringList **obj);
	void (*print)(StringList *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct StringListMethods {
	void (*set)(StringList *self, const String **strs, const int numElements);
	void (*setAt)(StringList *self, const String **strs, const int numElements, const int index);
	void (*add)(StringList *self, const String **strs, const int numElements);
	void (*addAt)(StringList *self, const String **strs, const int numElements, const int index);
	void (*copyOtherBetween)(StringList *self, const StringList *other, const int startIndex, const int endIndex);
	void (*listFromDelimiter)(StringList *self, const char *str, const char *delim);
	String* (*get)(const StringList *self, const int index);
	bool (*contains)(const StringList *self, const String *str);
	int (*getFirstIndexOf)(const StringList *self, const String *str);
	int (*getLastIndexOf)(const StringList *self, const String *str);
	void (*remove)(StringList *self, const String *str);
	void (*removeAt)(StringList *self, const int index);
	void (*removeBetween)(StringList *self, const int startIndex, const int endIndex);
	void (*trimToSize)(StringList *self, const int numElements);
	void (*clear)(StringList *self);
	bool (*equals)(const StringList *self, const StringList *other);
	bool (*isEmpty)(const StringList *self);
};

typedef struct StringListObj {
	GenericLinkedList *gll;
	int length;
	bool ownsData;
	const struct StringListMethods *methods;
} StringList;

extern const struct StringListClass_t StringListClass;

#endif
