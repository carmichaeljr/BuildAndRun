#ifndef GENERIC_LINKED_LIST_CLASS
#define GENERIC_LINKED_LIST_CLASS

#include <stdlib.h>
#include <stdbool.h>

typedef struct GenericLinkedListObj GenericLinkedList;

struct GenericLinkedList_t {
	GenericLinkedList* (*new)(void);
	void (*delete)(GenericLinkedList **obj);
	void (*print)(const GenericLinkedList *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct GenericLinkedListMethods {
	void (*setElementSize)(GenericLinkedList *self, const size_t newSize);
	void (*set)(GenericLinkedList *self, const void *newElements, const int numElements);
	void (*setAt)(GenericLinkedList *self, const void *newElements, const int numElements, const int index);
	void (*add)(GenericLinkedList *self, const void *newElements, const int numElements);
	void (*addAt)(GenericLinkedList *self, const void *newElements, const int numElements, const int index);
	void* (*get)(const GenericLinkedList *self, const int index);
	bool (*contains)(const GenericLinkedList *self, const void *token);
	int (*getFirstIndexOf)(const GenericLinkedList *self, const void *token);
	int (*getLastIndexOf)(const GenericLinkedList *self, const void *token);
	void (*remove)(GenericLinkedList *self, const void *token);
	void (*removeAt)(GenericLinkedList *self, const int index);
	void (*removeBetween)(GenericLinkedList *self, const int startIndex, const int endIndex);
	void (*trimToSize)(GenericLinkedList *self, const int numElements);
	void (*copyOtherBetween)(GenericLinkedList *self, const GenericLinkedList *other, const int startIndex, const int endIndex);
	void (*clear)(GenericLinkedList *self);
	bool (*isEmpty)(const GenericLinkedList *self);
	bool (*equals)(const GenericLinkedList *self, const GenericLinkedList *other);
};

struct GenericLinkedListNode {
	void *element;
	struct GenericLinkedListNode *previous;
	struct GenericLinkedListNode *next;
};

struct GenericLinkedListObj {
	const struct GenericLinkedListMethods *methods;
	size_t elementSize;
	int numElements;
	struct GenericLinkedListNode *head;
	struct GenericLinkedListNode *tail;
};

extern const struct GenericLinkedList_t GenericLinkedListClass;

#endif
