#include <string.h>
#include <stdbool.h>
#include "XMLFile.h"
#include "Object.h"
#include "Print.h"
#include "GenericTree.h"
#include "StringList.h"
#include "File.h"
#include "XMLTag.h"

static XMLFile* _XMLFileConstructor(void);
static void _XMLFilePrintObj(XMLFile *obj);
static bool _XMLFileDebug(void);
static bool _XMLFileDebugParser(const String *path);
static bool _XMLFileDebugWrite(const String *path);
static bool _XMLFileDebugAdd(const String *path);
static bool _XMLFileDebugRemove(const String *path);
static bool _XMLFileDebugSet(const String *path);
static bool _XMLFileDebugSearch(const String *path);
static bool _XMLFileDebugMisc(const String *path);
static bool _XMLFileDebugCopy(const String *path);
static void _XMLFileDestructor(XMLFile **obj);
static void XMLFile_SetSource(XMLFile *self, const String *path);
static void XMLFile_Write(XMLFile *self);
static bool XMLFile_PlaceCurrentNode(XMLFile *self, const StringList *path);
static bool XMLFile_MoveDown(XMLFile *self);
static bool XMLFile_MoveNext(XMLFile *self);
static bool XMLFile_MoveUp(XMLFile *self);
static bool XMLFile_StartIter(XMLFile *self);
static bool XMLFile_StartIterAtCurNode(XMLFile *self);
static bool XMLFile_HasNext(const XMLFile *self);
static XMLTag* XMLFile_GetNext(XMLFile *self);
static char XMLFile_GetIterAction(const XMLFile *self);
static void XMLFile_Add(XMLFile *self, const XMLTag *tag);
static void XMLFile_AddAt(XMLFile *self, const StringList *path, const XMLTag *tag);
static void XMLFile_Set(XMLFile *self, const XMLTag *newTag);
static void XMLFile_SetAt(XMLFile *self, const StringList *path, const XMLTag *newTag);
static XMLTag* XMLFile_Get(const XMLFile *self);
static XMLTag* XMLFile_GetAt(XMLFile *self, const StringList *path);
static int XMLFile_GetNumChildTags(const XMLFile *self);
static int XMLFile_GetNumChildTagsAt(XMLFile *self, const StringList *path);
static bool XMLFile_BranchExists(XMLFile *self, const StringList *path);
static void XMLFile_CopyOtherBelowCurNode(XMLFile *self, XMLFile *other);
static void XMLFile_Remove(XMLFile *self);
static void XMLFile_Clear(XMLFile *self);
static bool XMLFile_Equals(XMLFile *self, XMLFile *other);
static bool XMLFile_IsEmpty(const XMLFile *self);
static void XMLFile_SetHeadNode(XMLFile *self);
static void XMLFile_ParseFile(XMLFile *self);
static IntList* XMLFile_Tokenizer(const String *raw);
static void XMLFile_ParseLine(XMLFile *self, const String *line);
static IntList* XMLFile_GetTagSetIndexes(const IntList *tokens, const String *raw);
static bool XMLFile_TagSetIndexesValid(const IntList *tagSet, const String *raw);
static void XMLFile_ModifyTree(XMLFile *self, const IntList *tagSet, const String *raw);
static String* XMLFile_IsTagData(const IntList *tagSet, const String *raw);
static XMLTag* XMLFile_IsOpeningTag(const XMLFile *self, const IntList *tagSet, const String *raw);
static bool XMLFile_IsClosingTag(const IntList *tagSet, const String *raw);
static void XMLFile_RemoveUsedTokens(IntList *tokens, const int greatestUsedIndex);
static bool XMLFile_PathExistsBelow(XMLFile *self, const StringList *path);
static bool XMLFile_TagInChildTags(XMLFile *self, const String *name);
static int XMLFile_GetIndentIncrement(XMLFile *self, const char iterAction);
static String* XMLFile_GetLineIndenting(XMLFile *self, const int indent);
static String* XMLFile_GenerateNextLine(XMLFile *self, const XMLTag *tag, const char iterAction, const int indent);
static void XMLFile_ConvertFromGTToXMLFile(XMLFile *self);

static const struct XMLFileMethods xmlFileMethods={
	.setSource=&XMLFile_SetSource,
	.write=&XMLFile_Write,
	.placeCurrentNode=&XMLFile_PlaceCurrentNode,
	.moveDown=&XMLFile_MoveDown,
	.moveNext=&XMLFile_MoveNext,
	.moveUp=&XMLFile_MoveUp,
	.startIter=&XMLFile_StartIter,
	.startIterAtCurNode=&XMLFile_StartIterAtCurNode,
	.hasNext=&XMLFile_HasNext,
	.getNext=&XMLFile_GetNext,
	.getIterAction=&XMLFile_GetIterAction,
	.add=&XMLFile_Add,
	.addAt=&XMLFile_AddAt,
	.set=&XMLFile_Set,
	.setAt=&XMLFile_SetAt,
	.get=&XMLFile_Get,
	.getAt=&XMLFile_GetAt,
	.getNumChildTags=&XMLFile_GetNumChildTags,
	.getNumChildTagsAt=&XMLFile_GetNumChildTagsAt,
	.branchExists=&XMLFile_BranchExists,
	.copyOtherBelowCurNode=&XMLFile_CopyOtherBelowCurNode,
	.remove=&XMLFile_Remove,
	.clear=&XMLFile_Clear,
	.equals=&XMLFile_Equals,
	.isEmpty=&XMLFile_IsEmpty,
};
const struct XMLFileClass_t XMLFileClass={
	.new=&_XMLFileConstructor,
	.delete=&_XMLFileDestructor,
	.print=&_XMLFilePrintObj,
	.debug=&_XMLFileDebug,
	.objSize=sizeof(XMLFile)
};

//Object Methods===============================================================
//Public Methods
static void XMLFile_SetSource(XMLFile *self, const String *path){
	self->methods->write(self);
	self->source->methods->setPath(self->source,path);
	if (FileClass.fileExists(self->source)){
		self->methods->clear(self);
		XMLFile_SetHeadNode(self);
		XMLFile_ParseFile(self);
	}
	XMLFile_ConvertFromGTToXMLFile(self);
}

static void XMLFile_Write(XMLFile *self){
	int indent=-1;
	self->gt->methods->pushCurrentNode(self->gt);
	if (self->gt->methods->startIter(self->gt)){
		self->source->methods->clear(self->source);
		while (self->gt->methods->hasNext(self->gt)){
			XMLTag *iterTag=self->gt->methods->getNext(self->gt);
			if (!iterTag->name->methods->equals(iterTag->name,"root")){
				char iterAction=self->gt->methods->getIterAction(self->gt);
				//Order of the next two function calls is important
				indent+=XMLFile_GetIndentIncrement(self,iterAction);
				String *line=XMLFile_GenerateNextLine(self,iterTag,iterAction,indent);
				self->source->methods->append(self->source,line);
				StringClass.delete(&line);
			}
		}
	}
	self->gt->methods->popCurrentNode(self->gt);
}

static bool XMLFile_PlaceCurrentNode(XMLFile *self, const StringList *path){
	bool rv[2]={ false,false };
	rv[0]=XMLFile_PathExistsBelow(self,path);
	self->gt->methods->pushCurrentNode(self->gt);
	self->gt->methods->placeCurrentNodeAtHeadNode(self->gt);
	rv[1]=(!rv[0])? XMLFile_PathExistsBelow(self,path): false;
	if (!rv[0] && rv[1]){
		self->gt->methods->clearPushedNodes(self->gt);
	} else if (rv[0] && !rv[1]){
		self->gt->methods->popCurrentNode(self->gt);
	}
	return (rv[0] || rv[1]);
}

static bool XMLFile_MoveDown(XMLFile *self){
	return self->gt->methods->moveDown(self->gt);
}

static bool XMLFile_MoveNext(XMLFile *self){
	return self->gt->methods->moveNext(self->gt);
}

static bool XMLFile_MoveUp(XMLFile *self){
	return self->gt->methods->moveUp(self->gt);
}

static bool XMLFile_StartIter(XMLFile *self){
	return self->gt->methods->startIter(self->gt);
}

static bool XMLFile_StartIterAtCurNode(XMLFile *self){
	return self->gt->methods->startIterAtCurNode(self->gt);
}

static bool XMLFile_HasNext(const XMLFile *self){
	return self->gt->methods->hasNext(self->gt);
}

static XMLTag* XMLFile_GetNext(XMLFile *self){
	return (XMLTag*)self->gt->methods->getNext(self->gt);
}

static char XMLFile_GetIterAction(const XMLFile *self){
	return self->gt->methods->getIterAction(self->gt);
}

static void XMLFile_Add(XMLFile *self, const XMLTag *tag){
	self->gt->methods->add(self->gt,tag);
	XMLFile_ConvertFromGTToXMLFile(self);
}

static void XMLFile_AddAt(XMLFile *self, const StringList *path, const XMLTag *tag){
	self->gt->methods->pushCurrentNode(self->gt);
	if (XMLFile_PlaceCurrentNode(self,path)){
		self->gt->methods->add(self->gt,tag);
		XMLFile_ConvertFromGTToXMLFile(self);
	}
	self->gt->methods->popCurrentNode(self->gt);
}

static void XMLFile_Set(XMLFile *self, const XMLTag *newTag){
	self->gt->methods->set(self->gt,newTag);
}

static void XMLFile_SetAt(XMLFile *self, const StringList *path, const XMLTag *newTag){
	self->gt->methods->pushCurrentNode(self->gt);
	if (XMLFile_PlaceCurrentNode(self,path)){
		self->gt->methods->set(self->gt,newTag);
	}
	self->gt->methods->popCurrentNode(self->gt);
}

static XMLTag* XMLFile_Get(const XMLFile *self){
	return (XMLTag*)self->gt->methods->get(self->gt);
}

static XMLTag* XMLFile_GetAt(XMLFile *self, const StringList *path){
	XMLTag *rv=NULL;
	self->gt->methods->pushCurrentNode(self->gt);
	if (XMLFile_PlaceCurrentNode(self,path)){
		rv=(XMLTag*)self->gt->methods->get(self->gt);
	}
	self->gt->methods->popCurrentNode(self->gt);
	return rv;
}

static int XMLFile_GetNumChildTags(const XMLFile *self){
	return self->gt->methods->getNumChildNodes(self->gt);
}

static int XMLFile_GetNumChildTagsAt(XMLFile *self, const StringList *path){
	int rv=-1;
	self->gt->methods->pushCurrentNode(self->gt);
	if (XMLFile_PlaceCurrentNode(self,path)){
		rv=self->gt->methods->getNumChildNodes(self->gt);
	}
	self->gt->methods->popCurrentNode(self->gt);
	return rv;
}

static bool XMLFile_BranchExists(XMLFile *self, const StringList *path){
	bool rv[2]={ false,false };
	self->gt->methods->pushCurrentNode(self->gt);
	rv[0]=XMLFile_PathExistsBelow(self,path);
	self->gt->methods->popCurrentNode(self->gt);
	self->gt->methods->pushCurrentNode(self->gt);
	self->gt->methods->placeCurrentNodeAtHeadNode(self->gt);
	rv[1]=(!rv[0])? XMLFile_PathExistsBelow(self,path): false;
	self->gt->methods->popCurrentNode(self->gt);
	return (rv[0] || rv[1]);
}

static void XMLFile_CopyOtherBelowCurNode(XMLFile *self, XMLFile *other){
	GenericTree *selfGt=self->gt;
	GenericTree *otherGt=other->gt;
	if (self->numTags==0){
		XMLFile_SetHeadNode(self);
	}
	selfGt->methods->copyOtherBelowCurNode(selfGt,otherGt);
	XMLFile_ConvertFromGTToXMLFile(self);
}

static void XMLFile_Remove(XMLFile *self){
	GenericTree *temp=self->gt;
	if (self->ownsData && temp->methods->startIterAtCurNode(temp)){
		while (temp->methods->hasNext(temp)){
			XMLTag *iterTag=temp->methods->getNext(temp);
			if (iterTag!=NULL){
				XMLTagClass.delete(&iterTag);
				temp->methods->set(temp,NULL);
			}
		}
	}
	temp->methods->remove(temp);
	XMLFile_ConvertFromGTToXMLFile(self);
}

static void XMLFile_Clear(XMLFile *self){
	GenericTree *temp=self->gt;
	if (self->ownsData && temp->methods->startIter(temp)){
		while (temp->methods->hasNext(temp)){
			XMLTag *iterTag=temp->methods->getNext(temp);
			if (iterTag!=NULL){
				XMLTagClass.delete(&iterTag);
				temp->methods->set(temp,NULL);
			}
		}
	}
	temp->methods->clear(temp);
	XMLFile_ConvertFromGTToXMLFile(self);
}

static bool XMLFile_Equals(XMLFile *self, XMLFile *other){
	bool rv=(self->numTags==other->numTags);
	self->gt->methods->pushCurrentNode(self->gt);
	other->gt->methods->pushCurrentNode(other->gt);
	if (rv && self->gt->methods->startIter(self->gt) && 
		  other->gt->methods->startIter(other->gt)){
		while (rv && self->gt->methods->hasNext(self->gt) &&
		       other->gt->methods->hasNext(other->gt)){
			XMLTag *temp1=self->gt->methods->getNext(self->gt);
			XMLTag *temp2=other->gt->methods->getNext(other->gt);
			rv&=temp1->methods->equals(temp1,temp2);
		}
	}
	self->gt->methods->popCurrentNode(self->gt);
	other->gt->methods->popCurrentNode(other->gt);
	return rv;
}

static bool XMLFile_IsEmpty(const XMLFile *self){
	return (self->numTags>0)? false: true;
}

//Private Methods
static void XMLFile_SetHeadNode(XMLFile *self){
	String *name=StringClass.new();
	XMLTag *node=XMLTagClass.new();
	name->methods->set(name,"root");
	node->methods->setName(node,name);
	self->gt->methods->add(self->gt,node);
	StringClass.delete(&name);
}

static void XMLFile_ParseFile(XMLFile *self){
	if (self->source->methods->startIter(self->source)){
		while (self->source->methods->hasNext(self->source)){
			String *line=self->source->methods->getNextLine(self->source);
			XMLFile_ParseLine(self,line);
			StringClass.delete(&line);
		}
	}
	self->source->methods->endIter(self->source);
}

static void XMLFile_ParseLine(XMLFile *self, const String *line){
	bool _break=false;
	IntList *tokens=XMLFile_Tokenizer(line);
	while (!_break){
		IntList *tagSet=XMLFile_GetTagSetIndexes(tokens,line);
		if (XMLFile_TagSetIndexesValid(tagSet,line)){
			XMLFile_ModifyTree(self,tagSet,line);
		}
		XMLFile_RemoveUsedTokens(tokens,1);
		if (tokens->length<2){
			_break=true;
		}
		IntListClass.delete(&tagSet);
	}
	IntListClass.delete(&tokens);
}

static IntList* XMLFile_Tokenizer(const String *raw){
	IntList *tokens=IntListClass.new();
	IntList *openBrackets=raw->methods->getIndexesOf(raw,'<');
	IntList *closeBrackets=raw->methods->getIndexesOf(raw,'>');
	IntList *slashes=raw->methods->getIndexesOf(raw,'/');
	IntList *quotes=raw->methods->getIndexesOf(raw,'"');
	tokens->methods->splice(tokens,openBrackets);
	tokens->methods->splice(tokens,closeBrackets);
	tokens->methods->splice(tokens,slashes);
	//Remove tokens between ><
	int dataStart=raw->methods->getFirstIndexOf(raw,'>');
	int dataEnd=raw->methods->getLastIndexOf(raw,'<');
	if (dataEnd>dataStart && dataStart>0 && dataEnd>0){
		//StringClass.print(raw);
		tokens->methods->removeNumRange(tokens,dataStart,dataEnd+1);
	}
	for (int i=0; i<quotes->length; i+=2){
		tokens->methods->removeNumRange(tokens,quotes->list[i],quotes->list[i+1]+1);
	}
	tokens->methods->sort(tokens);
	IntListClass.delete(&openBrackets);
	IntListClass.delete(&closeBrackets);
	IntListClass.delete(&slashes);
	IntListClass.delete(&quotes);
	return tokens;
}


static IntList* XMLFile_GetTagSetIndexes(const IntList *tokens, const String *raw){
	int limit=2;
	IntList *rv=IntListClass.new();
	for (int i=0; i<limit && i<tokens->length; i++){
		rv->methods->add(rv,tokens->list[i]);
		if (raw->str[tokens->list[i]]=='/'){
			limit++;
		}
	}
	return rv;
}

static bool XMLFile_TagSetIndexesValid(const IntList *tagSet, const String *raw){
	bool rv=(tagSet->length<2 || tagSet->length>3)? false: true;
	if (rv){
		//Should only have one opening, and one closing bracket
		//Slash should always be in the middle
		int bracketCount[2]={ 0,0 };
		for (int i=0; i<tagSet->length; i++){
			if (raw->str[tagSet->list[i]]=='<'){
				bracketCount[0]++;
			} else if (raw->str[tagSet->list[i]]=='>'){
				bracketCount[1]++;
			}
		}
		if (tagSet->length==3){
			rv&=(raw->str[tagSet->list[1]]=='/')? true: false;
		}
		rv&=(bracketCount[0]==1 && bracketCount[1]==1)? true: false;
	}
	return rv;
}

static void XMLFile_ModifyTree(XMLFile *self, const IntList *tagSet, const String *raw){
	String *data=XMLFile_IsTagData(tagSet,raw);
	XMLTag *node=XMLFile_IsOpeningTag(self,tagSet,raw);
	bool closingTag=XMLFile_IsClosingTag(tagSet,raw);
	if (data!=NULL){
		XMLTag *currentTag=self->gt->methods->get(self->gt);
		currentTag->methods->setData(currentTag,data);
		StringClass.delete(&data);
	} else if (node!=NULL){
		self->gt->methods->add(self->gt,node);
		self->gt->methods->placeCurrentNode(self->gt,node,1);
	}
	if (closingTag){
		self->gt->methods->moveUp(self->gt);
	}
}

static String* XMLFile_IsTagData(const IntList *tagSet, const String *raw){
	String *rv=NULL;
	if (tagSet->length==2 && tagSet->list[1]-tagSet->list[0]>1 &&
	    raw->str[tagSet->list[0]]=='>' && raw->str[tagSet->list[1]]=='<'){
		rv=StringClass.new();
		rv->methods->copyStrBetween(rv,raw,tagSet->list[0]+1,tagSet->list[tagSet->length-1]);
	}
	return rv;
}

static XMLTag* XMLFile_IsOpeningTag(const XMLFile *self, const IntList *tagSet, const String *raw){
	XMLTag *rv=XMLTagClass.new();
	String *rawTag=StringClass.new();
	XMLTag *currentTag=self->gt->methods->get(self->gt);
	rawTag->methods->copyStrBetween(rawTag,raw,tagSet->list[0],tagSet->list[tagSet->length-1]+1);
	rv->methods->parseFromRaw(rv,rawTag);
	StringClass.delete(&rawTag);
	if (rv->name->length>0 && 
	    raw->str[tagSet->list[0]]=='<' && raw->str[tagSet->list[tagSet->length-1]]=='>' &&
	    !rv->name->methods->equals(rv->name,currentTag->name->str)){
		return rv;
	}
	XMLTagClass.delete(&rv);
	return NULL;
}

static bool XMLFile_IsClosingTag(const IntList *tagSet, const String *raw){
	if (tagSet->length==3 && raw->str[tagSet->list[1]]=='/'){
		return true;
	}
	return false;
}

static void XMLFile_RemoveUsedTokens(IntList *tokens, const int greatestUsedIndex){
	tokens->methods->removeBetween(tokens,0,greatestUsedIndex);
}

static bool XMLFile_PathExistsBelow(XMLFile *self, const StringList *path){
	bool rv=true;
	for (int i=0; i<path->length && rv; i++){
		String *iterStr=path->methods->get(path,i);
		rv&=XMLFile_TagInChildTags(self,iterStr);
	}
	return rv;
}

static bool XMLFile_TagInChildTags(XMLFile *self, const String *name){
	bool rv=self->methods->moveDown(self);
	XMLTag *temp=NULL;
	if (rv){
		temp=(XMLTag*)self->gt->methods->get(self->gt);
		rv=name->methods->equals(name,temp->name->str);
		while (!rv && self->methods->moveNext(self)){
			temp=(XMLTag*)self->gt->methods->get(self->gt);
			rv=name->methods->equals(name,temp->name->str);
		}
	}
	return rv;
}

static int XMLFile_GetIndentIncrement(XMLFile *self, const char iterAction){
	int rv=0;
	if (iterAction=='u'){
		rv--;
	} else if (iterAction=='d'){
		rv++;
	}
	return rv;
}

static String* XMLFile_GetLineIndenting(XMLFile *self, const int indentLevel){
	String *rv=StringClass.new();
	String *indentStr=StringClass.new();
	indentStr->methods->set(indentStr,"\t");
	for (int i=0; i<indentLevel; i++){
		rv->methods->concat(rv,indentStr->str);
	}
	StringClass.delete(&indentStr);
	return rv;
}

static String* XMLFile_GenerateNextLine(XMLFile *self, const XMLTag *tag, const char iterAction, const int indent){
	String *line=XMLFile_GetLineIndenting(self,indent);
	String *openingTag=tag->methods->getRawOpeningTag(tag);
	String *closingTag=tag->methods->getRawClosingTag(tag);
	if (iterAction=='u'){
		line->methods->concat(line,tag->data->str);
		line->methods->concat(line,closingTag->str);
	} else if (strchr("dn",iterAction)!=NULL &&
		   self->methods->getNumChildTags(self)>0){
		line->methods->concat(line,openingTag->str);
	} else if (strchr("dn",iterAction)!=NULL &&
		   self->methods->getNumChildTags(self)==0){
		line->methods->concat(line,openingTag->str);
		line->methods->concat(line,tag->data->str);
		line->methods->concat(line,closingTag->str);
	}
	line->methods->concatChar(line,'\n');
	StringClass.delete(&openingTag);
	StringClass.delete(&closingTag);
	return line;
}

static void XMLFile_ConvertFromGTToXMLFile(XMLFile *self){
	self->numTags=self->gt->numNodes;
}

//Class Methods================================================================
static XMLFile* _XMLFileConstructor(void){
	XMLFile *rv=(XMLFile*)createObject(XMLFileClass.objSize);
	rv->ownsData=true;
	rv->numTags=0;
	rv->source=FileClass.new();
	rv->gt=GenericTreeClass.new();
	rv->gt->methods->setElementSize(rv->gt,sizeof(XMLTag));
	rv->methods=&xmlFileMethods;
	return rv;
}

static void _XMLFilePrintObj(XMLFile *obj){
	PrintClass.print("<XMLFile Obj[Addr: %p]>\n",obj);
	PrintClass.incrementIndentLevel(1);
	FileClass.print(obj->source);
	GenericTreeClass.print(obj->gt);
	PrintClass.incrementIndentLevel(-1);
}

static bool _XMLFileDebug(void){
	bool successfull=true;
	String *path=StringClass.new();
	path->methods->concatChar(path,'.');
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"debug");
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"XMLFileClassDebug.xml");
	successfull&=_XMLFileDebugParser(path);
	successfull&=_XMLFileDebugWrite(path);
	successfull&=_XMLFileDebugAdd(path);
	successfull&=_XMLFileDebugRemove(path);
	successfull&=_XMLFileDebugSet(path);
	successfull&=_XMLFileDebugSearch(path);
	successfull&=_XMLFileDebugMisc(path);
	successfull&=_XMLFileDebugCopy(path);
	StringClass.delete(&path);
	PrintClass.objectDebug("XMLFile",successfull);
	return successfull;
}

static bool _XMLFileDebugParser(const String *path){
	bool successfull=true;
	XMLFile *test=XMLFileClass.new();
	test->methods->setSource(test,path);
	(test->numTags!=16)? successfull=false: 0;
	XMLTag *node=test->methods->get(test);
	(!node->name->methods->equals(node->name,"root"))? successfull=false: 0;
	(!test->methods->moveDown(test))? successfull=false: 0;
	node=test->methods->get(test);
	(!node->name->methods->equals(node->name,"ProgramSettings"))? successfull=false: 0;
	(!test->methods->moveNext(test))? successfull=false: 0;
	node=test->methods->get(test);
	(!node->name->methods->equals(node->name,"LanguageSettings"))? successfull=false: 0;
	(!test->methods->moveUp(test))? successfull=false: 0;
	node=test->methods->get(test);
	(!node->name->methods->equals(node->name,"root"))? successfull=false: 0;
	//XMLFileClass.print(test);
	XMLFileClass.delete(&test);
	return successfull;
}

static bool _XMLFileDebugWrite(const String *path){
	bool successfull=true;
	XMLFile *test=XMLFileClass.new();
	XMLFile *test2=XMLFileClass.new();
	test->methods->setSource(test,path);
	test->methods->write(test);
	test2->methods->setSource(test2,path);
	(!test->methods->equals(test,test2))? successfull=false: 0;
	//XMLFileClass.print(test);
	//XMLFileClass.print(test2);
	XMLFileClass.delete(&test2);
	XMLFileClass.delete(&test);
	return successfull;
}

static bool _XMLFileDebugAdd(const String *path){
	bool successfull=true;
	StringList *branch=StringListClass.new();
	branch->ownsData=true;
	String *branches[3]={ NULL,NULL,NULL };
	for (int i=0; i<3; i++){
		branches[i]=StringClass.new();
	}
	branches[0]->methods->set(branches[0],"ProgramSettings");
	branches[1]->methods->set(branches[1],"testing");
	branches[2]->methods->set(branches[2],"testing");
	branch->methods->add(branch,(const String**)branches,2);
	String *name=StringClass.new();
	name->methods->set(name,"testing");
	XMLTag *tag1=XMLTagClass.new();
	XMLTag *tag2=XMLTagClass.new();
	tag1->methods->setName(tag1,name);
	tag2->methods->setName(tag2,name);
	XMLFile *test=XMLFileClass.new();
	test->methods->setSource(test,path);
	test->methods->moveDown(test);
	test->methods->add(test,tag1);
	(!test->methods->placeCurrentNode(test,branch))? successfull=false: 0;
	test->methods->addAt(test,branch,tag2);
	branch->methods->add(branch,(const String**)&branches[2],1);
	(!test->methods->placeCurrentNode(test,branch))? successfull=false: 0;
	//XMLFileClass.print(test);
	StringClass.delete(&name);
	StringListClass.delete(&branch);
	XMLFileClass.delete(&test);
	return successfull;
}

static bool _XMLFileDebugRemove(const String *path){
	bool successfull=true;
	StringList *branch=StringListClass.new();
	branch->ownsData=true;
	String *branches[3]={ NULL,NULL,NULL };
	for (int i=0; i<3; i++){
		branches[i]=StringClass.new();
	}
	branches[0]->methods->set(branches[0],"LanguageSettings");
	branches[1]->methods->set(branches[1],"LangDef");
	branches[2]->methods->set(branches[2],"BuildFile");
	branch->methods->add(branch,(const String**)branches,3);
	XMLFile *test=XMLFileClass.new();
	test->methods->setSource(test,path);
	test->methods->moveDown(test);
	test->methods->moveNext(test);
	test->methods->remove(test);
	(test->numTags!=3)? successfull=false: 0;
	(test->methods->placeCurrentNode(test,branch))? successfull=false: 0;
	//XMLFileClass.print(test);
	StringListClass.delete(&branch);
	XMLFileClass.delete(&test);
	return successfull;
}

static bool _XMLFileDebugSet(const String *path){
	bool successfull=true;

	StringList *branch=StringListClass.new();
	branch->ownsData=true;
	String *branches[3]={ NULL,NULL,NULL };
	for (int i=0; i<3; i++){
		branches[i]=StringClass.new();
	}
	branches[0]->methods->set(branches[0],"testing");
	branches[1]->methods->set(branches[1],"LangDef");
	branches[2]->methods->set(branches[2],"BuildFile");
	branch->methods->add(branch,(const String**)branches,3);

	String *name=StringClass.new();
	name->methods->set(name,"testing");
	XMLTag *tag=XMLTagClass.new();
	XMLTag *tag2=XMLTagClass.new();
	tag->methods->setName(tag,name);
	tag2->methods->setName(tag2,name);

	XMLFile *test=XMLFileClass.new();
	test->methods->setSource(test,path);
	test->methods->moveDown(test);
	test->methods->moveNext(test);
	test->methods->set(test,tag);
	XMLTag *temp=test->methods->get(test);
	(!temp->name->methods->equals(temp->name,name->str))? successfull=false: 0;
	test->methods->setAt(test,branch,tag2);
	branches[2]->methods->set(branches[2],tag2->name->str);
	(!test->methods->placeCurrentNode(test,branch))? successfull=false: 0;
	//XMLFileClass.print(test);
	StringClass.delete(&name);
	StringListClass.delete(&branch);
	XMLFileClass.delete(&test);
	return successfull;
}

static bool _XMLFileDebugSearch(const String *path){
	bool successfull=true;
	StringList *branch=StringListClass.new();
	branch->ownsData=true;
	String *branches[3]={ NULL,NULL,NULL };
	for (int i=0; i<3; i++){
		branches[i]=StringClass.new();
	}
	branches[0]->methods->set(branches[0],"LanguageSettings");
	branches[1]->methods->set(branches[1],"LangDef");
	branches[2]->methods->set(branches[2],"BuildFile");
	branch->methods->add(branch,(const String**)branches,3);
	//StringListClass.print(branch);
	XMLFile *test=XMLFileClass.new();
	test->methods->setSource(test,path);
	XMLTag *temp=test->methods->get(test);
	(!temp->name->methods->equals(temp->name,"root"))? successfull=false: 0;
	temp=test->methods->getAt(test,branch);
	(!temp->name->methods->equals(temp->name,branches[2]->str))? successfull=false: 0;
	(test->methods->getNumChildTags(test)!=2)? successfull=false: 0;
	(test->methods->getNumChildTagsAt(test,branch)!=0)? successfull=false: 0;
	(!test->methods->branchExists(test,branch))? successfull=false: 0;
	(!test->methods->placeCurrentNode(test,branch))? successfull=false: 0;
	branches[0]->methods->set(branches[0],"ProgramSettings");
	//StringListClass.print(branch);
	(test->methods->placeCurrentNode(test,branch))? successfull=false: 0;
	//XMLFileClass.print(test);
	StringListClass.delete(&branch);
	XMLFileClass.delete(&test);
	return successfull;
}

static bool _XMLFileDebugMisc(const String *path){
	bool successfull=true;
	int cntr=0;
	XMLFile *test1=XMLFileClass.new();
	XMLFile *test2=XMLFileClass.new();
	(!test1->methods->isEmpty(test1))? successfull=false: 0;
	test1->methods->setSource(test1,path);
	test2->methods->setSource(test2,path);
	(!test1->methods->equals(test1,test2))? successfull=false: 0;
	XMLTag *temp=test1->methods->get(test1);
	temp->name->methods->set(temp->name,"testing");
	(test1->methods->equals(test1,test2))? successfull=false: 0;
	(test1->methods->isEmpty(test1))? successfull=false: 0;
	if (test1->methods->startIter(test1)){
		while (test1->methods->hasNext(test1)){
			test1->methods->getNext(test1);
			if (test1->methods->getIterAction(test1)!='u'){
				cntr++;
			}
		}
	}
	(test1->numTags!=cntr)? successfull=false: 0;
	//XMLFileClass.print(test1);
	//XMLFileClass.print(test2);
	XMLFileClass.delete(&test1);
	XMLFileClass.delete(&test2);
	return successfull;
}

static bool _XMLFileDebugCopy(const String *path){
	bool successfull=true;
	XMLFile *test=XMLFileClass.new();
	XMLFile *test2=XMLFileClass.new();
	test->methods->setSource(test,path);
	test2->methods->copyOtherBelowCurNode(test2,test);
	test2->ownsData=false;
	(!test->methods->equals(test,test2))? successfull=false: 0;
	//XMLFileClass.print(test);
	//XMLFileClass.print(test2);
	XMLFileClass.delete(&test);
	XMLFileClass.delete(&test2);
	return successfull;
}

static void _XMLFileDestructor(XMLFile **obj){
	(*obj)->methods->clear(*obj);
	FileClass.delete(&(*obj)->source);
	GenericTreeClass.delete(&(*obj)->gt);
	deleteObject((void**)obj);
}
