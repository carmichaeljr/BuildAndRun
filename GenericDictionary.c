#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "GenericDictionary.h"
#include "Object.h"
#include "GenericLinkedList.h"
#include "Print.h"

static GenericDictionary* _GenericDictionaryConstructor(void);
static void _GenericDictionaryPrintObj(GenericDictionary *obj);
static bool _GenericDictionaryDebug(void);
static bool _GenericDictionaryDebug_Add(const int *intVals, const char *charVals);
static bool _GenericDictionaryDebug_Remove(const int *intVals, const char *charVals);
static bool _GenericDictionaryDebug_Set(const int *intVals, const char *charVals);
static bool _GenericDictionaryDebug_Search(const int *intVals, const char *charVals);
static bool _GenericDictionaryDebug_Misc(const int *intVals, const char *charVals);
static void _GenericDictionaryDestructor(GenericDictionary **obj);
static void GenericDictionary_SetElementSize(GenericDictionary *self, const size_t keySize, const size_t valSize);
static void GenericDictionary_Set(GenericDictionary *self, const void *keys, const void *vals, const int numElements);
static void GenericDictionary_SetVal(GenericDictionary *self, const void *key, const void *value);
static void GenericDictionary_Add(GenericDictionary *self, const void *key, const void *value);
static void* GenericDictionary_Get(const GenericDictionary *self, const void *token);
static bool GenericDictionary_KeysContain(const GenericDictionary *self, const void *token);
static bool GenericDictionary_ValuesContain(const GenericDictionary *self, const void *token);
static void GenericDictionary_Remove(GenericDictionary *self, const void *token);
static void GenericDictionary_Clear(GenericDictionary *self);
static bool GenericDictionary_Equals(const GenericDictionary *self, const GenericDictionary *other);
static bool GenericDictionary_IsEmpty(const GenericDictionary *self);
static void GenericDictionary_ConvertFromGLLToDictionary(GenericDictionary *self);

static const struct GenericDictionaryMethods genericDictionaryMethods={
	.setElementSize=&GenericDictionary_SetElementSize,
	.set=&GenericDictionary_Set,
	.setVal=&GenericDictionary_SetVal,
	.add=&GenericDictionary_Add,
	.get=&GenericDictionary_Get,
	.keysContain=&GenericDictionary_KeysContain,
	.valuesContain=&GenericDictionary_ValuesContain,
	.remove=&GenericDictionary_Remove,
	.clear=&GenericDictionary_Clear,
	.equals=&GenericDictionary_Equals,
	.isEmpty=&GenericDictionary_IsEmpty,
};
const struct GenericDictionaryClass_t GenericDictionaryClass={
	.new=&_GenericDictionaryConstructor,
	.delete=&_GenericDictionaryDestructor,
	.print=&_GenericDictionaryPrintObj,
	.debug=&_GenericDictionaryDebug,
	.objSize=sizeof(GenericDictionary)
};

//Object Methods================================================================
//Public Methods
static void GenericDictionary_SetElementSize(GenericDictionary *self, const size_t keySize, const size_t valSize){
	self->keys->methods->setElementSize(self->keys,keySize);
	self->values->methods->setElementSize(self->values,valSize);
}

static void GenericDictionary_Set(GenericDictionary *self, const void *keys, const void *vals, const int numElements){
	self->keys->methods->set(self->keys,keys,numElements);
	self->values->methods->set(self->values,vals,numElements);
	GenericDictionary_ConvertFromGLLToDictionary(self);
}

static void GenericDictionary_SetVal(GenericDictionary *self, const void *key, const void *value){
	if (self->numElements>0 && self->methods->keysContain(self,key)){
		int index=self->keys->methods->getFirstIndexOf(self->keys,key);
		self->values->methods->setAt(self->values,value,1,index);
	}
}

static void GenericDictionary_Add(GenericDictionary *self, const void *key, const void *value){
	if (!self->methods->keysContain(self,key)){
		self->keys->methods->add(self->keys,key,1);
		self->values->methods->add(self->values,value,1);
		GenericDictionary_ConvertFromGLLToDictionary(self);
	}
}

static void* GenericDictionary_Get(const GenericDictionary *self, const void *token){
	int index=self->keys->methods->getFirstIndexOf(self->keys,token);
	return self->values->methods->get(self->values,index);
}

static bool GenericDictionary_KeysContain(const GenericDictionary *self, const void *token){
	return self->keys->methods->contains(self->keys,token);
}

static bool GenericDictionary_ValuesContain(const GenericDictionary *self, const void *token){
	return self->values->methods->contains(self->values,token);
}

static void GenericDictionary_Remove(GenericDictionary *self, const void *token){
	if (self->numElements>0 && self->methods->keysContain(self,token)){
		int index=self->keys->methods->getFirstIndexOf(self->keys,token);
		self->keys->methods->removeAt(self->keys,index);
		self->values->methods->removeAt(self->values,index);
		GenericDictionary_ConvertFromGLLToDictionary(self);
	}
}

static void GenericDictionary_Clear(GenericDictionary *self){
	self->keys->methods->clear(self->keys);
	self->values->methods->clear(self->values);
	GenericDictionary_ConvertFromGLLToDictionary(self);
}

static bool GenericDictionary_Equals(const GenericDictionary *self, const GenericDictionary *other){
	bool rv=true;
	rv&=self->keys->methods->equals(self->keys,other->keys);
	rv&=self->values->methods->equals(self->values,other->values);
	return rv;
}

static bool GenericDictionary_IsEmpty(const GenericDictionary *self){
	return (self->numElements>0)? false: true;
}

//Private Methods
static void GenericDictionary_ConvertFromGLLToDictionary(GenericDictionary *self){
	self->numElements=self->keys->numElements;
}

//Class Methods================================================================
static GenericDictionary* _GenericDictionaryConstructor(void){
	GenericDictionary *rv=(GenericDictionary*)createObject(GenericDictionaryClass.objSize);
	rv->keys=GenericLinkedListClass.new();
	rv->values=GenericLinkedListClass.new();
	rv->numElements=0;
	rv->methods=&genericDictionaryMethods;
	return rv;
}

static void _GenericDictionaryPrintObj(GenericDictionary *obj){
	PrintClass.print("<GenericDictionary Obj[Addr: %p]: NumElements: %d  KeySize: %d  ValSize: %d>\n",
			obj,obj->numElements,obj->keys->elementSize,obj->values->elementSize);
	PrintClass.incrementIndentLevel(1);
	GenericLinkedListClass.print(obj->keys);
	GenericLinkedListClass.print(obj->values);
	PrintClass.incrementIndentLevel(-1);
}

static bool _GenericDictionaryDebug(void){
	bool successfull=true;
	int intVals[]={1,2,3,4,5};
	char charVals[]={'a','b','c','d','e'};
	successfull&=_GenericDictionaryDebug_Add((const int*)intVals,(const char*)charVals);
	successfull&=_GenericDictionaryDebug_Remove((const int*)intVals,(const char*)charVals);
	successfull&=_GenericDictionaryDebug_Set((const int *)intVals,(const char *)charVals);
	successfull&=_GenericDictionaryDebug_Search((const int*)intVals,(const char *)charVals);
	successfull&=_GenericDictionaryDebug_Misc((const int*)intVals,(const char *)charVals);
	PrintClass.objectDebug("GenericDictionary",successfull);
	return successfull;
}

static bool _GenericDictionaryDebug_Add(const int *intVals, const char *charVals){
	bool rv=true;
	GenericDictionary *test=GenericDictionaryClass.new();
	test->methods->setElementSize(test,sizeof(int),sizeof(char));
	for (int i=0; i<5; i++){
		test->methods->add(test,intVals+i,charVals+i);
		test->methods->add(test,intVals+i,charVals+i); //should not add, key already used
	}
	//for (int i=0; i<5; i++){
	//	printf("%d %s %c\n",i,(test->methods->hasKey(test,intVals+i))? "True": "False",
	//				*(char*)test->methods->get(test,intVals+i));
	//}
	(test->numElements!=5)? rv=false: 0;
	(*(char*)test->methods->get(test,intVals)!=*charVals)? rv=false: 0;
	(*(char*)test->methods->get(test,intVals+3)!=*(charVals+3))? rv=false: 0;
	(*(char*)test->methods->get(test,intVals+4)!=*(charVals+4))? rv=false: 0;
	//GenericDictionaryClass.print(test);
	GenericDictionaryClass.delete(&test);
	return rv;
}

static bool _GenericDictionaryDebug_Remove(const int *intVals, const char *charVals){
	bool rv=true;
	GenericDictionary *test=GenericDictionaryClass.new();
	test->methods->setElementSize(test,sizeof(int),sizeof(char));
	for (int i=0; i<5; i++){
		test->methods->add(test,intVals+i,charVals+i);
	}
	test->methods->remove(test,intVals+3);
	(test->numElements!=4)? rv=false: 0;
	(test->methods->get(test,(void*)(intVals+3))!=NULL)? rv=false: 0;
	//GenericDictionaryClass.print(test);
	test->methods->clear(test);
	(test->numElements!=0)? rv=false: 0;
	GenericDictionaryClass.delete(&test);
	return rv;
}

static bool _GenericDictionaryDebug_Set(const int *intVals, const char *charVals){
	bool rv=true;
	GenericDictionary *test=GenericDictionaryClass.new();
	test->methods->setElementSize(test,sizeof(int),sizeof(char));
	test->methods->set(test,intVals,charVals,5);
	//GenericDictionaryClass.print(test);
	(test->numElements!=5)? rv=false: 0;
	(*(char*)test->methods->get(test,intVals)!=*charVals)? rv=false: 0;
	(*(char*)test->methods->get(test,intVals+3)!=*(charVals+3))? rv=false: 0;
	(*(char*)test->methods->get(test,intVals+4)!=*(charVals+4))? rv=false: 0;
	test->methods->setVal(test,intVals,charVals+4);
	(*(char*)test->methods->get(test,intVals)!=*(charVals+4))? rv=false: 0;
	GenericDictionaryClass.delete(&test);
	return rv;
}

static bool _GenericDictionaryDebug_Search(const int *intVals, const char *charVals){
	bool rv=true;
	GenericDictionary *test=GenericDictionaryClass.new();
	test->methods->setElementSize(test,sizeof(int),sizeof(char));
	test->methods->set(test,intVals,charVals,3);
	(!test->methods->keysContain(test,intVals+2))? rv=false: 0;
	(!test->methods->valuesContain(test,charVals+2))? rv=false: 0;
	(test->methods->keysContain(test,intVals+4))? rv=false: 0;
	(test->methods->valuesContain(test,charVals+4))? rv=false: 0;
	GenericDictionaryClass.delete(&test);
	return rv;
}

static bool _GenericDictionaryDebug_Misc(const int *intVals, const char *charVals){
	bool rv=true;
	GenericDictionary *test=GenericDictionaryClass.new();
	GenericDictionary *test2=GenericDictionaryClass.new();
	test->methods->setElementSize(test,sizeof(int),sizeof(char));
	test2->methods->setElementSize(test2,sizeof(int),sizeof(char));
	(!test->methods->isEmpty(test))? rv=false: 0;
	(test->keys->elementSize!=sizeof(int))? rv=false: 0;
	(test->values->elementSize!=sizeof(char))? rv=false: 0;
	test->methods->set(test,intVals,charVals,5);
	test2->methods->set(test2,intVals,charVals,5);
	(test->methods->isEmpty(test))? rv=false: 0;
	(!test->methods->equals(test,test2))? rv=false: 0;
	GenericDictionaryClass.delete(&test);
	GenericDictionaryClass.delete(&test2);
	return rv;
}

static void _GenericDictionaryDestructor(GenericDictionary **obj){
	GenericLinkedListClass.delete(&(*obj)->keys);
	GenericLinkedListClass.delete(&(*obj)->values);
	deleteObject((void**)obj);
}
