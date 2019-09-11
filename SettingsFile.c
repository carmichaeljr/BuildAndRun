#include <stdbool.h>
#include "SettingsFile.h"
#include "Object.h"
#include "Print.h"
#include "File.h"
#include "XMLFile.h"
#include "StringList.h"

static SettingsFile* _SettingsFileConstructor(void);
static String* _SettingsFileGetPathToSettingsFile(void);
static void _SettingsFilePrintObj(SettingsFile *obj);
static bool _SettingsFileDebug(void);
static bool _SettingsFileDebugProgramSettings(const String *filePath);
static bool _SettingsFileDebugLanguageSettings(const String *filePath);
static bool _SettingsFileDebugBuildFile(const String *filePath);
//static bool _SettingsFileDebugGeneral(const char **data);
//static bool _SettingsFileDebugValid(const char **data);
//static bool _SettingsFileDebugOpReq(const char **data);
static void _SettingsFileDestructor(SettingsFile **obj);
static StringList* SettingsFile_GetAcceptedFirstArgs(const SettingsFile *self);
static bool SettingsFile_LangDefExists(const SettingsFile *self, const String *ext);
static XMLFile* SettingsFile_GetBuildFileInfo(const SettingsFile *self, const String *ext);
static const String* SettingsFile_GetOutputPath(const SettingsFile *self, const String *ext);
static void SettingsFile_OpenForUser(const SettingsFile *self);

static const struct SettingsFileMethods settingsFileMethods={
	.getAcceptedFirstArgs=&SettingsFile_GetAcceptedFirstArgs,
	.langDefExists=&SettingsFile_LangDefExists,
	.getBuildFileInfo=&SettingsFile_GetBuildFileInfo,
	.getOutputPath=&SettingsFile_GetOutputPath,
	.openForUser=&SettingsFile_OpenForUser,
};
const struct SettingsFileClass_t SettingsFileClass={
	.new=&_SettingsFileConstructor,
	.delete=&_SettingsFileDestructor,
	.print=&_SettingsFilePrintObj,
	.debug=&_SettingsFileDebug,
	.objSize=sizeof(SettingsFile),
};

//Object methods===============================================================
//Public methods
static StringList* SettingsFile_GetAcceptedFirstArgs(const SettingsFile *self){
	String *tagAttrName=StringClass.new();
	tagAttrName->methods->set(tagAttrName,"accepted");
	StringList *treePath=StringListClass.new();
	treePath->ownsData=true;
	String *branches[2];
	branches[0]=StringClass.new();
	branches[1]=StringClass.new();
	branches[0]->methods->set(branches[0],"ProgramSettings");
	branches[1]->methods->set(branches[1],"LanguageSettings");
	treePath->methods->add(treePath,(const String**)&branches[0],1);
	treePath->methods->add(treePath,(const String**)&branches[1],1);
	self->xmlFile->methods->placeCurrentNode(self->xmlFile,treePath);
	XMLTag *tag=self->xmlFile->methods->get(self->xmlFile);
	String *data=tag->methods->getAttribute(tag,tagAttrName);
	StringList *rv=StringListClass.new();
	rv->ownsData=true;
	rv->methods->listFromDelimiter(rv,data->str,",");
	StringListClass.delete(&treePath);
	StringClass.delete(&tagAttrName);
	return rv;
}

static bool SettingsFile_LangDefExists(const SettingsFile *self, const String *ext){
	bool rv=false;
	String *attrName=StringClass.new();
	attrName->methods->set(attrName,"ext");
	String *branchOne=StringClass.new();
	branchOne->methods->set(branchOne,"LanguageSettings");
	StringList *branch=StringListClass.new();
	branch->ownsData=true;
	branch->methods->add(branch,(const String**)&branchOne,1);
	self->xmlFile->methods->placeCurrentNode(self->xmlFile,branch);
	self->xmlFile->methods->moveDown(self->xmlFile);
	XMLTag *tag=self->xmlFile->methods->get(self->xmlFile);
	String *iterExt=tag->methods->getAttribute(tag,attrName);
	rv=ext->methods->equals(ext,iterExt->str);
	while (!rv && self->xmlFile->methods->moveNext(self->xmlFile)){
		XMLTag *tag=self->xmlFile->methods->get(self->xmlFile);
		String *iterExt=tag->methods->getAttribute(tag,attrName);
		rv=ext->methods->equals(ext,iterExt->str);
	}
	StringClass.delete(&attrName);
	StringListClass.delete(&branch);
	return rv;
}

static XMLFile* SettingsFile_GetBuildFileInfo(const SettingsFile *self, const String *ext){
	XMLFile *rv=XMLFileClass.new();
	if (self->methods->langDefExists(self,ext)){
		rv->ownsData=false;
		rv->methods->copyOtherBelowCurNode(rv,self->xmlFile);
	}
	return rv;
}

static const String* SettingsFile_GetOutputPath(const SettingsFile *self, const String *ext){
	const String *rv=NULL;
	String *outputAttr=StringClass.new();
	outputAttr->methods->set(outputAttr,"outputPath");
	if (self->methods->langDefExists(self,ext)){
		XMLTag *tag=self->xmlFile->methods->get(self->xmlFile);
		rv=(const String*)tag->methods->getAttribute(tag,outputAttr);
	}
	StringClass.delete(&outputAttr);
	return rv;
}

static void SettingsFile_OpenForUser(const SettingsFile *self){
	String *cwd=FileClass.getCwd();
	String *cmd=StringClass.new();
	cmd->methods->concat(cmd,"cd /D \"");
	cmd->methods->concat(cmd,cwd->str);
	cmd->methods->concat(cmd,"\" && ");
	cmd->methods->concat(cmd,"C:\\Windows\\notepad.exe ./data/ProgramSettings.xml");
	system(cmd->str);
	StringClass.delete(&cwd);
	StringClass.delete(&cmd);
}

//Private methods
//Class methods================================================================
static SettingsFile* _SettingsFileConstructor(void){
	String *path=_SettingsFileGetPathToSettingsFile();
	SettingsFile *rv=(SettingsFile*)createObject(SettingsFileClass.objSize);
	rv->xmlFile=XMLFileClass.new();
	rv->xmlFile->methods->setSource(rv->xmlFile,path);
	rv->xmlFile->ownsData=true;
	rv->methods=&settingsFileMethods;
	StringClass.delete(&path);
	return rv;
}

static String* _SettingsFileGetPathToSettingsFile(void){
	String *rv=FileClass.getPathToApplication();
	int lastPathSep=rv->methods->getLastIndexOf(rv,FileClass.pathSeparator);
	rv->methods->trimSubString(rv,lastPathSep+1,rv->length);
	rv->methods->concat(rv,"data");
	rv->methods->concatChar(rv,FileClass.pathSeparator);
	rv->methods->concat(rv,"ProgramSettings.xml");
	return rv;
}

static void _SettingsFilePrintObj(SettingsFile *obj){
	PrintClass.print("<SettingsFile Obj[Addr: %p]>\n",obj);
	PrintClass.incrementIndentLevel(1);
	XMLFileClass.print(obj->xmlFile);
	PrintClass.incrementIndentLevel(-1);
}

static bool _SettingsFileDebug(void){
	bool successfull=true;
	String *path=StringClass.new();
	path->methods->concatChar(path,'.');
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"debug");
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"ProgramSettingsDebug.xml");
	successfull&=_SettingsFileDebugProgramSettings(path);
	successfull&=_SettingsFileDebugLanguageSettings(path);
	successfull&=_SettingsFileDebugBuildFile(path);
	PrintClass.objectDebug("SettingsFile",successfull);
	StringClass.delete(&path);
	return successfull;
}

static bool _SettingsFileDebugProgramSettings(const String *filePath){
	bool successfull=true;
	SettingsFile *test=SettingsFileClass.new();
	test->xmlFile->methods->setSource(test->xmlFile,filePath);
	StringList *firstArgs=test->methods->getAcceptedFirstArgs(test);
	(firstArgs->length!=8)? successfull=false: 0;
	String *temp=firstArgs->methods->get(firstArgs,0);
	(!temp->methods->equals(temp,"c"))? successfull=false: 0;
	temp=firstArgs->methods->get(firstArgs,firstArgs->length-2);
	(!temp->methods->equals(temp,"cel"))? successfull=false: 0;
	StringListClass.delete(&firstArgs);
	//SettingsFileClass.print(test);
	SettingsFileClass.delete(&test);
	return successfull;
}

static bool _SettingsFileDebugLanguageSettings(const String *filePath){
	bool successfull=true;
	String *ext=StringClass.new();
	ext->methods->set(ext,"py");
	SettingsFile *test=SettingsFileClass.new();
	test->xmlFile->methods->setSource(test->xmlFile,filePath);
	const String *outputPath=test->methods->getOutputPath(test,ext);
	(!outputPath->methods->equals(outputPath,"\"C:\\Users\\JackCarmichael\\Documents\""))? successfull=false: 0;
	(!test->methods->langDefExists(test,ext))? successfull=false: 0;
	ext->methods->set(ext,"notALang");
	(test->methods->langDefExists(test,ext))? successfull=false: 0;
	//SettingsFileClass.print(test);
	SettingsFileClass.delete(&test);
	return successfull;
}

static bool _SettingsFileDebugBuildFile(const String *filePath){
	bool successfull=true;
	String *ext=StringClass.new();
	ext->methods->set(ext,"py");
	String *path=StringClass.new();
	path->methods->concatChar(path,'.');
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"debug");
	path->methods->concatChar(path,FileClass.pathSeparator);
	path->methods->concat(path,"LanguageSettingsDebug.xml");
	SettingsFile *test=SettingsFileClass.new();
	test->xmlFile->methods->setSource(test->xmlFile,filePath);
	XMLFile *validation=XMLFileClass.new();
	validation->methods->setSource(validation,path);
	//XMLFileClass.print(validation);
	XMLFile *buildFile=test->methods->getBuildFileInfo(test,ext);
	//XMLFileClass.print(buildFile);
	(buildFile->numTags<=0)? successfull=false: 0;	
	(!validation->methods->equals(validation,buildFile))? successfull=false: 0;
	StringClass.delete(&ext);
	StringClass.delete(&path);
	XMLFileClass.delete(&validation);
	XMLFileClass.delete(&buildFile);
	SettingsFileClass.delete(&test);
	return successfull;
}

static void _SettingsFileDestructor(SettingsFile **obj){
	XMLFileClass.delete(&(*obj)->xmlFile);
	deleteObject((void**)obj);
}
