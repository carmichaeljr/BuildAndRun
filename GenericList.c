#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GenericList.h"
#include "Object.h"
#include "Print.h"

static GenericList* _GenericListConstructor(void);
static void _GenericListDestructor(GenericList **obj);
static void _GenericListPrint(const GenericList *obj);
static bool _GenericListDebug(void);
static void GenericList_SetElementSize(GenericList *self, const size_t newSize);
static void GenericList_SetListSize(GenericList *self, const int numElements);
static void GenericList_Set(GenericList *self, const void *newELements, const int numElements);
static void GenericList_SetAt(GenericList *self, const void *newElements, const int numElements, const int index);
static void GenericList_Add(GenericList *self, const void *newElement, const int numElements);
static void GenericList_AddAt(GenericList *self, const void *newElement, const int numElements, int index);
static void GenericList_CopyOtherBetween(GenericList *self, const GenericList *other, const int startIndex, const int EndIndex);
static void* GenericList_Get(const GenericList *self, const int index);
static bool GenericList_Contains(const GenericList *self, const void *token);
static int GenericList_GetFirstIndexOf(const GenericList *self, const void *token);
static int GenericList_GetLastIndexOf(const GenericList *self, const void *token);
static void GenericList_Remove(GenericList *self, const void *token);
static void GenericList_RemoveAt(GenericList *self, const int index);
static void GenericList_RemoveBetween(GenericList *self, const int startIndex, const int endIndex);
static void GenericList_TrimToSize(GenericList *self, const int numElements);
static void GenericList_Clear(GenericList *self);
static bool GenericList_IsEmpty(const GenericList *self);
static bool GenericList_Equals(const GenericList *self, const GenericList *other);
static bool GenericList_IsInitalized(const GenericList *self);
static bool GenericList_ResizeMemoryBlock(GenericList *self, const int newLen);
static bool GenericList_NewMemoryBlock(GenericList *self, const int newLen);
static void* GenericList_GetPointerToLocation(const GenericList *self, const int index);
static bool GenericList_AreListsCompatible(const GenericList *self, const GenericList *other);

static const struct GenericListMethods genericListMethods={
	.setElementSize=&GenericList_SetElementSize,
	.setListSize=&GenericList_SetListSize,
	.set=&GenericList_Set,
	.setAt=&GenericList_SetAt,
	.add=&GenericList_Add,
	.addAt=&GenericList_AddAt,
	.copyOtherBetween=GenericList_CopyOtherBetween,
	.get=&GenericList_Get,
	.contains=&GenericList_Contains,
	.getFirstIndexOf=&GenericList_GetFirstIndexOf,
	.getLastIndexOf=&GenericList_GetLastIndexOf,
	.remove=GenericList_Remove,
	.removeAt=GenericList_RemoveAt,
	.removeBetween=GenericList_RemoveBetween,
	.trimToSize=&GenericList_TrimToSize,
	.clear=&GenericList_Clear,
	.isEmpty=&GenericList_IsEmpty,
	.equals=&GenericList_Equals
};

const struct GenericList_t GenericListClass={
	.new=_GenericListConstructor,
	.delete=_GenericListDestructor,
	.print=_GenericListPrint,
	.debug=_GenericListDebug,
	.objSize=sizeof(GenericList)
};

//Object Methods================================================================
//Public methods
static void GenericList_SetElementSize(GenericList *self, const size_t newSize){
	if (newSize!=self->elementSize){
		if (!self->methods->isEmpty(self)){
			self->methods->clear(self);
		}
		self->elementSize=newSize;
	}
}

static void GenericList_SetListSize(GenericList *self, const int numElements){
	if (GenericList_IsInitalized(self)){
		if (GenericList_NewMemoryBlock(self,numElements)){
			self->numElements=numElements;
		}
	}
}

static void GenericList_Set(GenericList *self, const void *newElements, const int numElements){
	if (GenericList_IsInitalized(self)){
		if (GenericList_NewMemoryBlock(self,numElements)){
			memcpy(self->list,newElements,self->elementSize*numElements);
			self->numElements=numElements;
		}
	}
}

static void GenericList_SetAt(GenericList *self, const void *newElements, const int numElements, const int index){
	if (GenericList_IsInitalized(self) && !self->methods->isEmpty(self) &&
	    index>=0 && index<self->numElements){
		void *destination=GenericList_GetPointerToLocation(self,index);
		int numCopieableElements=(numElements+index<self->numElements)? numElements: self->numElements-index;
		memcpy(destination,newElements,self->elementSize*numCopieableElements);
	} else if (self->methods->isEmpty(self)){
		self->methods->set(self,newElements,numElements);
	}
}

static void GenericList_Add(GenericList *self, const void *newElements, const int numElements){
	if (GenericList_IsInitalized(self) && 
	    GenericList_ResizeMemoryBlock(self,self->numElements+numElements)){
		self->numElements+=numElements;
		void *temp=GenericList_GetPointerToLocation(self,self->numElements-numElements);
		memmove(temp,newElements,self->elementSize*numElements);
	}
}

static void GenericList_AddAt(GenericList *self, const void *newElements, const int numElements, const int index){
	if (GenericList_IsInitalized(self) && !self->methods->isEmpty(self) && 
	    index>=0 && index<self->numElements){
		if (GenericList_ResizeMemoryBlock(self,self->numElements+numElements)){
			self->numElements+=numElements;
			void *source=GenericList_GetPointerToLocation(self,index);
			void *destination=GenericList_GetPointerToLocation(self,index+numElements);
			int size=self->elementSize*(self->numElements-index-1);
			memmove(destination,source,size);
			memcpy(source,newElements,self->elementSize*numElements);
		}
	} else if (self->methods->isEmpty(self) || index==self->numElements){
		self->methods->add(self,newElements,numElements);
	}
}

static void GenericList_CopyOtherBetween(GenericList *self, const GenericList *other, const int startIndex, const int endIndex){
	if (GenericList_AreListsCompatible(self,other) && !other->methods->isEmpty(other)){
		int start=(startIndex<0)? 0: startIndex;
		int end=(endIndex>other->numElements)? other->numElements: endIndex;
		if (end>start && GenericList_NewMemoryBlock(self,end-start)){
			void *source=GenericList_GetPointerToLocation(other,start);
			if (source!=NULL){
				memcpy(self->list,source,self->elementSize*(end-start));
				self->numElements=end-start;
			}
		}
	}
}

static void* GenericList_Get(const GenericList *self, const int index){
	if (!self->methods->isEmpty(self) && GenericList_IsInitalized(self)){
		return GenericList_GetPointerToLocation(self,index);
	}
	return NULL;
}

static bool GenericList_Contains(const GenericList *self, const void *token){
	bool rv=false;
	for (int i=0; i<self->numElements && !rv; i++){
		void *iterElement=GenericList_GetPointerToLocation(self,i);
		if (memcmp(iterElement,token,self->elementSize)==0){
			rv=true;
		}
	}
	return rv;
}

static int GenericList_GetFirstIndexOf(const GenericList *self, const void *token){
	int rv=-1;
	for (int i=0; i<self->numElements && rv==-1; i++){
		void *iterElement=GenericList_GetPointerToLocation(self,i);
		if (memcmp(iterElement,token,self->elementSize)==0){
			rv=i;
		}
	}
	return rv;
}

static int GenericList_GetLastIndexOf(const GenericList *self, const void *token){
	int rv=-1;
	for (int i=self->numElements-1; i>=0 && rv==-1; i--){
		void *iterElement=GenericList_GetPointerToLocation(self,i);
		if (memcmp(iterElement,token,self->elementSize)==0){
			rv=i;
		}
	}
	return rv;
}

static void GenericList_Remove(GenericList *self, const void *token){
	int offset=0;
	if (GenericList_IsInitalized(self) && !self->methods->isEmpty(self)){
		for (int i=0; i<self->numElements; i++){
			void *iterElement=GenericList_GetPointerToLocation(self,i);
			if (memcmp(iterElement,token,self->elementSize)!=0){
				void *destination=GenericList_GetPointerToLocation(self,i-offset);
				memmove(destination,iterElement,self->elementSize);
			} else {
				offset++;
			}
		}
		GenericList_ResizeMemoryBlock(self,self->numElements-offset);
		self->numElements-=offset;
	}
}

static void GenericList_RemoveAt(GenericList *self, const int index){
	if (!self->methods->isEmpty(self) && GenericList_IsInitalized(self) &&
	    index>=0 && index<self->numElements){
		void *destination=GenericList_GetPointerToLocation(self,index);
		void *source=GenericList_GetPointerToLocation(self,index+1);
		int size=self->elementSize*(self->numElements-index);
		if (destination!=NULL && source!=NULL){
			memmove(destination,source,size);
		}
		GenericList_ResizeMemoryBlock(self,self->numElements-1);
		self->numElements--;
	}
}

static void GenericList_RemoveBetween(GenericList *self, const int startIndex, const int endIndex){
	if (GenericList_IsInitalized(self) && !self->methods->isEmpty(self) &&
	    startIndex>=0 && startIndex<endIndex && endIndex<=self->numElements){
		void *destination=GenericList_GetPointerToLocation(self,startIndex);
		void *source=GenericList_GetPointerToLocation(self,endIndex);
		int size=self->elementSize*(self->numElements-endIndex);
		if (destination!=NULL && source!=NULL){
			memmove(destination,source,size);
		}
		GenericList_ResizeMemoryBlock(self,self->numElements-(endIndex-startIndex));
		self->numElements-=(endIndex-startIndex);
	}
}

static void GenericList_TrimToSize(GenericList *self, const int numElements){
	if (!self->methods->isEmpty(self) && GenericList_IsInitalized(self)){
		int newSize=(numElements<self->numElements)? numElements: self->numElements;
		if (newSize!=self->numElements && GenericList_ResizeMemoryBlock(self,newSize)){
			self->numElements=newSize;
		}
	}
}

static void GenericList_Clear(GenericList *self){
	if (!self->methods->isEmpty(self)){
		if (self->list!=NULL){
			free(self->list);
			self->list=NULL;
		}
		self->numElements=0;
	}
}

static bool GenericList_IsEmpty(const GenericList *self){
	if (self->list==NULL || self->numElements==0){
		return true;
	}
	return false;
}

static bool GenericList_Equals(const GenericList *self, const GenericList *other){
	bool rv=false;
	if (GenericList_AreListsCompatible(self,other) && self->numElements==other->numElements &&
	    self->list!=NULL && other->list!=NULL){
		int size=self->numElements*self->elementSize;
		if (memcmp(self->list,other->list,size)==0){
			rv=true;
		}
	}
	return rv;
}

//Private Methods
static bool GenericList_IsInitalized(const GenericList *self){
	return (self->elementSize>0)? true: false;
}

static bool GenericList_ResizeMemoryBlock(GenericList *self, const int newLen){
	void *reallocPntr=realloc(self->list,self->elementSize*newLen);
	if (reallocPntr!=NULL){
		self->list=reallocPntr;
		return true;
	} else {
		self->list=NULL;
	}
	return false;
}

static bool GenericList_NewMemoryBlock(GenericList *self, const int newLen){
	if (self->list!=NULL){
		free(self->list);
	}
	void *mallocPntr=malloc(self->elementSize*newLen);
	if (mallocPntr!=NULL){
		self->list=mallocPntr;
		return true;
	}
	return false;
}

static void* GenericList_GetPointerToLocation(const GenericList *self, const int index){
	char *tempList=(char *)self->list;
	if (index<self->numElements){
		return (void*)(tempList+(self->elementSize*index));
	}
	return NULL;
}

static bool GenericList_AreListsCompatible(const GenericList *self, const GenericList *other){
	bool compatibile=true;
	(!GenericList_IsInitalized(self))? compatibile=false: 0;
	(!GenericList_IsInitalized(other))? compatibile=false: 0;
	(self->elementSize!=other->elementSize)? compatibile=false: 0;
	return compatibile;
}

//Class Methods=================================================================
static GenericList* _GenericListConstructor(void){
	GenericList *rv=createObject(GenericListClass.objSize);
	rv->list=NULL;
	rv->elementSize=0;
	rv->numElements=0;
	rv->methods=&genericListMethods;
	return rv;
}

static void _GenericListPrint(const GenericList *obj){
	PrintClass.print("<GenericList Obj[Addr: %p]: NumElements: %d  ElementSize: %d>\n",
			obj,obj->numElements,obj->elementSize);
}

static bool _GenericListDebug(void){
	bool successfull=true;
	char *temp="FUCK!Why is the code not working?";
	GenericList *test=GenericListClass.new();
	(!test->methods->isEmpty(test))? successfull=false: 0;
	test->methods->setElementSize(test,sizeof(char));
	test->methods->add(test,(temp+0),1);
	test->methods->add(test,(temp+1),1);
	test->methods->add(test,(temp+3),1);
	test->methods->add(test,(temp+4),1);
	test->methods->addAt(test,(temp+2),1,2);
	test->methods->get(test,0);
	test->methods->get(test,1);
	test->methods->get(test,2);
	test->methods->get(test,3);
	test->methods->get(test,4);
	(test->methods->isEmpty(test))? successfull=false: 0;
	(test->numElements!=5)? successfull=false: 0;
	(*(char*)test->methods->get(test,0)!=*temp)? successfull=false: 0;
	(*(char*)test->methods->get(test,2)!=*(temp+2))? successfull=false: 0;
	(!test->methods->contains(test,(temp+4)))? successfull=false: 0;
	(test->methods->getFirstIndexOf(test,(temp+2))!=2)? successfull=false: 0;
	(test->methods->getLastIndexOf(test,temp)!=0)? successfull=false: 0;
	test->methods->trimToSize(test,4);
	(test->numElements!=4)? successfull=false: 0;
	GenericList *test2=GenericListClass.new();
	test2->methods->setElementSize(test2,sizeof(char));
	test2->methods->set(test2,temp,4);
	(!test->methods->equals(test,test2))? successfull=false: 0;
	test2->methods->removeAt(test2,3);
	(test2->numElements!=3)? successfull=false: 0;
	test2->methods->clear(test2);
	(test2->numElements!=0)? successfull=false: 0;
	test2->methods->copyOtherBetween(test2,test,2,4);
	(test2->numElements!=2)? successfull=false: 0;
	test2->methods->setAt(test2,(void*)temp,2,0);
	(*(char*)test2->methods->get(test2,0)!=*temp)? successfull=false: 0;
	GenericList *test3=GenericListClass.new();
	test3->methods->setElementSize(test3,sizeof(char));
	test3->methods->set(test3,temp,strlen(temp));
	test3->methods->remove(test3,(temp+17));
	(test3->numElements!=30)? successfull=false: 0;
	test3->methods->removeBetween(test3,5,29);
	(test3->numElements!=6)? successfull=false: 0;
	//for (int i=0; i<test3->numElements; i++){
	//	printf("element[%d]: %c\n",i,*(char *)test3->methods->get(test3,i));
	//}
	//GenericListClass.print(test);
	//GenericListClass.print(test2);
	//GenericListClass.print(test3);
	GenericListClass.delete(&test);
	GenericListClass.delete(&test2);
	GenericListClass.delete(&test3);
	PrintClass.objectDebug("GenericList",successfull);
	return successfull;
}

static void _GenericListDestructor(GenericList **obj){
	free((*obj)->list);
	deleteObject((void**)obj);
}
