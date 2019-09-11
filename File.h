#ifndef FILE_CLASS
#define FILE_CLASS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "String.h"
#include "StringList.h"

#define WINDOWS 0
#define UNIX 1
#define LINUX 2
#define MAC 3
#define OS_UK -1

#if defined(_WIN32) || defined(WIN63)
	#define OS WINDOWS
#elif defined(unix) || defined(__unix) || defined(__unix__)
	#define OS UNIX
#elif defined(__APPLE__) || defined(__MACH__)
	#define OS MAC
#elif defined(__linux__) || defined(linux) || defined(__linux)
	#define OS LINUX
#else
	#define OS OS_UK
#endif

typedef struct FileObj File;

struct FileClass_t {
	char pathSeparator;
	File* (*new)(void);
	void (*delete)(File **obj);
	void (*print)(File *obj);
	bool (*fileExists)(const File *obj);
	String* (*getCwd)(void);
	String* (*getPathFromList)(const StringList *dirs);
	String* (*getPathToApplication)(void);
	StringList* (*getFilesInDir)(const String *dir);
	bool (*debug)(void);
	size_t objSize;
};

struct FileMethods {
	void (*setPath)(File *self, const String *newPath);
	StringList* (*getDirList)(const File *self);
	void (*softChangeExtension)(File *self, const String *newExt);
	String* (*getLineWithToken)(File *self, const String *token);
	bool (*startIter)(File *self);
	bool (*hasNext)(File *self);
	String* (*getNextLine)(File *self);
	String* (*getUntilNextToken)(File *self, const char token);
	void (*endIter)(File *self);
	void (*write)(File *self, const String *data);
	void (*append)(File *self, const String *data);
	int (*getNumLines)(File *self);
	void (*clear)(File *self);
	void (*rename)(File *self, const String *newName);
	void (*changeExtension)(File *self, const String *newExt);
	bool (*remove)(File *self);
	bool (*equals)(File *self, File *other);
};

struct FileObj {
	FILE *filePntr;
	String *absPath;
	String *path;
	String *drive;
	String *name;
	String *extension;
	const struct FileMethods *methods;
};

extern const struct FileClass_t FileClass;

#endif
