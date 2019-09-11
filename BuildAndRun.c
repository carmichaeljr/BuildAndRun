//#include "Math.h"
#include "Print.h"
#include "File.h"
#include "GenericList.h"
#include "GenericLinkedList.h"
#include "GenericDictionary.h"
#include "GenericTree.h"
#include "IntList.h"
#include "String.h"
#include "StringList.h"
#include "StrStrDictionary.h"
#include "XMLTag.h"
#include "XMLFile.h"
#include "SettingsFile.h"
#include "BuildFile.h"
#include "Arguments.h"
#include "CompileAndExecute.h"
#include "LineCount.h"

void debug(void);
void openSettings(void);
void runCE(ProgramArgs *args);
void runL(ProgramArgs *args);

int main(int argc, char **argv){
	ProgramArgs *args=ProgramArgsClass.new();
	args->methods->set(args,(const char**)argv,argc);
	if (args->methods->areValid(args)){
		if (args->methods->operationRequested(args,'d')){
			debug();
		} else if (args->methods->operationRequested(args,'s')){
			openSettings();
		}
		if (args->methods->operationRequested(args,'c') ||
		    args->methods->operationRequested(args,'e')){
			runCE(args);
		}
		if (args->methods->operationRequested(args,'l')){
			runL(args);
		}
	} else {
		PrintClass.printError("The given arguments were not correct.\n");
		PrintClass.incrementIndentLevel(1);
		PrintClass.print("Accepted arguments:\n");
		PrintClass.incrementIndentLevel(1);
		PrintClass.print("./BuildAndRun [s,d]\n");
		PrintClass.print("./BuildAndRun [l,c,e,ce,cel] [mainFile]\n");
		PrintClass.print("s: open settings\n");
		PrintClass.print("d: run debug\n");
		PrintClass.print("c: compile\n");
		PrintClass.print("e: execute, and the shown combinations of the same commands\n");
		PrintClass.print("Where mainFile is the name of a file in the CURRENT directory\n");
		PrintClass.incrementIndentLevel(-2);
	}
	ProgramArgsClass.delete(&args);
	return 0;
}

void debug(void){
	bool successfull=true;
	//MathClass.debug();
	successfull&=FileClass.debug();
	successfull&=GenericListClass.debug();
	successfull&=GenericLinkedListClass.debug();
	successfull&=GenericDictionaryClass.debug();
	successfull&=GenericTreeClass.debug();
	successfull&=IntListClass.debug();
	successfull&=StringClass.debug();
	successfull&=StringListClass.debug();
	successfull&=StrStrDictionaryClass.debug();
	successfull&=XMLTagClass.debug();
	successfull&=XMLFileClass.debug();

	successfull&=SettingsFileClass.debug();
	successfull&=BuildFileClass.debug();
	successfull&=ProgramArgsClass.debug();
	successfull&=CompileAndExecuteClass.debug();
	successfull&=LineCountClass.debug();
	PrintClass.print("Overall debug: %s\n",(successfull)? "OK": "ERROR");
}

void openSettings(void){
	PrintClass.print("Opening settings file for editing...\n");
	SettingsFile *settings=SettingsFileClass.new();
	settings->methods->openForUser(settings);
	SettingsFileClass.delete(&settings);
}

void runCE(ProgramArgs *args){
	bool _continue=true;
	File *mainFile=args->methods->getSecondArgAsFile(args);
	BuildFile *buildFile=BuildFileClass.new();
	SettingsFile *settingsFile=SettingsFileClass.new();
	if (BuildFileClass.buildFileExists(mainFile->absPath)){
		buildFile->methods->setSource(buildFile,mainFile);
	} else if (settingsFile->methods->langDefExists(settingsFile,mainFile->extension)){
		buildFile->methods->generateBuildFile(buildFile,mainFile);
	} else {
		PrintClass.printError("The type of file given does not have an entry in the settings file.\n");
		PrintClass.incrementIndentLevel(1);
		PrintClass.print("To fix this run the program again with the 's' flag,\n");
		PrintClass.print("and add the file type to the settings file\n");
		PrintClass.incrementIndentLevel(-1);
		_continue=false;
	}
	if (_continue){
		CompileAndExecute *cae=CompileAndExecuteClass.new();
		cae->methods->setCommands(cae,buildFile,args);
		cae->methods->runCommands(cae);
		CompileAndExecuteClass.delete(&cae);
	}
	SettingsFileClass.delete(&settingsFile);
	BuildFileClass.delete(&buildFile);
	FileClass.delete(&mainFile);
}

void runL(ProgramArgs *args){
	bool _continue=true;
	File *mainFile=args->methods->getSecondArgAsFile(args);
	BuildFile *buildFile=BuildFileClass.new();
	SettingsFile *settingsFile=SettingsFileClass.new();
	if (BuildFileClass.buildFileExists(mainFile->absPath)){
		buildFile->methods->setSource(buildFile,mainFile);
	} else if (settingsFile->methods->langDefExists(settingsFile,mainFile->extension)){
		buildFile->methods->generateBuildFile(buildFile,mainFile);
	} else {
		PrintClass.printError("The type of file given does not have an entry in the settings file.\n");
		PrintClass.incrementIndentLevel(1);
		PrintClass.print("To fix this run the program again with the 's' flag,\n");
		PrintClass.print("and add the file type to the settings file\n");
		PrintClass.incrementIndentLevel(-1);
		_continue=false;
	}
	if (_continue){
		LineCount *lineCount=LineCountClass.new();
		lineCount->methods->setLineCountList(lineCount,buildFile);
		lineCount->methods->runLineCount(lineCount);
		LineCountClass.delete(&lineCount);
	}
	SettingsFileClass.delete(&settingsFile);
	BuildFileClass.delete(&buildFile);
	FileClass.delete(&mainFile);
}
