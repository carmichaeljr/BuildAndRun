#include <stdbool.h>
#include "Arguments.h"
#include "Object.h"
#include "Print.h"
#include "File.h"
#include "StringList.h"
#include "SettingsFile.h"

static ProgramArgs* _ProgramArgsConstructor(void);
static void _ProgramArgsPrintObj(ProgramArgs *obj);
static bool _ProgramArgsDebug(void);
static bool _ProgramArgsDebugGeneral(const char **data);
static bool _ProgramArgsDebugValid(const char **data);
static bool _ProgramArgsDebugOpReq(const char **data);
static bool _ProgramArgsDebugGetSecondArgAsFile(const char **data);
static void _ProgramArgsDestructor(ProgramArgs **obj);
static void ProgramArgs_Set(ProgramArgs *self, const char **argv, const int argc);
static int ProgramArgs_GetNumArgs(const ProgramArgs *self);
static const String* ProgramArgs_Get(const ProgramArgs *self, const int index);
static bool ProgramArgs_AreValid(const ProgramArgs *self);
static bool ProgramArgs_OperationRequested(const ProgramArgs *self, const char operation);
static File* ProgramArgs_GetSecondArgAsFile(const ProgramArgs *self);
static bool ProgramArgs_FirstArgValid(const ProgramArgs *self);
static bool ProgramArgs_SecondArgNeeded(const ProgramArgs *self);
static bool ProgramArgs_SecondArgValid(const ProgramArgs *self);

static const struct ProgramArgsMethods programArgsMethods={
	.set=&ProgramArgs_Set,
	.get=&ProgramArgs_Get,
	.getNumArgs=&ProgramArgs_GetNumArgs,
	.areValid=&ProgramArgs_AreValid,
	.operationRequested=&ProgramArgs_OperationRequested,
	.getSecondArgAsFile=&ProgramArgs_GetSecondArgAsFile,
};
const struct ProgramArgsClass_t ProgramArgsClass={
	.new=&_ProgramArgsConstructor,
	.delete=&_ProgramArgsDestructor,
	.print=&_ProgramArgsPrintObj,
	.debug=&_ProgramArgsDebug,
	.objSize=sizeof(ProgramArgs),
};

//Object Methods===============================================================
//Public Methods
static void ProgramArgs_Set(ProgramArgs *self, const char **argv, const int argc){
	self->args->methods->clear(self->args);
	(argc>0)? self->progName->methods->set(self->progName,argv[0]): 0;
	for (int i=1; i<argc; i++){
		String *iterStr=StringClass.new();
		iterStr->methods->set(iterStr,argv[i]);
		self->args->methods->add(self->args,(const String**)&iterStr,1);
	}
}

static int ProgramArgs_GetNumArgs(const ProgramArgs *self){
	return self->args->length;
}

static const String* ProgramArgs_Get(const ProgramArgs *self, const int index){
	String *rv=self->args->methods->get(self->args,index);
	return (const String*)rv;
}

static bool ProgramArgs_AreValid(const ProgramArgs *self){
	bool rv=(self->args->length>0 && self->args->length<3);
	rv&=(rv)? ProgramArgs_FirstArgValid(self): false;
	rv&=(rv && ProgramArgs_SecondArgNeeded(self))? ProgramArgs_SecondArgValid(self): true;
	return rv;
}

static bool ProgramArgs_OperationRequested(const ProgramArgs *self, const char operation){
	const String *firstArg=self->methods->get(self,0);
	int index=firstArg->methods->getFirstIndexOf(firstArg,operation);
	return (index>=0);
}

static File* ProgramArgs_GetSecondArgAsFile(const ProgramArgs *self){
	File *rv=(self->args->length>=2)? FileClass.new(): NULL;
	if (rv!=NULL){
		rv->methods->setPath(rv,self->methods->get(self,1));
	}
	return rv;
}

//Private Methods
static bool ProgramArgs_FirstArgValid(const ProgramArgs *self){
	bool rv=true;
	const String *firstArg=self->methods->get(self,0);
	SettingsFile *temp=SettingsFileClass.new();
	const StringList *firstArgs=temp->methods->getAcceptedFirstArgs(temp);
	rv=(firstArgs->methods->getFirstIndexOf(firstArgs,firstArg)>=0)? true: false;
	SettingsFileClass.delete(&temp);
	return rv;
}

static bool ProgramArgs_SecondArgNeeded(const ProgramArgs *self){
	const String *firstArg=self->methods->get(self,0);
	bool rv=!(firstArg->methods->equals(firstArg,"s") || firstArg->methods->equals(firstArg,"d"));
	return rv;
}

static bool ProgramArgs_SecondArgValid(const ProgramArgs *self){
	bool rv=(self->args->length>=2);
	if (rv){
		File *file=FileClass.new();
		file->methods->setPath(file,self->methods->get(self,1));
		rv&=FileClass.fileExists(file);
		FileClass.delete(&file);
	}
	return rv;
}

//Class Methods================================================================
static ProgramArgs* _ProgramArgsConstructor(void){
	ProgramArgs *rv=(ProgramArgs*)createObject(ProgramArgsClass.objSize);
	rv->progName=StringClass.new();
	rv->args=StringListClass.new();
	rv->args->ownsData=true;
	rv->methods=&programArgsMethods;
	return rv;
}

static void _ProgramArgsPrintObj(ProgramArgs *obj){
	PrintClass.print("<ProgramArgs Obj[Addr: %p]: argc: %d>\n",obj,obj->args->length);
	PrintClass.incrementIndentLevel(1);
	StringClass.print(obj->progName);
	StringListClass.print(obj->args);
	PrintClass.incrementIndentLevel(-1);
}

static bool _ProgramArgsDebug(void){
	bool successfull=true;
	String *path=StringClass.new();
	path->methods->concatChar(path,'.');
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"debug");
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"FileClassDebug.txt");
	char *data[3]={ "progName","cel",path->str };
	successfull&=_ProgramArgsDebugGeneral((const char**)data);
	successfull&=_ProgramArgsDebugValid((const char**)data);
	successfull&=_ProgramArgsDebugOpReq((const char**)data);
	successfull&=_ProgramArgsDebugGetSecondArgAsFile((const char**)data);
	StringClass.delete(&path);
	PrintClass.objectDebug("ProgramArgs",successfull);
	return successfull;
}

static bool _ProgramArgsDebugGeneral(const char **data){
	bool successfull=true;
	ProgramArgs *test=ProgramArgsClass.new();
	test->methods->set(test,data,3);
	(!test->progName->methods->equals(test->progName,data[0]))? successfull=false: 0;
	const String *temp=test->methods->get(test,0);
	(!temp->methods->equals(temp,data[1]))? successfull=false: 0;
	temp=test->methods->get(test,1);
	(!temp->methods->equals(temp,data[2]))? successfull=false: 0;
	//ProgramArgsClass.print(test);
	ProgramArgsClass.delete(&test);
	return successfull;
}

static bool _ProgramArgsDebugValid(const char **data){
	bool successfull=true;
	ProgramArgs *test=ProgramArgsClass.new();
	test->methods->set(test,data,3);
	(!test->methods->areValid(test))? successfull=false: 0;
	data[1]="cew";
	test->methods->set(test,data,3);
	(test->methods->areValid(test))? successfull=false: 0;
	data[1]="cel";
	//ProgramArgsClass.print(test);
	ProgramArgsClass.delete(&test);
	return successfull;
}

static bool _ProgramArgsDebugOpReq(const char **data){
	bool successfull=true;
	ProgramArgs *test=ProgramArgsClass.new();
	test->methods->set(test,data,3);
	(!test->methods->operationRequested(test,'l'))? successfull=false: 0;
	(!test->methods->operationRequested(test,'c'))? successfull=false: 0;
	(!test->methods->operationRequested(test,'e'))? successfull=false: 0;
	(test->methods->operationRequested(test,'a'))? successfull=false: 0;
	//ProgramArgsClass.print(test);
	ProgramArgsClass.delete(&test);
	return successfull;
}

static bool _ProgramArgsDebugGetSecondArgAsFile(const char **data){
	bool successfull=true;
	String *path=StringClass.new();
	path->methods->concatChar(path,'.');
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"debug");
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"FileClassDebug.txt");
	File *validation=FileClass.new();
	validation->methods->setPath(validation,path);
	ProgramArgs *test=ProgramArgsClass.new();
	test->methods->set(test,data,3);
	File *secondArgFile=test->methods->getSecondArgAsFile(test);
	(!validation->methods->equals(validation,secondArgFile))? successfull=false: 0;
	ProgramArgsClass.delete(&test);
	FileClass.delete(&secondArgFile);
	FileClass.delete(&validation);
	StringClass.delete(&path);
	return successfull;
}

static void _ProgramArgsDestructor(ProgramArgs **obj){
	StringClass.delete(&(*obj)->progName);
	StringListClass.delete(&(*obj)->args);
	deleteObject((void**)obj);
}
