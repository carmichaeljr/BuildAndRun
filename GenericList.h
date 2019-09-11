#ifndef GENERICLIST_CLASS
#define GENERICLIST_CLASS

#include <stdlib.h>
#include <stdbool.h>

typedef struct GenericListObj GenericList;

struct GenericList_t {
	GenericList* (*new)(void);
	void (*delete)(GenericList **obj);
	void (*print)(const GenericList *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct GenericListMethods {
	void (*setElementSize)(GenericList *self, const size_t newSize);
	void (*setListSize)(GenericList *self, const int numElements);
	void (*set)(GenericList *self, const void *newElements, const int numElements);
	void (*setAt)(GenericList *self, const void *newElements, const int numElements, const int index);
	void (*add)(GenericList *self, const void *newElement, const int numElements);
	void (*addAt)(GenericList *self, const void *newElement, const int numElements,const int index);
	void (*copyOtherBetween)(GenericList *self, const GenericList *other, const int startIndex, const int endIndex);
	void* (*get)(const GenericList *self, const int index);
	bool (*contains)(const GenericList *self, const void *token);
	int (*getFirstIndexOf)(const GenericList *self, const void *token);
	int (*getLastIndexOf)(const GenericList *self, const void *token);
	void (*remove)(GenericList *self, const void *token);
	void (*removeAt)(GenericList *self, const int index);
	void (*removeBetween)(GenericList *self, const int startIndex, const int endIndex);
	void (*trimToSize)(GenericList *self, const int numElements);
	void (*clear)(GenericList *self);
	bool (*isEmpty)(const GenericList *self);
	bool (*equals)(const GenericList *self, const GenericList *other);
};

struct GenericListObj {
	void *list;
	int numElements;
	size_t elementSize;
	const struct GenericListMethods *methods;
};

extern const struct GenericList_t GenericListClass;

#endif
