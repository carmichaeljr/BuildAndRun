#include <stdbool.h>
#include "XMLTag.h"
#include "String.h"
#include "StrStrDictionary.h"
#include "Object.h"
#include "Print.h"


static XMLTag* _XMLTagConstructor(void);
static void _XMLTagPrintObj(XMLTag *obj);
static bool _XMLTagDebug(void);
static bool _XMLTagDebugAdd(void);
static bool _XMLTagDebugRemove(void);
static bool _XMLTagDebugSet(void);
static bool _XMLTagDebugMisc(void);
static bool _XMLTagDebugSetProperlyFormattedData(void);
static bool _XMLTagDebugSetClosingTag(void);
static bool _XMLTagDebugSetExtraniousSpaces(void);
static bool _XMLTagDebugSetHangingValues(void);
static bool _XMLTagDebugSetQuotes(void);
static bool _XMLTagDebugGetRaw(void);
static void _XMLTagDestructor(XMLTag **obj);
static void XMLTag_ParseFromRaw(XMLTag *self, const String *raw);
static String* XMLTag_GetRawOpeningTag(const XMLTag *self);
static String* XMLTag_GetRawClosingTag(const XMLTag *self);
static void XMLTag_SetName(XMLTag *self, const String *newName);
static void XMLTag_SetData(XMLTag *self, const String *newData);
static void XMLTag_SetAttribute(XMLTag *self, const String *key, const String *val);
static void XMLTag_AddAttribute(XMLTag *self, const String *key, const String *val);
static String* XMLTag_GetAttribute(const XMLTag *self, const String *key);
static void XMLTag_RemoveAttribute(XMLTag *self, const String *token);
static void XMLTag_ClearAttributes(XMLTag *self);
static void XMLTag_Clear(XMLTag *self);
static bool XMLTag_Equals(const XMLTag *self, const XMLTag *other);
static IntList* XMLTag_Tokenizer(const String *raw);
static void XMLTag_SetNameFromRaw(XMLTag *self, IntList *tokens, const String *raw);
static void XMLTag_SetAttributesFromRaw(XMLTag *self, IntList *tokens, const String *raw);
static IntList* XMLTag_GetAttrSetIndexes(const IntList *tokens, const String *raw);
static bool XMLTag_AttrIndexesValid(const IntList *attrSetIndexes, const String *raw);
static void XMLTag_AddAttrSet(XMLTag *self, const IntList *attrSetIndexes, const String *raw);
static void XMLTag_SetDataFromRaw(XMLTag *self, IntList *tokens, const String *raw);
static void XMLTag_RemoveUsedTokens(IntList *tokens, const int greatestUsedIndex);

static const struct XMLTagMethods xmlTagMethods={
	.parseFromRaw=&XMLTag_ParseFromRaw,
	.getRawOpeningTag=&XMLTag_GetRawOpeningTag,
	.getRawClosingTag=&XMLTag_GetRawClosingTag,
	.setName=&XMLTag_SetName,
	.setData=&XMLTag_SetData,
	.setAttribute=&XMLTag_SetAttribute,
	.addAttribute=&XMLTag_AddAttribute,
	.getAttribute=&XMLTag_GetAttribute,
	.removeAttribute=&XMLTag_RemoveAttribute,
	.clearAttributes=&XMLTag_ClearAttributes,
	.clear=&XMLTag_Clear,
	.equals=&XMLTag_Equals,
};
const struct XMLTagClass_t XMLTagClass={
	.new=&_XMLTagConstructor,
	.delete=&_XMLTagDestructor,
	.print=&_XMLTagPrintObj,
	.debug=&_XMLTagDebug,
	.objSize=sizeof(XMLTag)
};

//Object Methods================================================================
//Public methods
static void XMLTag_ParseFromRaw(XMLTag *self, const String *raw){
	IntList *tokens=XMLTag_Tokenizer(raw);
	//PrintClass.print("Entire token list\n");
	//IntListClass.print(tokens);
	XMLTag_SetNameFromRaw(self,tokens,raw);
	//PrintClass.print("Tokens after parsing name\n");
	//IntListClass.print(tokens);
	XMLTag_SetAttributesFromRaw(self,tokens,raw);
	//PrintClass.print("Tokens after parsing attrs\n");
	//IntListClass.print(tokens);
	XMLTag_SetDataFromRaw(self,tokens,raw);
	//PrintClass.print("Tokens after parsing data\n");
	//IntListClass.print(tokens);
	IntListClass.delete(&tokens);
}

static String* XMLTag_GetRawOpeningTag(const XMLTag *self){
	String *rv=StringClass.new();
	rv->methods->concatChar(rv,'<');
	rv->methods->concat(rv,self->name->str);
	(self->attributes->size>0)? rv->methods->concatChar(rv,' '): 0;
	StringList *keys=self->attributes->methods->getKeys(self->attributes);
	for (int i=0; i<self->attributes->size; i++){
		String *key=keys->methods->get(keys,i);
		String *val=self->attributes->methods->get(self->attributes,key);
		rv->methods->concat(rv,key->str);
		rv->methods->concatChar(rv,'=');
		rv->methods->concat(rv,val->str);
		if (i<self->attributes->size-1){
			rv->methods->concatChar(rv,' ');
		}
	}
	rv->methods->concatChar(rv,'>');
	StringListClass.delete(&keys);
	return rv;
}

static String* XMLTag_GetRawClosingTag(const XMLTag *self){
	String *rv=StringClass.new();
	rv->methods->concat(rv,"</");
	rv->methods->concat(rv,self->name->str);
	rv->methods->concatChar(rv,'>');
	return rv;
}

static void XMLTag_SetName(XMLTag *self, const String *newName){
	self->name->methods->set(self->name,newName->str);
}

static void XMLTag_SetData(XMLTag *self, const String *newData){
	self->data->methods->set(self->data,newData->str);
}

static void XMLTag_SetAttribute(XMLTag *self, const String *key, const String *val){
	self->attributes->methods->setVal(self->attributes,key,val);
}

static void XMLTag_AddAttribute(XMLTag *self, const String *key, const String *val){
	self->attributes->methods->add(self->attributes,&key,&val,1);
}

static String* XMLTag_GetAttribute(const XMLTag *self, const String *key){
	return self->attributes->methods->get(self->attributes,key);
}

static void XMLTag_RemoveAttribute(XMLTag *self, const String *token){
	self->attributes->methods->remove(self->attributes,token);
}

static void XMLTag_ClearAttributes(XMLTag *self){
	self->attributes->methods->clear(self->attributes);
}

static void XMLTag_Clear(XMLTag *self){
	self->name->methods->set(self->name,"");
	self->data->methods->set(self->data,"");
	self->methods->clearAttributes(self);
}

static bool XMLTag_Equals(const XMLTag *self, const XMLTag *other){
	bool rv=self->name->methods->equals(self->name,other->name->str);
	rv&=self->data->methods->equals(self->data,other->data->str);
	rv&=self->attributes->methods->equals(self->attributes,other->attributes);
	return rv;
}

//Private methods
static IntList* XMLTag_Tokenizer(const String *raw){
	IntList *tokens=IntListClass.new();
	IntList *spaces=raw->methods->getIndexesOf(raw,' ');
	IntList *equalSigns=raw->methods->getIndexesOf(raw,'=');
	IntList *quotes=raw->methods->getIndexesOf(raw,'"');
	IntList *openBrackets=raw->methods->getIndexesOf(raw,'<');
	IntList *closeBrackets=raw->methods->getIndexesOf(raw,'>');
	int endSlash=raw->methods->getLastIndexOf(raw,'/');

	tokens->methods->splice(tokens,spaces);
	tokens->methods->splice(tokens,equalSigns);
	//Remove tokens between ><
	int dataStart=raw->methods->getFirstIndexOf(raw,'>');
	int dataEnd=raw->methods->getLastIndexOf(raw,'<');
	if (dataEnd>dataStart && dataStart>0 && dataEnd>0){
		tokens->methods->removeNumRange(tokens,dataStart,dataEnd+1);
	}
	for (int i=0; i<quotes->length; i+=2){
		tokens->methods->removeNumRange(tokens,quotes->list[i],quotes->list[i+1]+1);
	}
	tokens->methods->splice(tokens,openBrackets);
	tokens->methods->splice(tokens,closeBrackets);
	(endSlash>0)? tokens->methods->add(tokens,endSlash): 0;
	tokens->methods->sort(tokens);

	IntListClass.delete(&spaces);
	IntListClass.delete(&equalSigns);
	IntListClass.delete(&quotes);
	IntListClass.delete(&openBrackets);
	IntListClass.delete(&closeBrackets);
	return tokens;
}

static void XMLTag_SetNameFromRaw(XMLTag *self, IntList *tokens, const String *raw){
	bool _break=false;
	for (int i=1; i<tokens->length && !_break; i++){
		if (tokens->list[i]-tokens->list[i-1]>1){
			self->name->methods->copyStrBetween(
					self->name,raw,tokens->list[i-1]+1,tokens->list[i]);
			XMLTag_RemoveUsedTokens(tokens,i);
			_break=true;
		}
	}
}

static void XMLTag_SetAttributesFromRaw(XMLTag *self, IntList *tokens, const String *raw){
	if (tokens->length>1 && 
	    raw->str[tokens->list[0]]!='/' && raw->str[tokens->list[0]]==' '){
		bool _break=false;
		while (!_break){
			IntList *attrSet=XMLTag_GetAttrSetIndexes(tokens,raw);
			if (XMLTag_AttrIndexesValid(attrSet,raw)){
				XMLTag_AddAttrSet(self,attrSet,raw);
				XMLTag_RemoveUsedTokens(tokens,1);
			}
			XMLTag_RemoveUsedTokens(tokens,1);
			if (raw->str[attrSet->list[attrSet->length-1]]=='>'){
				_break=true;
			}
			IntListClass.delete(&attrSet);
		}
	}
}

static IntList* XMLTag_GetAttrSetIndexes(const IntList *tokens, const String *raw){
	IntList *rv=IntListClass.new();
	rv->methods->add(rv,tokens->list[0]);
	for (int i=1; i<tokens->length && rv->length<3; i++){
		if (tokens->list[i]-rv->list[rv->length-1]==1 &&
		    raw->str[tokens->list[i]]=='=' && raw->str[rv->list[rv->length-1]]==' '){
			rv->list[rv->length-1]=tokens->list[i];
		} else if (raw->str[tokens->list[i]]=='>' || raw->str[tokens->list[i]]=='/'){
			rv->methods->add(rv,tokens->list[i]);
		} else if (tokens->list[i]-rv->list[rv->length-1]>1){
			rv->methods->add(rv,tokens->list[i]);
		} 
	}
	return rv;
}

static bool XMLTag_AttrIndexesValid(const IntList *attrSetIndexes, const String *raw){
	bool rv=(attrSetIndexes->length==3)? true: false;
	if (rv){
		//Two spaces or two equal signs in a row indicate improper formatting
		//Two ending characters in one set of indexes indicates improper formatting
		int endBracketCount=0;
		for (int i=1; i<3 && rv; i++){
			if (raw->str[attrSetIndexes->list[i]]==raw->str[attrSetIndexes->list[i-1]]){
				rv=false;
			} else if (raw->str[attrSetIndexes->list[i]]=='>' ||
				raw->str[attrSetIndexes->list[i]]=='/'){
				endBracketCount++;
			}
		}
		(endBracketCount>1)? rv=false: 0;
	}
	return rv;
}

static void XMLTag_AddAttrSet(XMLTag *self, const IntList *attrSetIndexes, const String *raw){
	String *key=StringClass.new();
	String *val=StringClass.new();
	StrStrDictionary *attrs=self->attributes;
	key->methods->copyStrBetween(key,raw,attrSetIndexes->list[0]+1,attrSetIndexes->list[1]);
	val->methods->copyStrBetween(val,raw,attrSetIndexes->list[1]+1,attrSetIndexes->list[2]);
	key->methods->trimWhitespace(key);
	val->methods->trimWhitespace(val);
	attrs->methods->add(attrs,(const String**)&key,(const String**)&val,1);
}

static void XMLTag_SetDataFromRaw(XMLTag *self, IntList *tokens, const String *raw){
	int startIndex=(tokens->length>0)? tokens->list[0]: -1;
	int endIndex=(tokens->length>0)? tokens->list[tokens->length-1]: -1;
	int tokenRemovalIndex=tokens->length;
	bool _break=false;
	for (int i=0; i<tokens->length && !_break; i++){
		if (raw->str[tokens->list[i]]=='>'){
			startIndex=tokens->list[i];
		} else if (startIndex>0 && raw->str[tokens->list[i]]=='<'){
			endIndex=tokens->list[i];
			tokenRemovalIndex=i;
			_break=true;
		}
	}
	if (startIndex>0){
		self->data->methods->copyStrBetween(self->data,raw,startIndex+1,endIndex);
	}
	XMLTag_RemoveUsedTokens(tokens,tokenRemovalIndex);
}

static void XMLTag_RemoveUsedTokens(IntList *tokens, const int greatestUsedIndex){
	tokens->methods->removeBetween(tokens,0,greatestUsedIndex);
}


//Class Methods================================================================
static XMLTag* _XMLTagConstructor(void){
	XMLTag *rv=(XMLTag*)createObject(XMLTagClass.objSize);
	rv->name=StringClass.new();
	rv->data=StringClass.new();
	rv->attributes=StrStrDictionaryClass.new();
	rv->attributes->ownsData=true;
	rv->methods=&xmlTagMethods;
	return rv;
}

static void _XMLTagPrintObj(XMLTag *obj){
	PrintClass.print("<XMLTag Obj[Addr: %p]>\n",obj);
	PrintClass.incrementIndentLevel(1);
	StringClass.print(obj->name);
	StringClass.print(obj->data);
	StrStrDictionaryClass.print(obj->attributes);
	PrintClass.incrementIndentLevel(-1);
}

static bool _XMLTagDebug(void){
	bool successfull=true;
	successfull&=_XMLTagDebugSet();
	successfull&=_XMLTagDebugAdd();
	successfull&=_XMLTagDebugRemove();
	successfull&=_XMLTagDebugMisc();
	successfull&=_XMLTagDebugSetProperlyFormattedData();
	successfull&=_XMLTagDebugSetClosingTag();
	successfull&=_XMLTagDebugSetExtraniousSpaces();
	successfull&=_XMLTagDebugSetHangingValues();
	successfull&=_XMLTagDebugSetQuotes();
	successfull&=_XMLTagDebugGetRaw();
	PrintClass.objectDebug("XMLTag",successfull);
	return successfull;
}

static bool _XMLTagDebugSet(void){
	bool successfull=true;
	XMLTag *temp=XMLTagClass.new();
	String *name=StringClass.new();
	String *data=StringClass.new();
	String *attrKey=StringClass.new();
	String *attrVal1=StringClass.new();
	String *attrVal2=StringClass.new();
	name->methods->set(name,"TestName");
	data->methods->set(data,"TestData");
	attrKey->methods->set(attrKey,"key");
	attrVal1->methods->set(attrVal1,"val");
	attrVal2->methods->set(attrVal2,"val2");
	temp->methods->setName(temp,(const String*)name);
	temp->methods->setData(temp,(const String*)data);
	temp->methods->addAttribute(temp,attrKey,attrVal1);
	temp->methods->setAttribute(temp,attrKey,attrVal2);
	(!temp->name->methods->equals(temp->name,"TestName"))? successfull=false: 0;
	(!temp->data->methods->equals(temp->data,"TestData"))? successfull=false: 0;
	//XMLTagClass.print(temp);
	StringClass.delete(&name);
	StringClass.delete(&data);
	XMLTagClass.delete(&temp);
	return successfull;
}

static bool _XMLTagDebugAdd(void){
	bool successfull=true;
	String *key=StringClass.new();
	String *val=StringClass.new();
	XMLTag *temp=XMLTagClass.new();
	key->methods->set(key,"testKey");
	val->methods->set(val,"testVal");
	temp->methods->addAttribute(temp,key,val);
	String *retVal=temp->methods->getAttribute(temp,key);
	(!retVal->methods->equals(retVal,"testVal"))? successfull=false: 0;
	//XMLTagClass.print(temp);
	XMLTagClass.delete(&temp);
	return successfull;
}

static bool _XMLTagDebugRemove(void){
	bool successfull=true;
	String *key=StringClass.new();
	String *val=StringClass.new();
	String *key2=StringClass.new();
	String *val2=StringClass.new();
	XMLTag *temp=XMLTagClass.new();
	key->methods->set(key,"testKey");
	val->methods->set(val,"testVal");
	key2->methods->set(key2,"testKey2");
	val2->methods->set(val2,"testVal2");
	temp->name->methods->set(temp->name,"testName");
	temp->data->methods->set(temp->data,"testData");
	temp->methods->addAttribute(temp,key,val);
	temp->methods->addAttribute(temp,key2,val2);
	temp->methods->removeAttribute(temp,key);
	(temp->attributes->size!=1)? successfull=false: 0;
	temp->methods->clearAttributes(temp);
	(temp->attributes->size>0)? successfull=false: 0;
	temp->methods->clear(temp);
	(temp->name->length>0)? successfull=false: 0;
	(temp->data->length>0)? successfull=false: 0;
	//XMLTagClass.print(temp);
	XMLTagClass.delete(&temp);
	return successfull;
}

static bool _XMLTagDebugMisc(void){
	bool successfull=true;
	String *key=StringClass.new();
	String *val=StringClass.new();
	String *key2=StringClass.new();
	String *val2=StringClass.new();
	XMLTag *tag1=XMLTagClass.new();
	XMLTag *tag2=XMLTagClass.new();
	key->methods->set(key,"testKey");
	key2->methods->set(key2,"testKey");
	val->methods->set(val,"testVal");
	val2->methods->set(val2,"testVal");
	tag1->name->methods->set(tag1->name,"testName");
	tag2->name->methods->set(tag2->name,"testName");
	tag1->methods->addAttribute(tag1,key,val);
	tag2->methods->addAttribute(tag2,key2,val2);
	(!tag1->methods->equals(tag1,tag2))? successfull=false: 0;
	XMLTagClass.delete(&tag1);
	XMLTagClass.delete(&tag2);
	return successfull;
}

static bool _XMLTagDebugSetProperlyFormattedData(void){
	bool successfull=true;
	XMLTag *temp=XMLTagClass.new();
	String *data=StringClass.new();
	String *testKey=StringClass.new();
	data->methods->set(data,"<TestName testAtr1=1 testAtr2=2>Test Value</TestName>");
	testKey->methods->set(testKey,"testAtr1");
	temp->methods->parseFromRaw(temp,data);
	(!temp->name->methods->equals(temp->name,"TestName"))? successfull=false: 0;
	(temp->attributes->size!=2)? successfull=false: 0;
	String *val=temp->attributes->methods->get(temp->attributes,testKey);
	(!val->methods->equals(val,"1"))? successfull=false: 0;
	(!temp->data->methods->equals(temp->data,"Test Value"))? successfull=false: 0;
	//XMLTagClass.print(temp);
	StringClass.delete(&data);
	StringClass.delete(&testKey);
	XMLTagClass.delete(&temp);
	return successfull;
}

static bool _XMLTagDebugSetClosingTag(void){
	bool successfull=true;
	XMLTag *temp=XMLTagClass.new();
	String *data=StringClass.new();
	String *testKey=StringClass.new();
	data->methods->set(data,"<TestName testAttr=1 testAttr=2/>");
	testKey->methods->set(testKey,"testAttr");
	temp->methods->parseFromRaw(temp,data);
	(!temp->name->methods->equals(temp->name,"TestName"))? successfull=false: 0;
	(temp->attributes->size!=2)? successfull=false: 0;
	String *val=temp->attributes->methods->get(temp->attributes,testKey);
	(!val->methods->equals(val,"1"))? successfull=false: 0;
	(temp->data->length>0)? successfull=false: 0;
	//XMLTagClass.print(temp);
	StringClass.delete(&data);
	StringClass.delete(&testKey);
	XMLTagClass.delete(&temp);
	return successfull;
}

static bool _XMLTagDebugSetExtraniousSpaces(void){
	bool successfull=true;
	XMLTag *temp=XMLTagClass.new();
	String *data=StringClass.new();
	String *testKey=StringClass.new();
	data->methods->set(data,"<TestName testAttr1= val1 testAttr2 =val2 testAttr3 = val3/>");
	testKey->methods->set(testKey,"testAttr1");
	temp->methods->parseFromRaw(temp,data);
	(!temp->name->methods->equals(temp->name,"TestName"))? successfull=false: 0;
	(temp->attributes->size!=3)? successfull=false: 0;
	String *val=temp->attributes->methods->get(temp->attributes,testKey);
	(!val->methods->equals(val,"val1"))? successfull=false: 0;
	(temp->data->length>0)? successfull=false: 0;
	//XMLTagClass.print(temp);
	StringClass.delete(&data);
	StringClass.delete(&testKey);
	XMLTagClass.delete(&temp);
	return successfull;
}

static bool _XMLTagDebugSetHangingValues(void){
	bool successfull=true;
	XMLTag *temp=XMLTagClass.new();
	String *data=StringClass.new();
	String *testKey=StringClass.new();
	data->methods->set(data,"<TestName testAttr1= testAttr2=val2 testAttr3=/>");
	testKey->methods->set(testKey,"testAttr2");
	temp->methods->parseFromRaw(temp,data);
	(!temp->name->methods->equals(temp->name,"TestName"))? successfull=false: 0;
	(temp->attributes->size!=2)? successfull=false: 0;
	String *val=temp->attributes->methods->get(temp->attributes,testKey);
	(!val->methods->equals(val,"val2"))? successfull=false: 0;
	(temp->data->length>0)? successfull=false: 0;
	//XMLTagClass.print(temp);
	StringClass.delete(&data);
	StringClass.delete(&testKey);
	XMLTagClass.delete(&temp);
	return successfull;

}

static bool _XMLTagDebugSetQuotes(void){
	bool successfull=true;
	XMLTag *temp=XMLTagClass.new();
	String *data=StringClass.new();
	String *testKey=StringClass.new();
	data->methods->set(data,"<TestName testAtr1=\"1\" testAtr2=\"2\">Test Value</TestName>");
	testKey->methods->set(testKey,"testAtr1");
	temp->methods->parseFromRaw(temp,data);
	(!temp->name->methods->equals(temp->name,"TestName"))? successfull=false: 0;
	(temp->attributes->size!=2)? successfull=false: 0;
	String *val=temp->attributes->methods->get(temp->attributes,testKey);
	(!val->methods->equals(val,"\"1\""))? successfull=false: 0;
	(!temp->data->methods->equals(temp->data,"Test Value"))? successfull=false: 0;
	//XMLTagClass.print(temp);
	StringClass.delete(&data);
	StringClass.delete(&testKey);
	XMLTagClass.delete(&temp);
	return successfull;
}

static bool _XMLTagDebugGetRaw(void){
	bool successfull=true;
	XMLTag *temp=XMLTagClass.new();
	String *data=StringClass.new();
	data->methods->set(data,"<TestName testAtr1=1 testAtr2=2>Test Value</TestName>");
	temp->methods->parseFromRaw(temp,data);
	String *returnedTag=temp->methods->getRawOpeningTag(temp);
	returnedTag->methods->concat(returnedTag,temp->data->str);
	String *retCloseTag=temp->methods->getRawClosingTag(temp);
	returnedTag->methods->concat(returnedTag,retCloseTag->str);
	(!data->methods->equals(data,returnedTag->str))? successfull=false: 0;
	//StringClass.print(data);
	//StringClass.print(returnedTag);
	//XMLTagClass.print(temp);
	StringClass.delete(&returnedTag);
	StringClass.delete(&retCloseTag);
	XMLTagClass.delete(&temp);
	return successfull;
}

static void _XMLTagDestructor(XMLTag **obj){
	StringClass.delete(&(*obj)->name);
	StringClass.delete(&(*obj)->data);
	StrStrDictionaryClass.delete(&(*obj)->attributes);
	deleteObject((void**)obj);
}
