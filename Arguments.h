#ifndef ARGUMENTS_CLASS
#define ARGUMENTS_CLASS

#include <stdbool.h>
#include "File.h"
#include "StringList.h"

typedef struct ProgramArgsObj ProgramArgs;

struct ProgramArgsClass_t {
	ProgramArgs* (*new)(void);
	void (*delete)(ProgramArgs **obj);
	void (*print)(ProgramArgs *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct ProgramArgsMethods {
	void (*set)(ProgramArgs *self, const char **argv, const int argc);
	int (*getNumArgs)(const ProgramArgs *self);
	const String* (*get)(const ProgramArgs *self, const int index);
	bool (*areValid)(const ProgramArgs *self);
	bool (*operationRequested)(const ProgramArgs *self, const char operation);
	File* (*getSecondArgAsFile)(const ProgramArgs *self);
};

struct ProgramArgsObj {
	String *progName;
	StringList *args;
	const struct ProgramArgsMethods *methods;
};

extern const struct ProgramArgsClass_t ProgramArgsClass;

#endif
