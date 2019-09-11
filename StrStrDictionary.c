#include <stdio.h>
#include <stdbool.h>
#include "StrStrDictionary.h"
#include "Object.h"
#include "Print.h"
#include "String.h"
#include "StringList.h"
#include "GenericDictionary.h"

static StrStrDictionary* _StrStrDictConstructor(void);
static void _StrStrDictPrintObj(StrStrDictionary *obj);
static bool _StrStrDictDebug(void);
static bool _StrStrDictDebugAdd(const String **keys, const String **vals);
static bool _StrStrDictDebugRemove(const String **keys, const String **vals);
static bool _StrStrDictDebugSet(const String **keys, const String **vals);
static bool _StrStrDictDebugSearch(const String **keys, const String **vals);
static bool _StrStrDictDebugMisc(const String **keys, const String **vals);
static void _StrStrDictDestructor(StrStrDictionary **obj);
static void StrStrDict_Set(StrStrDictionary *self, const String **keys, const String **vals, const int numElements);
static void StrStrDict_SetVal(StrStrDictionary *self, const String *key, const String *val);
static void StrStrDict_Add(StrStrDictionary *self, const String **keys, const String **vals, const int numElements);
static String* StrStrDict_Get(const StrStrDictionary *self, const String *token);
static StringList* StrStrDict_GetKeys(const StrStrDictionary *self);
static bool StrStrDict_KeysContain(const StrStrDictionary *self, const String *token);
static bool StrStrDict_ValuesContain(const StrStrDictionary *self, const String *token);
static void StrStrDict_Remove(StrStrDictionary *self, const String *token);
static void StrStrDict_Clear(StrStrDictionary *self);
static bool StrStrDict_Equals(const StrStrDictionary *self, const StrStrDictionary *other);
static bool StrStrDict_IsEmpty(const StrStrDictionary *self);
static void StrStrDict_ConvertFromGDToStrStrDictionary(StrStrDictionary *self);
static struct StrStrDictKeyValuePair StrStrDict_GetKeyValuePairFromIndex(const StrStrDictionary *self, const int index);

static const struct StrStrDictionaryMethods strStrDictMethods={
	.set=&StrStrDict_Set,
	.setVal=&StrStrDict_SetVal,
	.add=&StrStrDict_Add,
	.get=&StrStrDict_Get,
	.getKeys=&StrStrDict_GetKeys,
	.keysContain=&StrStrDict_KeysContain,
	.valuesContain=&StrStrDict_ValuesContain,
	.remove=&StrStrDict_Remove,
	.clear=&StrStrDict_Clear,
	.equals=&StrStrDict_Equals,
	.isEmpty=&StrStrDict_IsEmpty,
};
const struct StrStrDictionaryClass_t StrStrDictionaryClass={
	.new=&_StrStrDictConstructor,
	.delete=&_StrStrDictDestructor,
	.print=&_StrStrDictPrintObj,
	.debug=&_StrStrDictDebug,
	.objSize=sizeof(StrStrDictionary)
};

//Object Methods===============================================================
//Public methods
static void StrStrDict_Set(StrStrDictionary *self, const String **keys, const String **vals, const int numElements){
	self->methods->clear(self);
	GenericDictionary *temp=self->gd;
	for (int i=0; i<numElements; i++){
		temp->methods->add(temp,*(keys+i),*(vals+i));
	}
	StrStrDict_ConvertFromGDToStrStrDictionary(self);
}

static void StrStrDict_SetVal(StrStrDictionary *self, const String *token, const String *newVal){
	String *iterStr=NULL;
	GenericLinkedList *keys=self->gd->keys;
	GenericLinkedList *vals=self->gd->values;
	for (int i=0; i<keys->numElements; i++){
		iterStr=(String*)keys->methods->get(keys,i);
		if (token->methods->equals(token,iterStr->str)){
			if (self->ownsData){
				String *oldStr=vals->methods->get(vals,i);
				vals->methods->removeAt(vals,i);
				vals->methods->addAt(vals,newVal,1,i);
				StringClass.delete(&oldStr);
			} else {
				vals->methods->setAt(vals,newVal,1,i);
			}
		}
	}
}

static void StrStrDict_Add(StrStrDictionary *self, const String **keys, const String **vals, const int numElements){
	GenericDictionary *temp=self->gd;
	for (int i=0; i<numElements; i++){
		temp->methods->add(temp,*(keys+i),*(vals+i));
	}
	StrStrDict_ConvertFromGDToStrStrDictionary(self);
}

static String* StrStrDict_Get(const StrStrDictionary *self, const String *token){
	String *rv=NULL;
	String *iterStr=NULL;
	GenericLinkedList *keys=self->gd->keys;
	for (int i=0; i<keys->numElements && rv==NULL; i++){
		iterStr=(String*)keys->methods->get(keys,i);
		if (token->methods->equals(token,iterStr->str)){
			rv=(String*)self->gd->values->methods->get(self->gd->values,i);
		}
	}
	return rv;
}

static StringList* StrStrDict_GetKeys(const StrStrDictionary *self){
	String *iterStr=NULL;
	StringList *rv=StringListClass.new();
	rv->ownsData=false;
	GenericLinkedList *keys=self->gd->keys;
	for (int i=0; i<keys->numElements; i++){
		iterStr=(String*)keys->methods->get(keys,i);
		rv->methods->add(rv,(const String**)&iterStr,1);
	}
	return rv;
}

static bool StrStrDict_KeysContain(const StrStrDictionary *self, const String *token){
	bool rv=false;
	String *iterStr=NULL;
	GenericLinkedList *keys=self->gd->keys;
	for (int i=0; i<keys->numElements && !rv; i++){
		iterStr=(String*)keys->methods->get(keys,i);
		if (token->methods->equals(token,iterStr->str)){
			rv=true;
		}
	}
	return rv;
}

static bool StrStrDict_ValuesContain(const StrStrDictionary *self, const String *token){
	bool rv=false;
	String *iterStr=NULL;
	GenericLinkedList *values=self->gd->values;
	for (int i=0; i<values->numElements && !rv; i++){
		iterStr=(String*)values->methods->get(values,i);
		if (token->methods->equals(token,iterStr->str)){
			rv=true;
		}
	}
	return rv;
}

static void StrStrDict_Remove(StrStrDictionary *self, const String *token){
	bool _break=false;
	GenericLinkedList *keys=self->gd->keys;
	GenericLinkedList *vals=self->gd->values;
	struct StrStrDictKeyValuePair iterPair;
	for (int i=0; i<keys->numElements && !_break; i++){
		iterPair=StrStrDict_GetKeyValuePairFromIndex(self,i);
		if (token->methods->equals(token,iterPair.key->str)){
			if (self->ownsData){
				StringClass.delete(&iterPair.key);
				StringClass.delete(&iterPair.value);
			}
			keys->methods->removeAt(keys,i);
			vals->methods->removeAt(vals,i);
			self->gd->numElements--;
			_break=true;	//Can break because there is only one entry in the dictionary
		}
	}
	StrStrDict_ConvertFromGDToStrStrDictionary(self);
}

static void StrStrDict_Clear(StrStrDictionary *self){
	struct StrStrDictKeyValuePair iterPair;
	for (int i=0; i<self->size && self->ownsData; i++){
		iterPair=StrStrDict_GetKeyValuePairFromIndex(self,i);
		StringClass.delete(&iterPair.key);
		StringClass.delete(&iterPair.value);
	}
	self->gd->methods->clear(self->gd);
	StrStrDict_ConvertFromGDToStrStrDictionary(self);
}

static bool StrStrDict_Equals(const StrStrDictionary *self, const StrStrDictionary *other){
	bool rv=(self->size==other->size)? true: false;
	struct StrStrDictKeyValuePair iterPair[2];
	if (rv){
		for (int i=0; i<self->size && rv; i++){
			iterPair[0]=StrStrDict_GetKeyValuePairFromIndex(self,i);
			iterPair[1].key=iterPair[0].key;
			iterPair[1].value=other->methods->get(other,iterPair[1].key);
			if (iterPair[1].value==NULL ||
			    !iterPair[0].value->methods->equals(iterPair[0].value,iterPair[1].value->str)){
				rv=false;
			}
		}
	}
	return rv;
}

static bool StrStrDict_IsEmpty(const StrStrDictionary *self){
	return (self->size>0)? false: true;
}

//Private methods
static void StrStrDict_ConvertFromGDToStrStrDictionary(StrStrDictionary *self){
	self->size=self->gd->numElements;
}

static struct StrStrDictKeyValuePair StrStrDict_GetKeyValuePairFromIndex(const StrStrDictionary *self, const int index){
	struct StrStrDictKeyValuePair rv;
	if (index<self->size){
		rv.key=(String*)self->gd->keys->methods->get(self->gd->keys,index);
		rv.value=(String*)self->gd->values->methods->get(self->gd->values,index);
	}
	return rv;
}

//Class Methods================================================================
static StrStrDictionary* _StrStrDictConstructor(void){
	StrStrDictionary *rv=(StrStrDictionary*)createObject(StrStrDictionaryClass.objSize);
	rv->size=0;
	rv->ownsData=false;
	rv->gd=GenericDictionaryClass.new();
	rv->gd->methods->setElementSize(rv->gd,sizeof(String),sizeof(String));
	rv->methods=&strStrDictMethods;
	return rv;
}

static void _StrStrDictPrintObj(StrStrDictionary *obj){
	PrintClass.print("<StrStrDictionary Obj[Addr: %p]: Size: %d OwnsData: %s>\n",
			obj,obj->size,(obj->ownsData)? "True": "False");
	PrintClass.incrementIndentLevel(1);
	GenericLinkedListClass.print(obj->gd->keys);
	GenericLinkedListClass.print(obj->gd->values);
	PrintClass.incrementIndentLevel(-1);
}

static bool  _StrStrDictDebug(void){
	bool successfull=true;
	String **testKeys=malloc(sizeof(String*)*3);
	String **testVals=malloc(sizeof(String*)*3);
	for (int i=0; i<3; i++){
		testKeys[i]=StringClass.new();
		testVals[i]=StringClass.new();
	}
	testKeys[0]->methods->set(testKeys[0],"Key1");
	testKeys[1]->methods->set(testKeys[1],"Key2");
	testKeys[2]->methods->set(testKeys[2],"Key3");
	testVals[0]->methods->set(testVals[0],"Val1");
	testVals[1]->methods->set(testVals[1],"Val2");
	testVals[2]->methods->set(testVals[2],"Val3");

	successfull&=_StrStrDictDebugAdd((const String**)testKeys,(const String**)testVals);
	successfull&=_StrStrDictDebugRemove((const String**)testKeys,(const String **)testVals);
	successfull&=_StrStrDictDebugSet((const String**)testKeys,(const String**)testVals);
	successfull&=_StrStrDictDebugSearch((const String**)testKeys,(const String**)testVals);
	successfull&=_StrStrDictDebugMisc((const String**)testKeys,(const String**)testVals);

	for (int i=0; i<2; i++){
		StringClass.delete(&testKeys[i]);
		StringClass.delete(&testVals[i]);
	}
	free(testKeys);
	free(testVals);
	PrintClass.objectDebug("StrStrDictionary",successfull);
	return successfull;
}

static bool _StrStrDictDebugAdd(const String **keys, const String **vals){
	bool successfull=true;
	StrStrDictionary *test=StrStrDictionaryClass.new();
	test->methods->add(test,keys,vals,2);
	for (int i=0; i<2; i++){
		(test->methods->get(test,keys[i])!=vals[i])? successfull=false: 0;
	}
	//StrStrDictionaryClass.print(test);
	StrStrDictionaryClass.delete(&test);
	return successfull;
}

static bool _StrStrDictDebugRemove(const String **keys, const String **vals){
	bool successfull=true;
	StrStrDictionary *test=StrStrDictionaryClass.new();
	test->methods->set(test,keys,vals,3);
	test->methods->remove(test,*(keys+1));
	(test->size!=2)? successfull=false: 0;
	(test->methods->get(test,keys[0])!=vals[0])? successfull=false: 0;
	(test->methods->get(test,keys[2])!=vals[2])? successfull=false: 0;
	test->methods->clear(test);
	(test->size!=0)? successfull=false: 0;
	//StrStrDictionaryClass.print(test);
	StrStrDictionaryClass.delete(&test);
	return successfull;
}

static bool _StrStrDictDebugSet(const String **keys, const String **vals){
	bool successfull=true;
	StrStrDictionary *test=StrStrDictionaryClass.new();
	test->methods->set(test,keys,vals,2);
	for (int i=0; i<2; i++){
		(test->methods->get(test,keys[i])!=vals[i])? successfull=false: 0;
	}
	test->methods->setVal(test,*keys,*(vals+2));
	(test->methods->get(test,keys[0])!=vals[2])? successfull=false: 0;
	//StrStrDictionaryClass.print(test);
	StrStrDictionaryClass.delete(&test);
	return successfull;
}

static bool _StrStrDictDebugSearch(const String **keys, const String **vals){
	bool successfull=true;
	StrStrDictionary *test=StrStrDictionaryClass.new();
	StrStrDictionary *test2=StrStrDictionaryClass.new();
	test->methods->set(test,keys,vals,2);
	test2->methods->set(test2,keys,vals,3);
	StringList *tempKeys=test2->methods->getKeys(test2);
	(!test->methods->keysContain(test,*keys))? successfull=false: 0;
	(!test->methods->keysContain(test,*(keys+1)))? successfull=false: 0;
	(test->methods->keysContain(test,*(keys+2)))? successfull=false: 0;
	(!test->methods->valuesContain(test,*vals))? successfull=false: 0;
	(!test->methods->valuesContain(test,*(vals+1)))? successfull=false: 0;
	(test->methods->valuesContain(test,*(vals+2)))? successfull=false: 0;
	(test->methods->equals(test,test2))? successfull=false: 0;
	test2->methods->remove(test2,*(keys+2));
	(!test->methods->equals(test,test2))? successfull=false: 0;
	(tempKeys->length!=3)? successfull=false: 0;
	for (int i=0; i<3; i++){
		(tempKeys->methods->getFirstIndexOf(tempKeys,keys[i])<0)? successfull=false: 0;
	}
	//StrStrDictionaryClass.print(test);
	//StrStrDictionaryClass.print(test2);
	StringListClass.delete(&tempKeys);
	StrStrDictionaryClass.delete(&test);
	StrStrDictionaryClass.delete(&test2);
	return successfull;
}

static bool _StrStrDictDebugMisc(const String **keys, const String **vals){
	bool successfull=true;
	StrStrDictionary *test=StrStrDictionaryClass.new();
	(!test->methods->isEmpty(test))? successfull=false: 0;
	test->methods->set(test,keys,vals,3);
	(test->methods->isEmpty(test))? successfull=false: 0;
	//StrStrDictionaryClass.print(test);
	StrStrDictionaryClass.delete(&test);
	return successfull;
}

static void _StrStrDictDestructor(StrStrDictionary **obj){
	(*obj)->methods->clear(*obj);
	GenericDictionaryClass.delete(&(*obj)->gd);
	deleteObject((void**)obj);
}
