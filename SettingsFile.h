#ifndef SETTINGS_FILE_CLASS
#define SETTINGS_FILE_CLASS

#include <stdlib.h>
#include <stdbool.h>
#include "XMLFile.h"

typedef struct SettingsFileObj SettingsFile;

struct SettingsFileClass_t {
	SettingsFile* (*new)(void);
	void (*delete)(SettingsFile **obj);
	void (*print)(SettingsFile *obj);
	bool (*debug)(void);
	size_t objSize;
};

struct SettingsFileMethods {
	StringList* (*getAcceptedFirstArgs)(const SettingsFile *self);
	bool (*langDefExists)(const SettingsFile *self, const String *ext);
	XMLFile* (*getBuildFileInfo)(const SettingsFile *self, const String *ext);
	const String* (*getOutputPath)(const SettingsFile *self, const String *ext);
	void (*openForUser)(const SettingsFile *self);
};

struct SettingsFileObj {
	XMLFile *xmlFile;
	const struct SettingsFileMethods *methods;
};

extern const struct SettingsFileClass_t SettingsFileClass;

#endif
