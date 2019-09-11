#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "String.h"
#include "Object.h"
#include "Print.h"
#include "GenericList.h"
#include "Math.h"
#include "IntList.h"


static String* _StringConstructor(void);
static void _StringPrintObj(const String *obj);
static bool _StringDebug(void);
static bool _StringDebugAdd(void);
static bool _StringDebugSet(void);
static bool _StringDebugRemove(void);
static bool _StringDebugSearch(void);
static bool _StringDebugMisc(void);
static void _StringDestructor(String **obj);
static void String_Print(const String *self);
static void String_SetLength(String *self);
static void String_ToUpperCase(String *self);
static void String_ToLowerCase(String *self);
static void String_TrimWhitespace(String *self);
static void String_SetSize(String *self, const int len);
static void String_Set(String *self, const char *newStr);
static bool String_Equals(const String *self, const char *other);
static void String_Concat(String *self, const char *newStr);
static bool String_Contains(const String *self, const char *token);
static void String_ConcatChar(String *self, const char newChar);
static void String_ConvertFromGenericListToStr(String *self);
static IntList* String_GetIndexesOf(const String *self, const char token);
static int String_GetLastIndexOf(const String *self, const char searchChar);
static int String_GetFirstIndexOf(const String *self, const char searchChar);
static void String_RemoveChars(String *self, const char *unwantedChars);
static int String_GetCharOccurences(const String *self, const char *searchChars);
static void String_TrimSubstring(String *self, const int startIndex, const int endIndex);
static void String_CopyStrBetween(String *self, const String *other, int startIndex, int endIndex);

static const struct StringMethods stringMethods={
	.print=&String_Print,
	.equals=&String_Equals,
	.set=&String_Set,
	.concat=&String_Concat,
	.concatChar=&String_ConcatChar,
	.setSize=&String_SetSize,
	.getFirstIndexOf=&String_GetFirstIndexOf,
	.getLastIndexOf=&String_GetLastIndexOf,
	.copyStrBetween=&String_CopyStrBetween,
	.removeChars=&String_RemoveChars,
	.trimSubString=&String_TrimSubstring,
	.trimWhitespace=&String_TrimWhitespace,
	.toUpperCase=&String_ToUpperCase,
	.toLowerCase=&String_ToLowerCase,
	.contains=&String_Contains,
	.getIndexesOf=&String_GetIndexesOf,
	.getCharOccurences=&String_GetCharOccurences,
};
const struct StringClass_t StringClass={
	.new=&_StringConstructor,
	.delete=&_StringDestructor,
	.print=&_StringPrintObj,
	.debug=&_StringDebug,
	.objSize=sizeof(String)
};

//Object Methods================================================================
//Public Methods
static void String_Set(String *self, const char *newStr){
	self->genericList->methods->set(self->genericList,newStr,strlen(newStr)+1);
	String_ConvertFromGenericListToStr(self);
}

static void String_Concat(String *self, const char *newStr){
	GenericList *temp=self->genericList;
	if (newStr!=NULL){
		temp->methods->removeAt(temp,temp->numElements-1);
		temp->methods->add(temp,newStr,strlen(newStr)+1);
		String_ConvertFromGenericListToStr(self);
	}
}

static void String_ConcatChar(String *self, const char newChar){
	GenericList *temp=self->genericList;
	temp->methods->addAt(temp,&newChar,1,temp->numElements-1);
	String_ConvertFromGenericListToStr(self);
}

static void String_SetSize(String *self, const int len){
	char *blank=" ";
	char *newLine="\n";
	GenericList *temp=self->genericList;
	temp->methods->setListSize(self->genericList,len);
	for (int i=0; i<temp->numElements-1; i++){
		temp->methods->setAt(temp,blank,1,i);
	}
	temp->methods->setAt(temp,newLine,1,temp->numElements-1);
	String_ConvertFromGenericListToStr(self);
}

static int String_GetFirstIndexOf(const String *self, const char searchChar){
	GenericList *temp=self->genericList;
	return temp->methods->getFirstIndexOf(temp,&searchChar);
}

static int String_GetLastIndexOf(const String *self, const char searchChar){
	GenericList *temp=self->genericList;
	return temp->methods->getLastIndexOf(temp,&searchChar);
}

static int String_GetCharOccurences(const String *self, const char *searchChars){
	//bool _break=false;
	//int curIndex=0;
	int rv=0;
	for (int i=0; i<self->length; i++){
		if (strchr(searchChars,self->str[i])!=NULL){
			rv++;
		}
	}
	//while (curIndex<self->length-1){
	//	_break=false;
	//	for (int i=curIndex+1; i<self->length && !_break; i++){
	//		curIndex=i;
	//		if (strchr(searchChars,self->str[i])!=NULL){
	//			_break=true;
	//			rv++;
	//		}
	//	}
	//}
	return rv;
}

static void String_CopyStrBetween(String *self, const String *other, const int startIndex, const int endIndex){
	GenericList *selfTemp=self->genericList;
	GenericList *otherTemp=other->genericList;
	selfTemp->methods->copyOtherBetween(selfTemp,otherTemp,startIndex,endIndex);
	String_ConvertFromGenericListToStr(self);
}

static void String_RemoveChars(String *self, const char *unwantedChars){
	GenericList *temp=self->genericList;
	for (int i=0; i<strlen(unwantedChars); i++){
		temp->methods->remove(temp,&unwantedChars[i]);
	}
	String_ConvertFromGenericListToStr(self);
}

static void String_TrimSubstring(String *self, const int startIndex, const int endIndex){
	GenericList *temp=self->genericList;
	temp->methods->removeBetween(temp,startIndex,endIndex);
	String_ConvertFromGenericListToStr(self);
}

static void String_TrimWhitespace(String *self){
	bool exit=false;
	if (self->str!=NULL){
		for (int i=0; i<self->length && !exit; i++){
			if (self->str[i]!=' ' && self->str[i]!='\t'){
				self->methods->trimSubString(self,0,i);
				exit=true;
			}
		}
		exit=false;
		for (int i=self->length-1; i>0 && !exit; i--){
			if (self->str[i]!=' ' && self->str[i]!='\t'){
				self->methods->trimSubString(self,i+1,self->length);
				exit=true;
			}
		}
	}
}

static void String_ToUpperCase(String *self){
	if (self->str!=NULL){
		for(int i=0; i<self->length; i++){
			self->str[i]=toupper(self->str[i]);
		}
	}
}

static void String_ToLowerCase(String *self){
	if (self->str!=NULL){
		for(int i=0; i<self->length; i++){
			self->str[i]=tolower(self->str[i]);
		}
	}
}

static IntList* String_GetIndexesOf(const String *self, const char token){
	IntList *rv=IntListClass.new();
	for (int i=0; i<self->length; i++){
		if (self->str[i]==token){
			rv->methods->add(rv,i);
		}
	}
	return rv;
}

static bool String_Contains(const String *self, const char *token){
	return (strstr(self->str,token)!=NULL)? true: false;
}

static bool String_Equals(const String *self, const char *other){
	bool rv=(self->str==NULL && other==NULL)? true: false;
	if (!rv && self->str!=NULL && other!=NULL && strcmp(self->str,other)==0){
		rv=true;
	}
	return rv;
}

static void String_Print(const String *self){
	printf("%s\n",self->str);
}

//Private Methods
static void String_ConvertFromGenericListToStr(String *self){
	char *nullTerminator="\0";
	GenericList *temp=self->genericList;
	bool strExists=(temp->numElements>0)? true: false;
	bool hasNullTerm=(temp->methods->getLastIndexOf(temp,nullTerminator)==temp->numElements-1)? true: false;
	if (strExists){
		if (!hasNullTerm){
			temp->methods->add(temp,nullTerminator,1);
		}
		self->str=(char*)temp->list;
		self->length=(self->genericList->numElements>0)? self->genericList->numElements-1: 0;
		//String_SetLength(self);
	} else {
		self->str=NULL;
		self->length=0;
	}
}

static void String_SetLength(String *self){
	self->length=strlen(self->str);
}

// Class Methods================================================================
static String* _StringConstructor(void){
	String *rv=(String*)createObject(StringClass.objSize);
	rv->genericList=GenericListClass.new();
	rv->genericList->methods->setElementSize(rv->genericList,sizeof(char));
	rv->str=NULL;
	rv->length=0;
	rv->methods=&stringMethods;
	return rv;
}

static void _StringPrintObj(const String *obj){
	PrintClass.print("<String Obj[Addr: %p]: '%s'[%d chars]>\n",obj,obj->str,obj->length);
}

static bool _StringDebug(void){
	bool successfull=true;
	successfull&=_StringDebugAdd();
	successfull&=_StringDebugSet();
	successfull&=_StringDebugRemove();
	successfull&=_StringDebugSearch();
	successfull&=_StringDebugMisc();
	PrintClass.objectDebug("String",successfull);
	return successfull;
}

static bool _StringDebugAdd(void){
	bool successfull=true;
	String *test=StringClass.new();
	test->methods->concat(test,"Testing");
	(test->length!=7)? successfull=false: 0;
	(strcmp(test->str,"Testing")!=0)? successfull=false: 0;
	test->methods->concatChar(test,'3');
	(strcmp(test->str,"Testing3")!=0)? successfull=false: 0;
	(test->length!=8)? successfull=false: 0;
	//StringClass.print(test);
	StringClass.delete(&test);
	return successfull;
}

static bool _StringDebugSet(void){
	bool successfull=true;
	String *test=StringClass.new();
	test->methods->concat(test,"Testing");
	test->methods->set(test,"TESTING!");
	(test->length!=8)? successfull=false: 0;
	(strcmp(test->str,"TESTING!")!=0)? successfull=false: 0;
	StringClass.delete(&test);
	return successfull;
}

static bool _StringDebugRemove(void){
	bool successfull=true;
	String *test=StringClass.new();
	String *test2=StringClass.new();
	test2->methods->set(test2,"NewStr");
	test->methods->set(test,"    abcdefghijklmnopqrstuvwxyz     ");
	test->methods->trimWhitespace(test);
	(test->length!=26)? successfull=false: 0;
	(strcmp(test->str,"abcdefghijklmnopqrstuvwxyz")!=0)? successfull=false: 0;
	test->methods->removeChars(test,"abc");
	(test->length!=23)? successfull=false: 0;
	(strcmp(test->str,"defghijklmnopqrstuvwxyz")!=0)? successfull=false: 0;
	test->methods->trimSubString(test,6,9);
	(test->length!=20)? successfull=false: 0;
	(strcmp(test->str,"defghimnopqrstuvwxyz")!=0)? successfull=false: 0;
	test2->methods->copyStrBetween(test2,test,0,6);
	(test2->length!=6)? successfull=false: 0;
	(strcmp(test2->str,"defghi")!=0)? successfull=false: 0;
	StringClass.delete(&test);
	StringClass.delete(&test2);
	return successfull;
}

static bool _StringDebugSearch(void){
	bool successfull=true;
	String *test=StringClass.new();
	test->methods->set(test,"abcd,cdef,efgh,ghij,ijkl,klmn,mnop,opqr,qruv,uvwx,wxyz");
	// 54 charaters long
	(!test->methods->contains(test,"efgh"))? successfull=false: 0;
	(test->methods->getFirstIndexOf(test,'k')!=22)? successfull=false: 0;
	(test->methods->getLastIndexOf(test,'k')!=25)? successfull=false: 0;
	(test->methods->getCharOccurences(test,"cde")!=6)? successfull=false: 0;
	(test->methods->equals(test,"abcdefghijklmnopqrstuvwxyz"))? successfull=false: 0;
	(!test->methods->equals(test,"abcd,cdef,efgh,ghij,ijkl,klmn,mnop,opqr,qruv,uvwx,wxyz"))? successfull=false: 0;
	IntList *indexes=test->methods->getIndexesOf(test,',');
	(indexes->length!=10 || indexes->list[0]!=4)? successfull=false: 0;
	IntListClass.delete(&indexes);
	StringClass.delete(&test);
	return successfull;
}

static bool _StringDebugMisc(void){
	bool successfull=true;
	String *test=StringClass.new();
	test->methods->set(test,"Testing This String");
	test->methods->toUpperCase(test);
	(strcmp(test->str,"TESTING THIS STRING")!=0)? successfull=false: 0;
	test->methods->toLowerCase(test);
	(strcmp(test->str,"testing this string")!=0)? successfull=false: 0;
	test->methods->setSize(test,10);
	(test->length!=10)? successfull=false: 0;
	StringClass.delete(&test);
	return successfull;
}

static void _StringDestructor(String **obj){
	GenericListClass.delete(&(*obj)->genericList);
	deleteObject((void**)obj);
}
