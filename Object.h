#ifndef OBJECT_FRAMEWORK
#define OBJECT_FRAMEWORK

#include <stdlib.h>

void* createObject(size_t size);
void deleteObject(void **obj);

#endif
