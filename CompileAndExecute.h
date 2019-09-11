#ifndef COMPILE_AND_EXECUTE_CLASS
#define COMPILE_AND_EXECUTE_CLASS

#include <stdlib.h>
#include <stdbool.h>
#include "Arguments.h"
#include "BuildFile.h"

typedef struct CompileAndExecuteObj CompileAndExecute;

struct CompileAndExecuteClass_t {
	CompileAndExecute* (*new)(void);
	void (*delete)(CompileAndExecute **obj);
	void (*print)(CompileAndExecute *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct CompileAndExecuteMethods {
	void (*setCommands)(CompileAndExecute *self, BuildFile *buildFile, const ProgramArgs *args);
	void (*runCommands)(CompileAndExecute *self);
};

struct CompileAndExecuteObj {
	int retVal;
	String *compileCmd;
	String *executeCmd;
	const struct CompileAndExecuteMethods *methods;
};

extern const struct CompileAndExecuteClass_t CompileAndExecuteClass;

#endif

