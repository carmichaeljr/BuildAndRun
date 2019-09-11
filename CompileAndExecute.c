#include <stdbool.h>
#include <process.h>
//#include <Windows.h>
#include "CompileAndExecute.h"
#include "Object.h"
#include "Print.h"
#include "File.h"
#include "Arguments.h"
#include "BuildFile.h"

static CompileAndExecute* _CompileAndExecuteConstructor(void);
static void _CompileAndExecutePrintObj(CompileAndExecute *obj);
static bool _CompileAndExecuteDebug(void);
static bool _CompileAndExecuteDebugGetCommands(BuildFile *buildFile, const ProgramArgs *progArgs);
static bool _CompileAndExecuteDebugRunCommands(BuildFile *buildFile, const ProgramArgs *progArgs);
static void _CompileAndExecuteDestructor(CompileAndExecute **obj);
static void CAE_SetCommands(CompileAndExecute *self, BuildFile *buildFile, const ProgramArgs *args);
static void CAE_RunCommands(CompileAndExecute *self);
static void CAE_PadCmdWithCwd(CompileAndExecute *self);
static void CAE_PrintProcessSuccess(const CompileAndExecute *self);

static const struct CompileAndExecuteMethods compileAndExecuteMethods={
	.setCommands=&CAE_SetCommands,
	.runCommands=&CAE_RunCommands,
};
const struct CompileAndExecuteClass_t CompileAndExecuteClass={
	.new=&_CompileAndExecuteConstructor,
	.delete=&_CompileAndExecuteDestructor,
	.print=&_CompileAndExecutePrintObj,
	.debug=&_CompileAndExecuteDebug,
	.objSize=sizeof(CompileAndExecute),
};

//Object Methods================================================================
//Public Methods
static void CAE_SetCommands(CompileAndExecute *self, BuildFile *buildFile, const ProgramArgs *args){
	String *cmdType=StringClass.new();
	if (args->methods->operationRequested(args,'c')){
		cmdType->methods->set(cmdType,"CompileCmd");
		StringClass.delete(&self->compileCmd);
		self->compileCmd=buildFile->methods->getCommand(buildFile,cmdType);
	}
	if (args->methods->operationRequested(args,'e')){
		cmdType->methods->set(cmdType,"ExecuteCmd");
		StringClass.delete(&self->executeCmd);
		self->executeCmd=buildFile->methods->getCommand(buildFile,cmdType);
	}
	CAE_PadCmdWithCwd(self);
	StringClass.delete(&cmdType);
}

static void CAE_RunCommands(CompileAndExecute *self){
	if (self->compileCmd->length>0){
		PrintClass.print("Compile process started >>>\n\n");
		self->retVal=system(self->compileCmd->str);
		PrintClass.print("\n<<< Compile process ended\n");
		CAE_PrintProcessSuccess(self);
	}
	if (self->executeCmd->length>0 && self->retVal!=0){
		self->retVal=(PrintClass.yesOrNoPrompt(
			      "Compilation failed, do you still wish to execute"))?
			      0: self->retVal;
	}
	if (self->executeCmd->length>0 && self->retVal==0){
		PrintClass.print("Execution process started >>>\n\n");
		self->retVal=system(self->executeCmd->str);
		PrintClass.print("\n<<< Execution process ended\n");
		CAE_PrintProcessSuccess(self);
	}
}

//Private Methods
static void CAE_PadCmdWithCwd(CompileAndExecute *self){
	String *tempBuffer=StringClass.new();
	String *cwd=FileClass.getCwd();
	String *cdCmd=StringClass.new();
	cdCmd->methods->concat(cdCmd,"cd /D \"");
	cdCmd->methods->concat(cdCmd,cwd->str);
	cdCmd->methods->concat(cdCmd,"\" && ");
	if (self->compileCmd->length>0){
		tempBuffer->methods->set(tempBuffer,self->compileCmd->str);
		self->compileCmd->methods->set(self->compileCmd,cdCmd->str);
		self->compileCmd->methods->concat(self->compileCmd,tempBuffer->str);
	}
	if (self->executeCmd->length>0){
		tempBuffer->methods->set(tempBuffer,self->executeCmd->str);
		self->executeCmd->methods->set(self->executeCmd,cdCmd->str);
		self->executeCmd->methods->concat(self->executeCmd,tempBuffer->str);
	}
	StringClass.delete(&tempBuffer);
	StringClass.delete(&cwd);
	StringClass.delete(&cdCmd);
}

static void CAE_PrintProcessSuccess(const CompileAndExecute *self){
	PrintClass.incrementIndentLevel(1);
	PrintClass.print("Process was: ");
	if (self->retVal==0){
		PrintClass.printWithoutIndent("SUCCESSFUL\n");
	} else if (self->retVal>0){
		PrintClass.printWithoutIndent("UNSUCCESSFUL\n");
		PrintClass.print("Error code: %d\n",self->retVal);
	} else {
		PrintClass.printWithoutIndent("NOT RUN\n");
		PrintClass.incrementIndentLevel(1);
		PrintClass.print("Please make sure the file you specified exists.");
		PrintClass.print("Error code: %d\n",self->retVal);
		PrintClass.incrementIndentLevel(-1);
	}
	PrintClass.incrementIndentLevel(-1);
}

//Class Methods================================================================
static CompileAndExecute* _CompileAndExecuteConstructor(void){
	CompileAndExecute *rv=(CompileAndExecute*)createObject(CompileAndExecuteClass.objSize);
	rv->retVal=0;
	rv->compileCmd=StringClass.new();
	rv->executeCmd=StringClass.new();
	rv->methods=&compileAndExecuteMethods;
	return rv;
}

static void _CompileAndExecutePrintObj(CompileAndExecute *obj){
	PrintClass.print("<CompileAndExecute Obj[Addr: %p]: retVal: %d>\n",obj,obj->retVal);
	PrintClass.incrementIndentLevel(1);
	StringClass.print(obj->compileCmd);
	StringClass.print(obj->executeCmd);
	PrintClass.incrementIndentLevel(-1);
}

static bool _CompileAndExecuteDebug(void){
	bool successfull=true;
	String *path=StringClass.new();
	path->methods->concatChar(path,'.');
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"debug");
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"CompileAndExecuteDebug.xml");
	char *data[3]={ "progName","cel",path->str };
	ProgramArgs *progArgs=ProgramArgsClass.new();
	progArgs->methods->set(progArgs,(const char**)data,3);
	File *mainFile=progArgs->methods->getSecondArgAsFile(progArgs);
	BuildFile *buildFile=BuildFileClass.new();
	buildFile->methods->setSource(buildFile,mainFile);
	successfull&=_CompileAndExecuteDebugGetCommands(buildFile,progArgs);
	//successfull&=_CompileAndExecuteDebugRunCommands(buildFile,progArgs);
	ProgramArgsClass.delete(&progArgs);
	BuildFileClass.delete(&buildFile);
	FileClass.delete(&mainFile);
	PrintClass.objectDebug("CompileAndExecute",successfull);
	return successfull;
}

static bool _CompileAndExecuteDebugGetCommands(BuildFile *buildFile, const ProgramArgs *progArgs){
	bool successfull=true;
	String *cwd=FileClass.getCwd();
	String *cmd=StringClass.new();
	cmd->methods->concat(cmd,"cd /D \"");
	cmd->methods->concat(cmd,cwd->str);
	cmd->methods->concat(cmd,"\" && ");
	cmd->methods->concat(cmd,"\"C:\\Windows\\notepad.exe\" \"./debug/ProgramSettingsDebug.xml\" ");
	CompileAndExecute *test=CompileAndExecuteClass.new();
	test->methods->setCommands(test,buildFile,progArgs);
	(!cmd->methods->equals(cmd,test->compileCmd->str))? successfull=false: 0;
	(!cmd->methods->equals(cmd,test->executeCmd->str))? successfull=false: 0;
	//CompileAndExecuteClass.print(test);
	CompileAndExecuteClass.delete(&test);
	StringClass.delete(&cmd);
	StringClass.delete(&cwd);
	return successfull;
}

static bool _CompileAndExecuteDebugRunCommands(BuildFile *buildFile, const ProgramArgs *progArgs){
	bool successfull=true;
	CompileAndExecute *test=CompileAndExecuteClass.new();
	test->methods->setCommands(test,buildFile,progArgs);
	test->methods->runCommands(test);
	(test->retVal!=0)? successfull=false: 0;
	//CompileAndExecuteClass.print(test);
	CompileAndExecuteClass.delete(&test);
	return successfull;
}

static void _CompileAndExecuteDestructor(CompileAndExecute **obj){
	StringClass.delete(&(*obj)->compileCmd);
	StringClass.delete(&(*obj)->executeCmd);
	deleteObject((void**)obj);
}
