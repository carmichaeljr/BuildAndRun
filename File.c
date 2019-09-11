#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include "File.h"
#include "Object.h"
#include "Print.h"
#include "String.h"
#include "StringList.h"
#if OS==WINDOWS
	#include <windows.h>
#endif

static File* _FileConstructor(void);
static void _FilePrintObj(File *obj);
static bool _FileExists(const File *obj);
static String* _FileGetCwd(void);
static String* _FileGetPathFromList(const StringList *dirs);
static String* _FileGetPathToApplication(void);
static StringList* _FileGetFilesInDir(const String *dir);
static bool _FileDebug(void);
static bool _FileDebugClassMethods(const String *path);
static bool _FileDebugClassMethodsManualVerification(const String *path);
static bool _FileDebugSetPath(const String *path);
static bool _FileDebugGet(const String *path);
static bool _FileDebugRead(const String *path);
static bool _FileDebugGeneralIO(const String *path);
static bool _FileDebugMisc(const String *path);
static void _FileDestructor(File **obj);
static void File_SetPath(File *self, const String *newPath);
static StringList* File_GetDirList(const File *self);
static String* File_GetLineWithToken(File *self, const String *token);
static bool File_StartIter(File *self);
static bool File_HasNext(File *self);
static String* File_GetNextLine(File *self);
static String* File_GetUntilNextToken(File *self, const char token);
static void File_EndIter(File *self);
static void File_Write(File *self, const String *data);
static void File_Append(File *self, const String *data);
static int File_GetNumLines(File *self);
static void File_Clear(File *self);
static void File_Rename(File *self, const String *newName);
static void File_ChangeExtension(File *self, const String *newExt);
static void File_SoftChangeExtension(File *self, const String *newExt);
static bool File_Remove(File *self);
static bool File_Equals(File *self, File *other);
static void File_SetAbsolutePath(File *self);
static void File_SetDrive(File *self);
static void File_SetName(File *self);
static void File_SetExtension(File *self);
static bool File_Open(File *self, const String *operation);
static void File_Close(File *self);
//static File* File_CreateTempFile(const File *self);

static const struct FileMethods fileMethods={
	.setPath=&File_SetPath,
	.getDirList=&File_GetDirList,
	.softChangeExtension=&File_SoftChangeExtension,
	.getLineWithToken=&File_GetLineWithToken,
	.startIter=&File_StartIter,
	.hasNext=&File_HasNext,
	.getNextLine=&File_GetNextLine,
	.getUntilNextToken=&File_GetUntilNextToken,
	.endIter=&File_EndIter,
	.write=&File_Write,
	.append=&File_Append,
	.getNumLines=&File_GetNumLines,
	.clear=&File_Clear,
	.rename=&File_Rename,
	.changeExtension=&File_ChangeExtension,
	.remove=&File_Remove,
	.equals=&File_Equals,
};
const struct FileClass_t FileClass={
	.new=&_FileConstructor,
	.delete=&_FileDestructor,
	.print=&_FilePrintObj,
	.fileExists=&_FileExists,
	.getCwd=&_FileGetCwd,
	.getPathFromList=&_FileGetPathFromList,
	.getPathToApplication=&_FileGetPathToApplication,
	.getFilesInDir=&_FileGetFilesInDir,
	.debug=&_FileDebug,
	.objSize=sizeof(File),
	.pathSeparator=
#if OS==WINDOWS
		'\\',
#else
		'/',
#endif
};

//Object Methods================================================================
//Public methods
//Soft IO
static void File_SetPath(File *self, const String *newPath){
	self->path->methods->copyStrBetween(self->path,newPath,0,newPath->length);
	File_SetAbsolutePath(self);
	File_SetDrive(self);
	File_SetName(self);
	File_SetExtension(self);
}

static StringList* File_GetDirList(const File *self){
	StringList *rv=StringListClass.new();
	rv->methods->listFromDelimiter(rv,self->absPath->str,&FileClass.pathSeparator);
	rv->ownsData=true;
	return rv;
}

static void File_SoftChangeExtension(File *self, const String *newExt){
	String *newPath=StringClass.new();
	String *temp=(self->absPath->str!=NULL)? self->absPath: self->path;
	int index=temp->methods->getLastIndexOf(temp,'.')+1;
	index=(index<0)? temp->length: index;
	newPath->methods->copyStrBetween(newPath,temp,0,index);
	(index==temp->length)? newPath->methods->concatChar(newPath,'.'): 0;
	newPath->methods->concat(newPath,newExt->str);
	self->methods->setPath(self,newPath);
}

//IO
static String* File_GetLineWithToken(File *self, const String *token){
	String *rv=StringClass.new();
	bool settingFound=false;
	if (self->methods->startIter(self)){
		while (self->methods->hasNext(self) && !settingFound){
			String *line=self->methods->getNextLine(self);
			if (line->methods->contains(line,token->str)){
				rv->methods->set(rv,line->str);
				settingFound=true;
			}
			StringClass.delete(&line);
		}
	}
	self->methods->endIter(self);
	return rv;
}

static bool File_StartIter(File *self){
	self->filePntr=fopen(self->absPath->str,"rb");
	return (self->filePntr!=NULL)? true: false;
}

static bool File_HasNext(File *self){
	if (self->filePntr!=NULL){
		int c=fgetc(self->filePntr);
		if (c!=EOF){
			fseek(self->filePntr,-1L,SEEK_CUR);
			return true;
		}
	}
	return false;
}

static String* File_GetNextLine(File *self){
	return self->methods->getUntilNextToken(self,'\n');
}

static String* File_GetUntilNextToken(File *self, const char token){
	String *rv=StringClass.new();
	if (self->filePntr!=NULL){
		int cntr=0;
		int curChar=fgetc(self->filePntr);
		for (cntr=0; curChar!=token && curChar!=EOF; cntr++){
			curChar=fgetc(self->filePntr);
		}
		fseek(self->filePntr,-cntr-1,SEEK_CUR);
		rv->methods->setSize(rv,cntr);
		fgets(rv->str,cntr+1,self->filePntr);
		fseek(self->filePntr,1,SEEK_CUR);
	}
	return rv;
}

static void File_EndIter(File *self){
	if (self->filePntr!=NULL){
		fclose(self->filePntr);
		self->filePntr=NULL;
	}
}

static void File_Write(File *self, const String *data){
	String *openOpp=StringClass.new();
	openOpp->methods->set(openOpp,"w");
	File_Open(self,openOpp);
	if (self->filePntr!=NULL){
		fputs(data->str,self->filePntr);
	}
	File_Close(self);
	StringClass.delete(&openOpp);
}

static void File_Append(File *self, const String *data){
	String *openOpp=StringClass.new();
	openOpp->methods->set(openOpp,"a");
	File_Open(self,openOpp);
	if (self->filePntr!=NULL){
		fputs(data->str,self->filePntr);
	}
	File_Close(self);
	StringClass.delete(&openOpp);
}

static int File_GetNumLines(File *self){
	String *openOpp=StringClass.new();
	openOpp->methods->set(openOpp,"rb");
	File_Open(self,openOpp);
	int cntr=(self->filePntr!=NULL)? 0: -1;
	int curChar=(self->filePntr!=NULL)? fgetc(self->filePntr): -1;
	while (self->filePntr!=NULL && curChar!=EOF){
		if (curChar=='\n'){
			cntr++;
		}
		curChar=fgetc(self->filePntr);
	}
	File_Close(self);
	StringClass.delete(&openOpp);
	return cntr;
}

static void File_Clear(File *self){
	String *openOpp=StringClass.new();
	openOpp->methods->set(openOpp,"w");
	File_Open(self,openOpp);
	File_Close(self);
	StringClass.delete(&openOpp);
}

static void File_Rename(File *self, const String *newName){
	String *newPath=StringClass.new();
	String *temp=(self->absPath->str!=NULL)? self->absPath: self->path;
	int startIndex=temp->methods->getLastIndexOf(temp,FileClass.pathSeparator)+1;
	int endIndex=temp->methods->getLastIndexOf(temp,'.');
	startIndex=(startIndex<0)? 0: startIndex;
	endIndex=(endIndex<0)? temp->length: endIndex;
	newPath->methods->copyStrBetween(newPath,temp,0,startIndex);
	newPath->methods->concat(newPath,newName->str);
	if (endIndex!=temp->length){
		newPath->methods->concatChar(newPath,'.');
		newPath->methods->concat(newPath,self->extension->str);
	}
	rename(self->absPath->str,newPath->str);
	self->methods->setPath(self,newPath);
}

static void File_ChangeExtension(File *self, const String *newExt){
	String *newPath=StringClass.new();
	String *temp=(self->absPath->str!=NULL)? self->absPath: self->path;
	int index=temp->methods->getLastIndexOf(temp,'.')+1;
	index=(index<0)? temp->length: index;
	newPath->methods->copyStrBetween(newPath,temp,0,index);
	(index==temp->length)? newPath->methods->concatChar(newPath,'.'): 0;
	newPath->methods->concat(newPath,newExt->str);
	rename(self->absPath->str,newPath->str);
	self->methods->setPath(self,newPath);
}

static bool File_Remove(File *self){
	return (remove(self->absPath->str)==0)? true: false;
}

static bool File_Equals(File *self, File *other){
	bool rv=(self->absPath->methods->equals(self->absPath,other->absPath->str));
	if (!rv && self->methods->startIter(self) && other->methods->startIter(other)){
		rv=(self->methods->hasNext(self) && other->methods->hasNext(other));
		while (rv){
			String *str1=self->methods->getNextLine(self);
			String *str2=self->methods->getNextLine(self);
			rv&=str1->methods->equals(str1,str2->str);
			rv&=(self->methods->hasNext(self) && other->methods->hasNext(other));
		}
		self->methods->endIter(self);
		other->methods->endIter(other);
	}
	return rv;
}


//static void File_WriteLine(File *self, const String *data, const int line){
//	File *output=File_CreateTempFile(self);
//	if (self->methods->startIter(self) && output->methods->open(output)){
//		for (int i=0; self->methods->hasNext(self); i++){
//			if (i+1==line){
//				fprintf(output->filePntr,"%s\n",data->str);
//			} else {
//				String *line=self->methods->getNext(self);
//				fprintf(output->filePntr,"%s\n",line->str);
//				StringClass.delete(&line);
//			}
//		}
//	}
//	self->methods->endIter(self);
//	output->methods->close(output);
//}

//Private methods
#if OS==WINDOWS
static void File_SetAbsolutePath(File *self){
	String *ap=self->absPath;
	ap->methods->setSize(ap,PATH_MAX);
	_fullpath(ap->str,self->path->str,PATH_MAX);
	int lineEndIndex=ap->methods->getFirstIndexOf(ap,'\n');
	ap->methods->trimSubString(ap,lineEndIndex,ap->length);
}
#elif OS==LINUX || OS==UNIX
static void File_SetAbsolutePath(File *self){
	String *ap=self->absPath;
	ap->methods->setSize(ap,PATH_MAX);
	realPath(ap->str,self->path->str);
	int lineEndIndex=ap->methods->getFirstIndexOf(ap,'\n');
	ap->methods->trimSubString(ap,lineEndIndex,ap->length);
}
#endif

#if OS==WINDOWS
static void File_SetDrive(File *self){
	String *temp=(self->absPath->str!=NULL)? self->absPath: self->path;
	int endIndex=temp->methods->getFirstIndexOf(temp,':')+1;
	if (endIndex>0){
		self->drive->methods->copyStrBetween(self->drive,temp,0,endIndex);
	}
}
#endif

static void File_SetName(File *self){
	String *temp=(self->absPath->str!=NULL)? self->absPath: self->path;
	int startIndex=temp->methods->getLastIndexOf(temp,FileClass.pathSeparator)+1;
	int endIndex=temp->methods->getLastIndexOf(temp,'.');
	startIndex=(startIndex<0)? 0: startIndex;
	endIndex=(endIndex<0)? temp->length: endIndex;
	self->name->methods->copyStrBetween(self->name,temp,startIndex,endIndex);
}

static void File_SetExtension(File *self){
	String *temp=(self->absPath->str!=NULL)? self->absPath: self->path;
	int startIndex=temp->methods->getLastIndexOf(temp,'.')+1;
	if (startIndex!=-1){
		self->extension->methods->copyStrBetween(self->extension,temp,startIndex,temp->length);
	}
}

static bool File_Open(File *self, const String *operation){
	if (self->filePntr==NULL){
		self->filePntr=fopen(self->absPath->str,operation->str);
	}
	return (self->filePntr!=NULL)? true: false;
}

static void File_Close(File *self){
	if (self->filePntr!=NULL){
		fclose(self->filePntr);
		self->filePntr=NULL;
	}
}

//static File* File_CreateTempFile(const File *self){
//	int cntr=0;
//	File *rv=FileClass.new();
//	String *temp=StringClass.new();
//	temp->methods->concatChar(temp,'.');
//	temp->methods->concat(temp,&FileClass.pathSeparator);
//	temp->methods->set(temp,self->name->str);
//	temp->methods->concatChar(temp,'0');
//	temp->methods->concatChar(temp,'.');
//	temp->methods->concat(temp,self->extension->str);
//	rv->methods->setPath(rv,temp);
//	while (FileClass.fileExists(rv)){
//		cntr++;
//		temp->str[self->name->length-1]=cntr+'0';
//		rv->methods->setPath(rv,temp);
//	}
//	StringClass.delete(&temp);
//	return rv;
//}

//Class Methods=================================================================
static File* _FileConstructor(void){
	File *rv=(File*)createObject(FileClass.objSize);
	rv->absPath=StringClass.new();
	rv->path=StringClass.new();
	rv->drive=StringClass.new();
	rv->name=StringClass.new();
	rv->extension=StringClass.new();
	rv->filePntr=NULL;
	rv->methods=&fileMethods;
	return rv;
}

static void _FilePrintObj(File *obj){
	PrintClass.print("<File Obj[Addr: %p]: Path: '%s'>\n",obj,obj->path->str);
	PrintClass.incrementIndentLevel(1);
	PrintClass.print("Abs Path: '%s'\n",obj->absPath->str);
	PrintClass.print("Drive: '%s'\n", obj->drive->str);
	PrintClass.print("Name: '%s'\n",obj->name->str);
	PrintClass.print("Extension: '%s'\n",obj->extension->str);
	PrintClass.incrementIndentLevel(-1);
}

static bool _FileExists(const File *obj){
	if (access(obj->path->str,F_OK)!=-1 || access(obj->absPath->str,F_OK)!=-1){
		return true;
	}
	return false;
}

static String* _FileGetCwd(void){
	String *rv=StringClass.new();
	rv->methods->setSize(rv,PATH_MAX);
	getcwd(rv->str,rv->length);
	int lineEndIndex=rv->methods->getFirstIndexOf(rv,'\n');
	rv->methods->trimSubString(rv,lineEndIndex,rv->length);
	return rv;
}

static String* _FileGetPathFromList(const StringList *dirs){
	String *rv=StringClass.new();
	for (int i=0; i<dirs->length; i++){
		String *iterStr=dirs->methods->get(dirs,i);
		rv->methods->concat(rv,iterStr->str);
		if (i<dirs->length-1){
			rv->methods->concatChar(rv,FileClass.pathSeparator);
		}
	}
	return rv;
}

#if OS==WINDOWS
static String* _FileGetPathToApplication(void){
	String *rv=StringClass.new();
	rv->methods->setSize(rv,1024);
	while (GetModuleFileName(NULL,rv->str,rv->length)==0 &&
	       rv->length<2048){
		rv->methods->setSize(rv,rv->length+100);
	}
	rv->methods->trimWhitespace(rv);
	return rv;
}
#elif OS==UNIX || OS==LINUX
static String* _FileGetPathToApplication(void){
	String *rv=StringClass.new();
	rv->methods->setSize(rv,1024);
	while (readlink("/proc/self/exe",rv->str,rv->length)==-1 &&
	       rv->length<2048){
		rv->methods->setSize(rv,rv->length+100);
	}
	rv->methods->trimWhitespace(rv);
	return rv;
}
#endif

#if OS==WINDOWS
static StringList* _FileGetFilesInDir(const String *path){
	bool _break=false;
	File *file=FileClass.new();
	file->methods->setPath(file,path);
	String *pathToApp=FileClass.getPathToApplication();
	String *dirPath=StringClass.new();
	int index=file->absPath->methods->getLastIndexOf(file->absPath,FileClass.pathSeparator);
	index=(index<0)? file->absPath->length: index;
	dirPath->methods->copyStrBetween(dirPath,file->absPath,0,index);
	StringList *rv=StringListClass.new();
	rv->ownsData=true;

	WIN32_FIND_DATA ffd;
	HANDLE hFind=INVALID_HANDLE_VALUE;
	hFind=FindFirstFile(file->absPath->str,&ffd);
	while (!_break && hFind!=INVALID_HANDLE_VALUE){
		String *iterStr=StringClass.new();
		iterStr->methods->set(iterStr,dirPath->str);
		iterStr->methods->concatChar(iterStr,FileClass.pathSeparator);
		iterStr->methods->concat(iterStr,ffd.cFileName);
		rv->methods->add(rv,(const String**)&iterStr,1);
		(FindNextFile(hFind,&ffd)==0)? _break=true: 0;
	}

	StringClass.delete(&pathToApp);
	StringClass.delete(&dirPath);
	FileClass.delete(&file);
	return rv;
}
#elif OS==UNIX || OS==MAC
static StringList* _FileGetFilesInDir(const String *dir){
	PrintClass.printError("Function not implemented on this platform.\n");
	StringList *rv=StringListClass.new();
	return rv;
}
#endif

static bool _FileDebug(void){
	bool successfull=true;
	String *path=StringClass.new();
	path->methods->concatChar(path,'.');
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"debug");
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"FileClassDebug.txt");
	successfull&=_FileDebugClassMethods(path);
	successfull&=_FileDebugClassMethodsManualVerification(path);
	successfull&=_FileDebugSetPath(path);
	successfull&=_FileDebugGet(path);
	successfull&=_FileDebugRead(path);
	successfull&=_FileDebugGeneralIO(path);
	successfull&=_FileDebugMisc(path);
	PrintClass.objectDebug("File",successfull);
	StringClass.delete(&path);
	return successfull;
}

static bool _FileDebugClassMethods(const String *path){
	bool successfull=true;
	StringList *dirs=StringListClass.new();
	dirs->ownsData=true;
	String *strs[3];
	for (int i=0; i<3; i++){
		strs[i]=StringClass.new();
	}
	strs[0]->methods->set(strs[0],".");
	strs[1]->methods->set(strs[1],"debug");
	strs[2]->methods->set(strs[2],"FileClassDebug.txt");
	dirs->methods->add(dirs,(const String**)strs,3);
	String *listPath=FileClass.getPathFromList(dirs);
	(!listPath->methods->equals(listPath,path->str))? successfull=false: 0;
	File *test=FileClass.new();
	String *cwdPath=StringClass.new();
	String *pathSep=StringClass.new();
	String *cwd=FileClass.getCwd();
	pathSep->methods->set(pathSep,&FileClass.pathSeparator);
	cwdPath->methods->copyStrBetween(cwdPath,cwd,0,cwd->length);
	cwdPath->methods->concat(cwdPath,pathSep->str);
	cwdPath->methods->concat(cwdPath,"debug");
	cwdPath->methods->concat(cwdPath,pathSep->str);
	cwdPath->methods->concat(cwdPath,"FileClassDebug.txt");
	test->methods->setPath(test,cwdPath);
	(cwd->length==0)? successfull=false: 0;
	(pathSep->length==0)? successfull=false: 0;
	(!FileClass.fileExists(test))? successfull=false: 0;

	StringListClass.delete(&dirs);
	StringClass.delete(&listPath);
	StringClass.delete(&cwdPath);
	StringClass.delete(&cwd);
	StringClass.delete(&pathSep);
	FileClass.delete(&test);
	return successfull;
}

static bool _FileDebugClassMethodsManualVerification(const String *path){
	bool successfull=true;
	//I do not know of any way to debug this...
	String *pathToApp=FileClass.getPathToApplication();
	//StringClass.print(pathToApp);
	StringClass.delete(&pathToApp);

	String *dir=StringClass.new();
	dir->methods->set(dir,"./debug/*.xml");
	StringList *filesInDir=FileClass.getFilesInDir(dir);
	//StringListClass.print(filesInDir);
	StringListClass.delete(&filesInDir);
	StringClass.delete(&dir);

	return successfull;
}

static bool _FileDebugSetPath(const String *path){
	bool successfull=true;
	File *testFile=FileClass.new();
	testFile->methods->setPath(testFile,path);
	(testFile->absPath->length==0)? successfull=false: 0;
	(!testFile->name->methods->equals(testFile->name,"FileClassDebug"))? successfull=false: 0;
	(!testFile->extension->methods->equals(testFile->extension,"txt"))? successfull=false: 0;
	//FileClass.print(testFile);
	FileClass.delete(&testFile);
	return successfull;
}

static bool _FileDebugGet(const String *path){
	bool successfull=true;
	String *searchToken=StringClass.new();
	File *testFile=FileClass.new();
	searchToken->methods->set(searchToken,"9");
	testFile->methods->setPath(testFile,path);
	StringList *dirList=testFile->methods->getDirList(testFile);
	String *temp=dirList->methods->get(dirList,dirList->length-1);
	(!temp->methods->equals(temp,"FileClassDebug.txt"))? successfull=false: 0;
	temp=dirList->methods->get(dirList,dirList->length-2);
	(!temp->methods->equals(temp,"debug"))? successfull=false: 0;
	String *line=testFile->methods->getLineWithToken(testFile,searchToken);
	(!line->methods->equals(line,"Testing9"))? successfull=false: 0;
	//StringListClass.print(dirList);
	//FileClass.print(testFile);
	StringClass.delete(&line);
	StringClass.delete(&searchToken);
	StringListClass.delete(&dirList);
	FileClass.delete(&testFile);
	return successfull;
}

static bool _FileDebugRead(const String *path){
	bool successfull=true;
	File *testFile=FileClass.new();
	testFile->methods->setPath(testFile,path);
	if (testFile->methods->startIter(testFile)){
		for (int i=0; testFile->methods->hasNext(testFile); i++){
			String *line=testFile->methods->getNextLine(testFile);
			String *comp=StringClass.new();
			comp->methods->set(comp,"TestingX");
			comp->str[comp->length-1]=i+'0';
			(!comp->methods->equals(comp,line->str))? successfull=false: 0;
			StringClass.delete(&line);
			StringClass.delete(&comp);
		}
	}
	testFile->methods->endIter(testFile);
	//FileClass.print(testFile);
	FileClass.delete(&testFile);
	return successfull;
}

static bool _FileDebugGeneralIO(const String *path){
	bool successfull=true;
	String *data=StringClass.new();
	data->methods->set(data,"Each\nWord\nShould\nBe\nOn\nA\nNew\nLine\n");
	String *newName=StringClass.new();
	newName->methods->set(newName,"FileClassDebug3");
	String *newFile=StringClass.new();
	newFile->methods->concatChar(newFile,'.');
	newFile->methods->concatChar(newFile,FileClass.pathSeparator);
	newFile->methods->concat(newFile,"debug");
	newFile->methods->concatChar(newFile,FileClass.pathSeparator);
	newFile->methods->concat(newFile,"FileClassDebug2.txt");

	File *testFile=FileClass.new();
	testFile->methods->setPath(testFile,newFile);
	(FileClass.fileExists(testFile))? successfull=false: 0;

	testFile->methods->write(testFile,data);
	(testFile->methods->getNumLines(testFile)!=8)? successfull=false: 0;

	testFile->methods->append(testFile,data);
	(testFile->methods->getNumLines(testFile)!=16)? successfull=false: 0;

	testFile->methods->clear(testFile);
	(testFile->methods->getNumLines(testFile)!=0)? successfull=false: 0;

	testFile->methods->rename(testFile,newName);
	(!newName->methods->equals(newName,testFile->name->str))? successfull=false: 0;

	String *newExt=StringClass.new();
	newExt->methods->set(newExt,"xml");
	testFile->methods->changeExtension(testFile,newExt);
	(!testFile->extension->methods->equals(testFile->extension,newExt->str))? successfull=false: 0;
	newExt->methods->set(newExt,"txt");
	testFile->methods->changeExtension(testFile,newExt);
	(!testFile->extension->methods->equals(testFile->extension,newExt->str))? successfull=false: 0;
	StringClass.delete(&newExt);

	(!FileClass.fileExists(testFile))? successfull=false: 0;
	(!testFile->methods->remove(testFile))? successfull=false: 0;
	(FileClass.fileExists(testFile))? successfull=false: 0;

	//FileClass.print(testFile);
	StringClass.delete(&data);
	StringClass.delete(&newName);
	StringClass.delete(&newFile);
	FileClass.delete(&testFile);
	return successfull;
}

static bool _FileDebugMisc(const String *path){
	bool successfull=true;
	File *file1=FileClass.new();
	File *file2=FileClass.new();
	file1->methods->setPath(file1,path);
	file2->methods->setPath(file2,path);
	(!file1->methods->equals(file1,file2))? successfull=false: 0;
	FileClass.delete(&file1);
	FileClass.delete(&file2);
	return successfull;
}

static void _FileDestructor(File **obj){
	File_Close(*obj);
	StringClass.delete(&(*obj)->absPath);
	StringClass.delete(&(*obj)->path);
	StringClass.delete(&(*obj)->drive);
	StringClass.delete(&(*obj)->name);
	StringClass.delete(&(*obj)->extension);
	deleteObject((void**)obj);
}
