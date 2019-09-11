#ifndef INTLIST_CLASS
#define INTLIST_CLASS

#include <stdlib.h>
#include <stdbool.h>
#include "GenericList.h"

typedef struct IntListObj IntList;

struct IntListClass_t {
	IntList* (*new)(void);
	void (*delete)(IntList **obj);
	void (*print)(IntList *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct IntListMethods {
	void (*add)(IntList *self, const int num);
	void (*splice)(IntList *self, const IntList *other);
	void (*setSize)(IntList *self, const int len);
	void (*sort)(IntList *self);
	int (*getFirstIndexOf)(const IntList *self, const int num);
	int (*getLastIndexOf)(const IntList *self, const int num);
	int (*getNumOccurences)(const IntList *self, const int searchNum);
	void (*copyOtherBetween)(IntList *self, const IntList *other, const int startIndex, const int endIndex);
	void (*remove)(IntList *self, const int unwantedNum);
	void (*removeNumRange)(IntList *self, const int rangeStart, const int rangeEnd);
	void (*removeAt)(IntList *self, const int index);
	void (*removeBetween)(IntList *self, const int startIndex, const int endIndex);
	void (*trimSubList)(IntList *self, const int startIndex, const int endIndex);
	IntList* (*getIndexesOf)(const IntList *self, const int token);
	bool (*contains)(const IntList *self, const int token);
	bool (*equals)(const IntList *self, const int *other);
	void (*clear)(IntList *self);
};

typedef struct IntListObj {
	GenericList *genericList;
	int *list;
	int length;
	const struct IntListMethods *methods;
} IntList;

extern const struct IntListClass_t IntListClass;

#endif
