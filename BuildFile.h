#ifndef BUILD_FILE_CLASS
#define BUILD_FILE_CLASS

#include <stdlib.h>
#include <stdbool.h>
#include "XMLFile.h"

typedef struct BuildFileObj BuildFile;

struct BuildFileClass_t {
	BuildFile* (*new)(void);
	void (*delete)(BuildFile **obj);
	bool (*buildFileExists)(const String *mainFile);
	void (*print)(BuildFile *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct BuildFileMethods {
	bool (*setSource)(BuildFile *self, const File *mainFile);
	void (*generateBuildFile)(BuildFile *self, const File *mainFile);
	String* (*getCommand)(BuildFile *self, const String *type);
	StringList* (*getLineCountList)(BuildFile *self);
};

struct BuildFileObj {
	XMLFile *xmlFile;
	const struct BuildFileMethods *methods;
};

extern const struct BuildFileClass_t BuildFileClass;

#endif

