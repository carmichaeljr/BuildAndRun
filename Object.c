#include <stdlib.h>
#include "Object.h"

void* createObject(size_t size){
	void *obj=NULL;
	while (obj==NULL){
		obj=malloc(size);
	}
	return obj;
}

//cloneObject - create a refernece count so that resources can be released when last resource is deleted

void deleteObject(void **obj){
	free(*obj);
	*obj=NULL;
}
