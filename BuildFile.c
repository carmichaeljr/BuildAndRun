#include <stdbool.h>
#include "BuildFile.h"
#include "Object.h"
#include "Print.h"
#include "File.h"
#include "XMLFile.h"
#include "XMLTag.h"
#include "SettingsFile.h"

static BuildFile* _BuildFileConstructor(void);
static void _BuildFilePrintObj(BuildFile *obj);
static bool _BuildFileExists(const String *mainFile);
static bool _BuildFileDebug(void);
static bool _BuildFileDebugClassMethods(void);
static bool _BuildFileDebugSetSource(void);
static bool _BuildFileDebugGetLineCountList(void);
static bool _BuildFileDebugGenerate(void);
static void _BuildFileDestructor(BuildFile **obj);
static bool BuildFile_SetSource(BuildFile *self, const File *mainFile);
static void BuildFile_GenerateBuildFile(BuildFile *self, const File *mainFile);
static String* BuildFile_GetCommand(BuildFile *self, const String *type);
static StringList* BuildFile_GetLineCountList(BuildFile *self);
static File* BuildFile_GetBuildFileFromMainFile(BuildFile *self, const File *mainFile);
static void BuildFile_CreateBuildFileTemplate(BuildFile *self, const File *file, const String *ext);
static XMLTag* BuildFile_CreateTagFromCurDirTag(void);
static void BuildFile_ReplaceTags(BuildFile *self, const File *mainFile, const String *outputPath);
static XMLTag* BuildFile_CreateTagFromMainFileTag(const File *mainFile, const XMLTag *iterTag);
static XMLTag* BuildFile_CreateTagFromOutputTag(const String *outputPath);

static const struct BuildFileMethods buildFileMethods={
	.setSource=&BuildFile_SetSource,
	.generateBuildFile=&BuildFile_GenerateBuildFile,
	.getCommand=&BuildFile_GetCommand,
	.getLineCountList=&BuildFile_GetLineCountList,
};
const struct BuildFileClass_t BuildFileClass={
	.new=&_BuildFileConstructor,
	.delete=&_BuildFileDestructor,
	.print=&_BuildFilePrintObj,
	.buildFileExists=&_BuildFileExists,
	.debug=&_BuildFileDebug,
	.objSize=sizeof(BuildFile),
};

//Object Methods===============================================================
//Public Methods
static bool BuildFile_SetSource(BuildFile *self, const File *mainFile){
	File *buildFile=BuildFile_GetBuildFileFromMainFile(self,mainFile);
	bool rv=BuildFileClass.buildFileExists(buildFile->absPath);
	if (rv){
		self->xmlFile->methods->setSource(self->xmlFile,buildFile->absPath);
	}
	FileClass.delete(&buildFile);
	return rv;
}

static void BuildFile_GenerateBuildFile(BuildFile *self, const File *mainFile){
	if (!BuildFileClass.buildFileExists(mainFile->absPath)){
		File *file=BuildFile_GetBuildFileFromMainFile(self,mainFile);

		BuildFile_CreateBuildFileTemplate(self,file,mainFile->extension);
		self->xmlFile->methods->setSource(self->xmlFile,file->absPath);

		SettingsFile *settings=SettingsFileClass.new();
		const String *outputPath=settings->methods->getOutputPath(settings,mainFile->extension);
		BuildFile_ReplaceTags(self,mainFile,outputPath);

		self->xmlFile->methods->write(self->xmlFile);
		SettingsFileClass.delete(&settings);
		FileClass.delete(&file);
	} else {
		self->methods->setSource(self,mainFile);
	}
}

static String* BuildFile_GetCommand(BuildFile *self, const String *type){
	bool _break=false;
	String *rv=StringClass.new();
	String *cmdAttr=StringClass.new();
	cmdAttr->methods->set(cmdAttr,"cmd");
	String *addSpaceAttr=StringClass.new();
	addSpaceAttr->methods->set(addSpaceAttr,"addSpace");
	self->xmlFile->methods->startIter(self->xmlFile);
	while (!_break && self->xmlFile->methods->hasNext(self->xmlFile)){
		XMLTag *iterTag=self->xmlFile->methods->getNext(self->xmlFile);
		_break=iterTag->name->methods->equals(iterTag->name,type->str);
		if (_break){
			String *temp=iterTag->methods->getAttribute(iterTag,cmdAttr);
			if (temp!=NULL){
				rv->methods->set(rv,temp->str);
				rv->methods->concatChar(rv,' ');
			}
		}
	}
	self->xmlFile->methods->startIterAtCurNode(self->xmlFile);
	while (self->xmlFile->methods->hasNext(self->xmlFile) &&
	       self->xmlFile->methods->getIterAction(self->xmlFile)!='u'){
		XMLTag *iterTag=self->xmlFile->methods->getNext(self->xmlFile);
		if (iterTag->name->methods->equals(iterTag->name,"Arg")){
			String *temp=iterTag->methods->getAttribute(iterTag,addSpaceAttr);
			rv->methods->concat(rv,iterTag->data->str);
			if (temp==NULL || temp->methods->equals(temp,"true")){
				rv->methods->concatChar(rv,' ');
			}
		}
	}
	StringClass.delete(&cmdAttr);
	StringClass.delete(&addSpaceAttr);
	return rv;
}

static StringList* BuildFile_GetLineCountList(BuildFile *self){
	bool _break=false;
	StringList *rv=StringListClass.new();
	rv->ownsData=true;
	self->xmlFile->methods->startIter(self->xmlFile);
	while (!_break && self->xmlFile->methods->hasNext(self->xmlFile)){
		XMLTag *iterTag=self->xmlFile->methods->getNext(self->xmlFile);
		_break=iterTag->name->methods->equals(iterTag->name,"LineCount");
	}
	self->xmlFile->methods->startIterAtCurNode(self->xmlFile);
	while (self->xmlFile->methods->hasNext(self->xmlFile) &&
	       self->xmlFile->methods->getIterAction(self->xmlFile)!='u'){
		XMLTag *iterTag=self->xmlFile->methods->getNext(self->xmlFile);
		if (iterTag->name->methods->equals(iterTag->name,"Files")){
			String *iterStr=StringClass.new();
			iterStr->methods->set(iterStr,iterTag->data->str);
			rv->methods->add(rv,(const String**)&iterStr,1);
		}
	}
	return rv;
}

//Private Methods
static File* BuildFile_GetBuildFileFromMainFile(BuildFile *self, const File *mainFile){
	String *xmlExt=StringClass.new();
	xmlExt->methods->set(xmlExt,"xml");
	File *file=FileClass.new();
	file->methods->setPath(file,mainFile->absPath);
	file->methods->softChangeExtension(file,xmlExt);
	StringClass.delete(&xmlExt);
	return file;
}

static void BuildFile_CreateBuildFileTemplate(BuildFile *self, const File *file, const String *ext){
	SettingsFile *settings=SettingsFileClass.new();
	XMLFile *buildFileInfo=settings->methods->getBuildFileInfo(settings,ext);
	buildFileInfo->source->methods->setPath(buildFileInfo->source,file->absPath);
	buildFileInfo->methods->write(buildFileInfo);
	SettingsFileClass.delete(&settings);
	XMLFileClass.delete(&buildFileInfo);
}

static void BuildFile_ReplaceTags(BuildFile *self, const File *mainFile, const String *outputPath){
	if (self->xmlFile->methods->startIter(self->xmlFile)){
		while (self->xmlFile->methods->hasNext(self->xmlFile)){
			XMLTag *iterTag=self->xmlFile->methods->getNext(self->xmlFile);
			//XMLTagClass.print(iterTag);
			if (iterTag->name->methods->equals(iterTag->name,"MainFile")){
				XMLTag *newTag=BuildFile_CreateTagFromMainFileTag(mainFile,iterTag);
				XMLTagClass.delete(&iterTag);
				self->xmlFile->methods->set(self->xmlFile,newTag);
			} else if (iterTag->name->methods->equals(iterTag->name,"OutputPath")){
				XMLTag *newTag=BuildFile_CreateTagFromOutputTag(outputPath);
				XMLTagClass.delete(&iterTag);
				self->xmlFile->methods->set(self->xmlFile,newTag);
			} else if (iterTag->name->methods->equals(iterTag->name,"CurrentDirectory")){
				XMLTag *newTag=BuildFile_CreateTagFromCurDirTag();
				XMLTagClass.delete(&iterTag);
				self->xmlFile->methods->set(self->xmlFile,newTag);
			}
		}
	}
}

static XMLTag* BuildFile_CreateTagFromMainFileTag(const File *mainFile, const XMLTag *iterTag){
	String *temp=StringClass.new();
	temp->methods->set(temp,"name");
	XMLTag *rv=XMLTagClass.new();
	String *attrVal=iterTag->methods->getAttribute(iterTag,temp);
	bool nameNeeded=(attrVal!=NULL)? attrVal->methods->equals(attrVal,"true"): false;
	temp->methods->set(temp,"extension");
	attrVal=iterTag->methods->getAttribute(iterTag,temp);
	bool extNeeded=(attrVal!=NULL)? attrVal->methods->equals(attrVal,"true"): false;
	temp->methods->set(temp,"Arg");
	rv->methods->setName(rv,temp);
	rv->data->methods->concatChar(rv->data,'"');
	(nameNeeded)? rv->data->methods->concat(rv->data,mainFile->name->str): 0;
	if (extNeeded){
		rv->data->methods->concatChar(rv->data,'.');
		rv->data->methods->concat(rv->data,mainFile->extension->str);
	}
	rv->data->methods->concatChar(rv->data,'"');
	StringClass.delete(&temp);
	return rv;
}

static XMLTag* BuildFile_CreateTagFromOutputTag(const String *outputPath){
	XMLTag *rv=XMLTagClass.new();
	rv->name->methods->set(rv->name,"Arg");
	if (outputPath->str[0]!='"'){
		rv->data->methods->concatChar(rv->data,'"');
	}
	rv->data->methods->concat(rv->data,outputPath->str);
	if (outputPath->str[outputPath->length-1]!='"'){
		rv->data->methods->concatChar(rv->data,'"');
	}
	return rv;
}

static XMLTag* BuildFile_CreateTagFromCurDirTag(void){
	String *cd=FileClass.getCwd();
	XMLTag *rv=XMLTagClass.new();
	rv->name->methods->set(rv->name,"Arg");
	rv->data->methods->concatChar(rv->data,'"');
	rv->data->methods->concat(rv->data,cd->str);
	rv->data->methods->concatChar(rv->data,'"');
	StringClass.delete(&cd);
	return rv;
}

//Class Methods================================================================
static BuildFile* _BuildFileConstructor(void){
	BuildFile *rv=(BuildFile*)createObject(BuildFileClass.objSize);
	rv->xmlFile=XMLFileClass.new();
	rv->methods=&buildFileMethods;
	return rv;
}

static bool _BuildFileExists(const String *mainFile){
	bool rv=false;
	String *xmlExt=StringClass.new();
	xmlExt->methods->set(xmlExt,"xml");
	File *test=FileClass.new();
	test->methods->setPath(test,mainFile);
	if (!test->extension->methods->equals(test->extension,"xml")){
		test->methods->softChangeExtension(test,xmlExt);
	}
	rv=FileClass.fileExists(test);
	FileClass.delete(&test);
	return rv;
}

static void _BuildFilePrintObj(BuildFile *obj){
	PrintClass.print("<BuildFile Obj[Addr: %p]>\n",obj);
	PrintClass.incrementIndentLevel(1);
	XMLFileClass.print(obj->xmlFile);
	PrintClass.incrementIndentLevel(-1);
}

static bool _BuildFileDebug(void){
	bool successfull=true;
	successfull&=_BuildFileDebugClassMethods();
	successfull&=_BuildFileDebugSetSource();
	successfull&=_BuildFileDebugGetLineCountList();
	successfull&=_BuildFileDebugGenerate();
	PrintClass.objectDebug("BuildFile",successfull);
	return successfull;
}

static bool _BuildFileDebugClassMethods(void){
	bool successfull=true;
	String *xmlExt=StringClass.new();
	xmlExt->methods->set(xmlExt,"xml");
	String *testFilePath=StringClass.new();
	testFilePath->methods->set(testFilePath,"debug");
	testFilePath->methods->concatChar(testFilePath,FileClass.pathSeparator);
	testFilePath->methods->concat(testFilePath,"BuildFileExistsTest.txt");
	File *testFile=FileClass.new();
	testFile->methods->setPath(testFile,testFilePath);
	(BuildFileClass.buildFileExists(testFilePath))? successfull=false: 0;
	testFile->methods->changeExtension(testFile,xmlExt);
	testFile->methods->write(testFile,testFilePath);
	(!BuildFileClass.buildFileExists(testFilePath))? successfull=false: 0;
	testFile->methods->remove(testFile);
	StringClass.delete(&testFilePath);
	StringClass.delete(&xmlExt);
	FileClass.delete(&testFile);
	return successfull;
}

static bool _BuildFileDebugSetSource(void){
	bool successfull=true;
	String *validationFilePath=StringClass.new();
	validationFilePath->methods->set(validationFilePath,"debug");
	validationFilePath->methods->concatChar(validationFilePath,FileClass.pathSeparator);
	validationFilePath->methods->concat(validationFilePath,"BuildFileDebug.xml");
	String *testFilePath=StringClass.new();
	testFilePath->methods->set(testFilePath,"debug");
	testFilePath->methods->concatChar(testFilePath,FileClass.pathSeparator);
	testFilePath->methods->concat(testFilePath,"TestBuildFile.py");
	File *testFile=FileClass.new();
	testFile->methods->setPath(testFile,testFilePath);
	XMLFile *validation=XMLFileClass.new();
	validation->methods->setSource(validation,validationFilePath);
	BuildFile *test=BuildFileClass.new();
	(!test->methods->setSource(test,testFile))? successfull=false: 0;
	(!validation->methods->equals(validation,test->xmlFile))? successfull=false: 0;
	FileClass.delete(&testFile);
	XMLFileClass.delete(&validation);
	StringClass.delete(&testFilePath);
	StringClass.delete(&validationFilePath);
	return successfull;
}

static bool _BuildFileDebugGetLineCountList(void){
	bool successfull=true;
	String *filePath=StringClass.new();
	filePath->methods->concatChar(filePath,'.');
	filePath->methods->concatChar(filePath,FileClass.pathSeparator);
	filePath->methods->concat(filePath,"debug");
	filePath->methods->concatChar(filePath,FileClass.pathSeparator);
	filePath->methods->concat(filePath,"LineCountDebug.xml");
	File *file=FileClass.new();
	file->methods->setPath(file,filePath);
	BuildFile *test=BuildFileClass.new();
	test->methods->setSource(test,file);
	StringList *lineCountList=test->methods->getLineCountList(test);
	(lineCountList->length!=1)? successfull=false: 0;
	String *validation=lineCountList->methods->get(lineCountList,0);
	(!validation->methods->equals(validation,"./debug/LineCountTest.txt"))? successfull=false: 0;
	//StringListClass.print(lineCountList);
	//BuildFileClass.print(test);
	FileClass.delete(&file);
	StringClass.delete(&filePath);
	StringListClass.delete(&lineCountList);
	BuildFileClass.delete(&test);
	return successfull;
}

static bool _BuildFileDebugGenerate(void){
	bool successfull=true;
	String *cmdType=StringClass.new();
	cmdType->methods->set(cmdType,"ExecuteCmd");
	String *cmdValidation=StringClass.new();
	cmdValidation->methods->set(cmdValidation,"\"C:\\Program Files\\Python36\\python.exe\" \"TestBuildFile.py\" ");
	String *validationPath=StringClass.new();
	validationPath->methods->concatChar(validationPath,'.');
	validationPath->methods->concatChar(validationPath,FileClass.pathSeparator);
	validationPath->methods->concat(validationPath,"debug");
	validationPath->methods->concatChar(validationPath,FileClass.pathSeparator);
	validationPath->methods->concat(validationPath,"BuildFileDebug.xml");
	String *testFilePath=StringClass.new();
	testFilePath->methods->set(testFilePath,"debug");
	testFilePath->methods->concatChar(testFilePath,FileClass.pathSeparator);
	testFilePath->methods->concat(testFilePath,"TestBuildFile.py");
	File *testFile=FileClass.new();
	testFile->methods->setPath(testFile,testFilePath);
	XMLFile *validation=XMLFileClass.new();
	validation->methods->setSource(validation,validationPath);
	BuildFile *test=BuildFileClass.new();
	test->methods->generateBuildFile(test,testFile);
	(!validation->methods->equals(validation,test->xmlFile))? successfull=false: 0;
	String *cmd=test->methods->getCommand(test,cmdType);
	(!cmdValidation->methods->equals(cmdValidation,cmd->str))? successfull=false: 0;
	//StringClass.print(cmd);
	//StringClass.print(cmdValidation);
	//BuildFileClass.print(test);
	StringClass.delete(&cmdType);
	StringClass.delete(&cmdValidation);
	StringClass.delete(&cmd);
	StringClass.delete(&testFilePath);
	StringClass.delete(&validationPath);
	FileClass.delete(&testFile);
	XMLFileClass.delete(&validation);
	BuildFileClass.delete(&test);
	return successfull;
}

static void _BuildFileDestructor(BuildFile **obj){
	XMLFileClass.delete(&(*obj)->xmlFile);
	deleteObject((void**)obj);
}
