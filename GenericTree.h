#ifndef GENERIC_TREE_CLASS
#define GENERIC_TREE_CLASS

#include <stdbool.h>
#include "GenericLinkedList.h"

typedef struct GenericTreeObj GenericTree;

struct GenericTreeClass_t {
	GenericTree* (*new)(void);
	void (*delete)(GenericTree **obj);
	void (*print)(GenericTree *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct GenericTreeMethods {
	void (*setElementSize)(GenericTree *self, const size_t newSize);
	bool (*placeCurrentNode)(GenericTree *self, const void *dataList, const int numNodes);
	void (*placeCurrentNodeAtHeadNode)(GenericTree *self);
	bool (*moveDown)(GenericTree *self);
	bool (*moveNext)(GenericTree *self);
	bool (*moveUp)(GenericTree *self);
	void (*add)(GenericTree *self, const void *data);
	void (*addAt)(GenericTree *self, const void *dataList, const int numNodes, const void *data);
	void (*set)(GenericTree *self, const void *data);
	void (*setAt)(GenericTree *self, const void *dataList, const int numNodes, const void *data);
	void* (*get)(const GenericTree *self);
	void* (*getAt)(GenericTree *self, const void *dataList, const int numNodes);
	int (*getNumChildNodes)(const GenericTree *self);
	int (*getNumChildNodesAt)(GenericTree *self, const void *dataList, const int numNodes);
	bool (*branchExists)(const GenericTree *self, const void *dataList, const int numNodes);
	void (*copyOtherBelowCurNode)(GenericTree *self, GenericTree *other);
	void (*remove)(GenericTree *self);
	void (*clear)(GenericTree *self);
	bool (*startIter)(GenericTree *self);
	bool (*startIterAtCurNode)(GenericTree *self);
	bool (*hasNext)(const GenericTree *self);
	void* (*getNext)(GenericTree *self);
	char (*getIterAction)(const GenericTree *self);
	bool (*equals)(GenericTree *self, GenericTree *other);
	bool (*isEmpty)(const GenericTree *self);
	void (*pushCurrentNode)(GenericTree *self);
	void* (*popCurrentNode)(GenericTree *self);
	void (*clearPushedNodes)(GenericTree *self);
};

struct GenericTreeNode {
	GenericLinkedList *childNodes;
	struct GenericTreeNode *parrentNode;
	void *data;
};

struct GenericTreeIterData {
	struct GenericTreeNode *iterNode;
	struct GenericTreeNode *startNode;
	char previousAction;
};

typedef struct GenericTreeObj {
	struct GenericTreeNode *headNode;
	struct GenericTreeNode *currentNode;
	struct GenericTreeIterData iterData;
	GenericLinkedList *pushQue;
	int numNodes;
	size_t elementSize;
	const struct GenericTreeMethods *methods;
} GenericTree;

extern const struct GenericTreeClass_t GenericTreeClass;

#endif
