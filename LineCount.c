#include <stdbool.h>
#include "LineCount.h"
#include "Object.h"
#include "Print.h"
#include "Arguments.h"
#include "StringList.h"
#include "BuildFile.h"
//#include "File.h"
//#include "Arguments.h"
//#include "BuildFile.h"

static LineCount* _LineCountConstructor(void);
static void _LineCountPrintObj(LineCount *obj);
static bool _LineCountDebug(void);
static bool _LineCountDebugSet(BuildFile *buildFile);
static void _LineCountDestructor(LineCount **obj);
static void LineCount_SetLineCountList(LineCount *self, BuildFile *buildFile);
static void LineCount_RunLineCount(LineCount *self);

static const struct LineCountMethods lineCountMethods={
	.setLineCountList=&LineCount_SetLineCountList,
	.runLineCount=&LineCount_RunLineCount,
};
const struct LineCountClass_t LineCountClass={
	.new=&_LineCountConstructor,
	.delete=&_LineCountDestructor,
	.print=&_LineCountPrintObj,
	.debug=&_LineCountDebug,
	.objSize=sizeof(LineCount),
};

//Object Methods===============================================================
//Public methods
static void LineCount_SetLineCountList(LineCount *self, BuildFile *buildFile){
	StringListClass.delete(&self->allFiles);
	self->allFiles=buildFile->methods->getLineCountList(buildFile);
}

static void LineCount_RunLineCount(LineCount *self){
	PrintClass.print("\nLine Count Process Started >>>\n");
	self->numLines=0;
	for (int i=0; i<self->allFiles->length; i++){
		String *iterStr=self->allFiles->methods->get(self->allFiles,i);
		StringList *possibleFiles=FileClass.getFilesInDir(iterStr);
		//StringListClass.print(possibleFiles);
		for (int i=0; i<possibleFiles->length; i++){
			String *iterFilePath=possibleFiles->methods->get(possibleFiles,i);
			File *iterFile=FileClass.new();
			iterFile->methods->setPath(iterFile,iterFilePath);
			int numLines=iterFile->methods->getNumLines(iterFile);
			PrintClass.print("File: '%-40s'Lines: %d\n",iterFile->name->str,numLines);
			self->numLines+=numLines;
			FileClass.delete(&iterFile);
		}
		StringListClass.delete(&possibleFiles);
	}
	PrintClass.print("Total Lines: %d\n",self->numLines);
	PrintClass.print("<<< Line Count Process ended\n");
}

//Private methods
//Class Methods================================================================
static LineCount* _LineCountConstructor(void){
	LineCount *rv=createObject(LineCountClass.objSize);
	rv->numLines=0;
	rv->allFiles=StringListClass.new();
	rv->allFiles->ownsData=true;
	rv->methods=&lineCountMethods;
	return rv;
}

static void _LineCountPrintObj(LineCount *obj){
	PrintClass.print("<LineCount Obj[Addr: %p]: numLines: %d>\n",obj,obj->numLines);
	PrintClass.incrementIndentLevel(1);
	StringListClass.print(obj->allFiles);
	PrintClass.incrementIndentLevel(-1);
}

static bool _LineCountDebug(void){
	bool successfull=true;
	String *path=StringClass.new();
	path->methods->concatChar(path,'.');
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"debug");
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"LineCountDebug.xml");
	char *data[3]={ "progName","cel",path->str };
	ProgramArgs *progArgs=ProgramArgsClass.new();
	progArgs->methods->set(progArgs,(const char**)data,3);
	File *mainFile=progArgs->methods->getSecondArgAsFile(progArgs);
	BuildFile *buildFile=BuildFileClass.new();
	buildFile->methods->setSource(buildFile,mainFile);
	successfull&=_LineCountDebugSet(buildFile);
	ProgramArgsClass.delete(&progArgs);
	BuildFileClass.delete(&buildFile);
	FileClass.delete(&mainFile);
	PrintClass.objectDebug("LineCount",successfull);
	return successfull;
}

static bool _LineCountDebugSet(BuildFile *buildFile){
	bool successfull=true;
	LineCount *test=LineCountClass.new();
	test->methods->setLineCountList(test,buildFile);
	(test->allFiles->length!=1)? successfull=false: 0;
	String *firstFile=test->allFiles->methods->get(test->allFiles,0);
	(!firstFile->methods->equals(firstFile,"./debug/LineCountTest.txt"))? successfull=false: 0;
	test->methods->runLineCount(test);
	(test->numLines!=10)? successfull=false: 0;
	//LineCountClass.print(test);
	LineCountClass.delete(&test);
	return successfull;
}

static void _LineCountDestructor(LineCount **obj){
	StringListClass.delete(&(*obj)->allFiles);
	deleteObject((void**)obj);
}
