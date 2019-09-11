#ifndef PRINT_CLASS
#define PRINT_CLASS

#include <stdbool.h>

struct PrintClass_t {
	void (*incrementIndentLevel)(const int increment);
	void (*setSpacesPerIndent)(const int numSpaces);
	void (*objectDebug)(const char *className, const bool success);
	void (*print)(const char *format, ...);
	void (*printWithoutIndent)(const char *format, ...);
	bool (*yesOrNoPrompt)(const char *question);
	void (*printError)(const char *format, ...);
};

extern const struct PrintClass_t PrintClass;

#endif
