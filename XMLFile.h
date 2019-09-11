#ifndef XML_FILE
#define XML_FILE

#include <stdbool.h>
#include "File.h"
#include "GenericTree.h"
#include "StringList.h"
#include "XMLTag.h"

typedef struct XMLFileObj XMLFile;

struct XMLFileClass_t {
	XMLFile* (*new)(void);
	void (*delete)(XMLFile **obj);
	void (*print)(XMLFile *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct XMLFileMethods {
	void (*setSource)(XMLFile *self, const String *path);
	void (*write)(XMLFile *self);
	bool (*placeCurrentNode)(XMLFile *self, const StringList *path);
	bool (*moveDown)(XMLFile *self);
	bool (*moveNext)(XMLFile *self);
	bool (*moveUp)(XMLFile *self);
	bool (*startIter)(XMLFile *self);
	bool (*startIterAtCurNode)(XMLFile *self);
	bool (*hasNext)(const XMLFile *self);
	XMLTag* (*getNext)(XMLFile *self);
	char (*getIterAction)(const XMLFile *self);
	void (*add)(XMLFile *self, const XMLTag *tag);
	void (*addAt)(XMLFile *self, const StringList *path, const XMLTag *tag);
	void (*set)(XMLFile *self, const XMLTag *newTag);
	void (*setAt)(XMLFile *self, const StringList *path, const XMLTag *tag);
	XMLTag* (*get)(const XMLFile *self);
	XMLTag* (*getAt)(XMLFile *self, const StringList *path);
	int (*getNumChildTags)(const XMLFile *self);
	int (*getNumChildTagsAt)(XMLFile *self, const StringList *path);
	bool (*branchExists)(XMLFile *self, const StringList *path);
	void (*copyOtherBelowCurNode)(XMLFile *self, XMLFile *other);
	void (*remove)(XMLFile *self);
	void (*clear)(XMLFile *self);
	bool (*equals)(XMLFile *self, XMLFile *other);
	bool (*isEmpty)(const XMLFile *self);
};

typedef struct XMLFileObj {
	bool ownsData;
	int numTags;
	File *source;
	GenericTree *gt;
	const struct XMLFileMethods *methods;
} XMLFile;

extern const struct XMLFileClass_t XMLFileClass;

#endif
