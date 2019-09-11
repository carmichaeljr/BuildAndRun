#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "StringList.h"
#include "Object.h"
#include "Print.h"
#include "String.h"
#include "GenericLinkedList.h"

static StringList* _StringListConstructor(void);
static void _StringListPrintObj(StringList *obj);
static bool _StringListDebug(void);
static bool _StringListDebug_Add(const String **strings);
static bool _StringListDebug_Remove(const String **strings);
static bool _StringListDebug_Set(const String **strings);
static bool _StringListDebug_Search(const String **strings);
static bool _StringListDebug_Misc(const String **strings);
static void _StringListDestructor(StringList **obj);
static void StringList_Set(StringList *self, const String **strs, const int numElements);
static void StringList_SetAt(StringList *self, const String **strs, const int numElements, const int index);
static void StringList_Add(StringList *self, const String **strs, const int numElements);
static void StringList_AddAt(StringList *self, const String **strs, const int numElements, const int index);
static void StringList_CopyOtherBetween(StringList *self, const StringList *other, const int startIndex, const int endIndex);
static void StringList_ListFromDelimiter(StringList *self, const char *str, const char *delim);
static String* StringList_Get(const StringList *self, const int index);
static bool StringList_Contains(const StringList *self, const String *str);
static int StringList_GetFirstIndexOf(const StringList *self, const String *str);
static int StringList_GetLastIndexOf(const StringList *self, const String *str);
static void StringList_Remove(StringList *self, const String *str);
static void StringList_RemoveAt(StringList *self, const int index);
static void StringList_RemoveBetween(StringList *self, const int startIndex, const int endIndex);
static void StringList_TrimToSize(StringList *self, const int numElements);
static void StringList_Clear(StringList *self);
static bool StringList_Equals(const StringList *self, const StringList *other);
static bool StringList_IsEmpty(const StringList *self);
static void StringList_ConvertFromGLLToStringList(StringList *self);

static const struct StringListMethods stringListMethods={
	.set=&StringList_Set,
	.setAt=&StringList_SetAt,
	.add=&StringList_Add,
	.addAt=&StringList_AddAt,
	.copyOtherBetween=&StringList_CopyOtherBetween,
	.listFromDelimiter=&StringList_ListFromDelimiter,
	.get=&StringList_Get,
	.contains=&StringList_Contains,
	.getFirstIndexOf=&StringList_GetFirstIndexOf,
	.getLastIndexOf=&StringList_GetLastIndexOf,
	.remove=&StringList_Remove,
	.removeAt=&StringList_RemoveAt,
	.removeBetween=&StringList_RemoveBetween,
	.trimToSize=&StringList_TrimToSize,
	.clear=&StringList_Clear,
	.equals=&StringList_Equals,
	.isEmpty=&StringList_IsEmpty,
};
const struct StringListClass_t StringListClass={
	.new=&_StringListConstructor,
	.delete=&_StringListDestructor,
	.print=&_StringListPrintObj,
	.debug=&_StringListDebug,
	.objSize=sizeof(StringList)
};

//Object Methods================================================================
//Public Methods
static void StringList_Set(StringList *self, const String **strs, const int numElements){
	for (int i=0; i<numElements; i++){
		self->gll->methods->setAt(self->gll,*(strs+i),1,i);
	}
	StringList_ConvertFromGLLToStringList(self);
}

static void StringList_SetAt(StringList *self, const String **strs, const int numElements, const int index){
	GenericLinkedList *gll=self->gll;
	for (int i=0; i<numElements; i++){
		gll->methods->setAt(gll,*(strs+i),1,index+i);
	}
	StringList_ConvertFromGLLToStringList(self);
}

static void StringList_Add(StringList *self, const String **strs, const int numElements){
	GenericLinkedList *gll=self->gll;
	for (int i=0; i<numElements; i++){
		gll->methods->add(gll,*(strs+i),1);
	}
	StringList_ConvertFromGLLToStringList(self);
}

static void StringList_AddAt(StringList *self, const String **strs, const int numElements, const int index){
	GenericLinkedList *gll=self->gll;
	for (int i=0; i<numElements; i++){
		gll->methods->addAt(gll,*(strs+i),1,index+i);
	}
	StringList_ConvertFromGLLToStringList(self);
}

static void StringList_CopyOtherBetween(StringList *self, const StringList *other, const int startIndex, const int endIndex){
	if (self->ownsData){
		self->methods->clear(self);
	}
	self->gll->methods->copyOtherBetween(self->gll,other->gll,startIndex,endIndex);
	StringList_ConvertFromGLLToStringList(self);
}

static void StringList_ListFromDelimiter(StringList *self, const char *str, const char *delim){
	String *origStr=StringClass.new();
	origStr->methods->set(origStr,str);
	char *section=strtok(origStr->str,delim);
	while (section!=NULL){
		String *temp=StringClass.new();
		temp->methods->set(temp,section);
		self->methods->add(self,(const String**)&temp,1);
		section=strtok(NULL,delim);
	}
	StringClass.delete(&origStr);
}

static String* StringList_Get(const StringList *self, const int index){
	String *rv=NULL;
	if (index>=0 && index<self->length){
		//gll returns a void pointer to the string struct
		rv=(String*)self->gll->methods->get(self->gll,index);
	}
	return rv;
}

static bool StringList_Contains(const StringList *self, const String *str){
	return (self->methods->getFirstIndexOf(self,str)!=-1)? true: false;
}

static int StringList_GetFirstIndexOf(const StringList *self, const String *str){
	int rv=-1;
	String *temp=NULL;
	for (int i=0; i<self->length && rv==-1; i++){
		temp=self->methods->get(self,i);
		if (temp->methods->equals(temp,str->str)){
			rv=i;
		}
	}
	return rv;
}

static int StringList_GetLastIndexOf(const StringList *self, const String *str){
	int rv=-1;
	String *temp=NULL;
	for (int i=self->length-1; i>=0 && rv==-1; i--){
		temp=self->methods->get(self,i);
		if (temp->methods->equals(temp,str->str)){
			rv=i;
		}
	}
	return rv;
}

static void StringList_Remove(StringList *self, const String *str){
	for (int i=0; i<self->length; i++){
		String *temp=self->methods->get(self,i);
		if (temp->methods->equals(temp,str->str)){
			self->gll->methods->remove(self->gll,temp);
			if (self->ownsData){
				StringClass.delete(&temp);
			}
			StringList_ConvertFromGLLToStringList(self);
			i--;
		}
	}
}

static void StringList_RemoveAt(StringList *self, const int index){
	if (self->ownsData){
		String *temp=self->methods->get(self,index);
		StringClass.delete(&temp);
	}
	self->gll->methods->removeAt(self->gll,index);
	StringList_ConvertFromGLLToStringList(self);
}

static void StringList_RemoveBetween(StringList *self, const int startIndex, const int endIndex){
	if (self->ownsData && startIndex>=0 && endIndex<=self->length &&
		startIndex<endIndex){
		for (int i=startIndex; i<endIndex; i++){
			String *temp=self->methods->get(self,i);
			StringClass.delete(&temp);
		}
	}
	self->gll->methods->removeBetween(self->gll,startIndex,endIndex);
	StringList_ConvertFromGLLToStringList(self);
}

static void StringList_TrimToSize(StringList *self, const int numElements){
	self->gll->methods->removeBetween(self->gll,numElements,self->length);
	StringList_ConvertFromGLLToStringList(self);
}

static void StringList_Clear(StringList *self){
	for (int i=0; i<self->gll->numElements && self->ownsData; i++){
		String *temp=self->methods->get(self,i);
		StringClass.delete(&temp);
	}
	self->gll->methods->clear(self->gll);
	StringList_ConvertFromGLLToStringList(self);
}

static bool StringList_Equals(const StringList *self, const StringList *other){
	bool rv=(self->length==other->length)? true: false;
	String *temp=NULL;
	String *temp2=NULL;
	if (rv){
		for (int i=0; i<self->length && rv; i++){
			temp=self->methods->get(self,i);
			temp2=other->methods->get(other,i);
			if (!temp->methods->equals(temp,temp2->str)){
				rv=false;
			}
		}
	}
	return rv;
}

static bool StringList_IsEmpty(const StringList *self){
	return (self->length==0)? true: false;
}
//
//Private Methods
static void StringList_ConvertFromGLLToStringList(StringList *self){
	self->length=self->gll->numElements;
}

//Class Methods=================================================================
static StringList* _StringListConstructor(void){
	StringList *rv=(StringList*)createObject(StringListClass.objSize);
	rv->length=0;
	rv->ownsData=false;
	rv->gll=GenericLinkedListClass.new();
	rv->gll->methods->setElementSize(rv->gll,sizeof(String));
	rv->methods=&stringListMethods;
	return rv;
}

static void _StringListPrintObj(StringList *obj){
	PrintClass.print("<StringList Obj[Addr: %p]: Length: %d>\n",obj,obj->length);
	PrintClass.incrementIndentLevel(1);
	for(int i=0; i<obj->length; i++){
		//print(". [%d]",i);
		StringClass.print(obj->methods->get(obj,i));
	}
	PrintClass.incrementIndentLevel(-1);
}

static bool _StringListDebug(void){
	bool successfull=true;
	String **strings=(String**)malloc(4*sizeof(String*));
	for (int i=0; i<4; i++){
		strings[i]=StringClass.new();
	}
	strings[0]->methods->set(strings[0],"String1");
	strings[1]->methods->set(strings[1],"String2");
	strings[2]->methods->set(strings[2],"String3");
	strings[3]->methods->set(strings[3],"String4");
	successfull&=_StringListDebug_Add((const String**)strings);
	successfull&=_StringListDebug_Remove((const String**)strings);
	successfull&=_StringListDebug_Set((const String**)strings);
	successfull&=_StringListDebug_Search((const String**)strings);
	successfull&=_StringListDebug_Misc((const String**)strings);
	
	//for (int i=0; i<4; i++){
	//	StringClass.print(*(strings+i));
	//	StringClass.delete(&*(strings+i));
	//}
	//printf("here2\n");
	for (int i=0; i<4; i++){
		StringClass.delete((strings+i));
	}
	//printf("here3\n");
	free(strings);
	PrintClass.objectDebug("StringList",successfull);
	return successfull;
}

static bool _StringListDebug_Add(const String **strings){
	bool rv=true;
	StringList *test=StringListClass.new();
	test->methods->addAt(test,strings,4,0);
	test->methods->add(test,strings,1);
	test->methods->addAt(test,strings+1,2,2);
	String *temp=test->methods->get(test,0);
	String *temp2=test->methods->get(test,3);
	String *temp3=test->methods->get(test,5);
	String *temp4=test->methods->get(test,6);
	(test->length!=7)? rv=false: 0;
	(!temp->methods->equals(temp,strings[0]->str))? rv=false: 0;
	(!temp2->methods->equals(temp2,strings[2]->str))? rv=false: 0;
	(!temp3->methods->equals(temp3,strings[3]->str))? rv=false: 0;
	(!temp4->methods->equals(temp4,strings[0]->str))? rv=false: 0;
	//StringListClass.print(test);
	StringListClass.delete(&test);
	return rv;
}

static bool _StringListDebug_Remove(const String **strings){
	bool rv=true;
	StringList *test=StringListClass.new();
	StringList *test2=StringListClass.new();
	for (int i=0; i<4; i++){
		test->methods->add(test,strings+i,1);
		test->methods->add(test,strings+i,1);
		test2->methods->add(test2,strings+i,1);
		test2->methods->add(test2,strings+i,1);
	}
	test->methods->removeAt(test,1);
	test->methods->removeAt(test,2);
	test->methods->removeAt(test,3);
	test->methods->removeAt(test,4);
	String *temp=test->methods->get(test,0);
	String *temp2=test->methods->get(test,3);
	(test->length!=4)? rv=false: 0;
	(!temp->methods->equals(temp,strings[0]->str))? rv=false: 0;
	(!temp2->methods->equals(temp2,strings[3]->str))? rv=false: 0;
	test->methods->removeBetween(test,0,2);
	temp=test->methods->get(test,0);
	temp2=test->methods->get(test,1);
	(test->length!=2)? rv=false: 0;
	(!temp->methods->equals(temp,strings[2]->str))? rv=false: 0;
	(!temp2->methods->equals(temp2,strings[3]->str))? rv=false: 0;
	test->methods->trimToSize(test,1);
	temp=test->methods->get(test,0);
	(test->length!=1)? rv=false: 0;
	(!temp->methods->equals(temp,strings[2]->str))? rv=false: 0;
	test2->methods->remove(test2,*(strings+2));
	test2->methods->remove(test2,*(strings));
	test2->methods->remove(test2,*(strings+3));
	(test2->length!=2)? rv=false: 0;
	temp=test2->methods->get(test2,0);
	(!temp->methods->equals(temp,strings[1]->str))? rv=false: 0;
	StringListClass.delete(&test);
	StringListClass.delete(&test2);
	return rv;
}

static bool _StringListDebug_Set(const String **strings){
	bool rv=true;
	StringList *test=StringListClass.new();
	test->methods->add(test,strings,4);
	test->methods->add(test,strings,4);
	test->methods->setAt(test,strings,4,1); 
	String *temp=test->methods->get(test,1);
	String *temp2=test->methods->get(test,4);
	String *temp3=test->methods->get(test,0);
	(test->length!=8)? rv=false: 0;
	(!temp->methods->equals(temp,strings[0]->str))? rv=false: 0;
	(!temp2->methods->equals(temp2,strings[3]->str))? rv=false: 0;
	(!temp3->methods->equals(temp3,strings[0]->str))? rv=false: 0;
	test->methods->set(test,strings,4);
	temp=test->methods->get(test,1);
	temp2=test->methods->get(test,3);
	(test->length!=8)? rv=false: 0;
	(!temp->methods->equals(temp,strings[1]->str))? rv=false: 0;
	(!temp->methods->equals(temp2,strings[3]->str))? rv=false: 0;
	//StringListClass.print(test);
	StringListClass.delete(&test);
	return rv;
}

static bool _StringListDebug_Search(const String **strings){
	bool rv=true;
	String *tempStr=StringClass.new();
	StringList *test=StringListClass.new();
	StringList *test2=StringListClass.new();
	tempStr->methods->set(tempStr,"String3");
	test->methods->add(test,strings,4);
	test2->methods->add(test2,strings,4);
	(test->methods->isEmpty(test))? rv=false: 0;
	(test->methods->getFirstIndexOf(test,tempStr)!=2)? rv=false: 0;
	(test->methods->getLastIndexOf(test,tempStr)!=2)? rv=false: 0;
	(!test->methods->contains(test,tempStr))? rv=false: 0;
	(!test->methods->equals(test,test2))? rv=false: 0;
	StringListClass.delete(&test);
	StringListClass.delete(&test2);
	StringClass.delete(&tempStr);
	return rv;
}

static bool _StringListDebug_Misc(const String **strings){
	bool rv=true;
	String *delimList=StringClass.new();
	delimList->methods->set(delimList,"A,B,C,D,E,F,");
	StringList *test=StringListClass.new();
	StringList *test2=StringListClass.new();
	StringList *test3=StringListClass.new();
	test->methods->add(test,strings+3,1);
	test2->methods->add(test2,strings,4);
	test->methods->copyOtherBetween(test,test2,1,4);
	(test->length!=3)? rv=false: 0;
	test3->methods->listFromDelimiter(test3,delimList->str,",");
	(test3->length!=6)? rv=false: 0;
	//StringListClass.print(test3);
	//StringListClass.print(test);
	StringListClass.delete(&test);
	StringListClass.delete(&test2);
	StringListClass.delete(&test3);
	StringClass.delete(&delimList);
	return rv;
}

static void _StringListDestructor(StringList **obj){
	(*obj)->methods->clear(*obj);
	GenericLinkedListClass.delete(&(*obj)->gll);
	deleteObject((void**)obj);
}
