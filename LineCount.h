#ifndef LINE_COUNT_CLASS
#define LINE_COUNT_CLASS

#include <stdlib.h>
#include <stdbool.h>
#include "StringList.h"
#include "BuildFile.h"

typedef struct LineCountObj LineCount;

struct LineCountClass_t {
	LineCount* (*new)(void);
	void (*delete)(LineCount **obj);
	void (*print)(LineCount *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct LineCountMethods {
	void (*setLineCountList)(LineCount *self, BuildFile *buildFile);
	void (*runLineCount)(LineCount *self);
};

struct LineCountObj {
	int numLines;
	StringList *allFiles;
	const struct LineCountMethods *methods;
};

extern const struct LineCountClass_t LineCountClass;

#endif

