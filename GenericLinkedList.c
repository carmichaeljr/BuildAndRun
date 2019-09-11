#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GenericLinkedList.h"
#include "Object.h"
#include "Print.h"

static GenericLinkedList* _GenericLinkedListConstructor(void);
static void _GenericLinkedListDestructor(GenericLinkedList **obj);
static void _GenericLinkedListPrint(const GenericLinkedList *obj);
static bool _GenericLinkedListDebug(void);
static bool _GenericLinkedListDebug_Add(double *heapFloat);
static bool _GenericLinkedListDebug_Remove(double *heapFloat);
static bool _GenericLinkedListDebug_Set(double *heapFloat);
static bool _GenericLinkedListDebug_Search(double *heapFloat);
static bool _GenericLinkedListDebug_Misc(double *heapFloat);
static void GLL_SetElementSize(GenericLinkedList *self, const size_t newSize);
static void GLL_Set(GenericLinkedList *self, const void *newElements, const int numElements);
static void GLL_SetAt(GenericLinkedList *self, const void *newElements, const int numElements, const int index);
static void GLL_Add(GenericLinkedList *self, const void *newElements, const int numElements);
static void GLL_AddAt(GenericLinkedList *self, const void *newElements, const int numElements, const int index);
static void GLL_CopyOtherBetween(GenericLinkedList *self, const GenericLinkedList *other, const int startIndex, const int endIndex);
static void* GLL_Get(const GenericLinkedList *self, const int index);
static bool GLL_Contains(const GenericLinkedList *self, const void *token);
static int GLL_GetFirstIndexOf(const GenericLinkedList *self, const void *token);
static int GLL_GetLastIndexOf(const GenericLinkedList *self, const void *token);
static void GLL_Remove(GenericLinkedList *self, const void *token);
static void GLL_RemoveAt(GenericLinkedList *self, const int index);
static void GLL_RemoveBetween(GenericLinkedList *self, const int startIndex, const int endIndex);
static void GLL_TrimToSize(GenericLinkedList *self, const int  numElements);
static void GLL_Clear(GenericLinkedList *self);
static bool GLL_IsEmpty(const GenericLinkedList *self);
static bool GLL_Equals(const GenericLinkedList *self, const GenericLinkedList *other);
static bool GLL_IsInitalized(const GenericLinkedList *self);
static void* GLL_GetPointerToElement(const GenericLinkedList *self, const void *newElements, const int index);
static struct GenericLinkedListNode* GLL_CreateNode(const void *element);
static struct GenericLinkedListNode* GLL_GetNode(const GenericLinkedList *self, const int index);
static bool GLL_AreListsCompatible(const GenericLinkedList *self, const GenericLinkedList *other);

static const struct GenericLinkedListMethods genericLinkedListMethods={
	.setElementSize=&GLL_SetElementSize,
	.set=&GLL_Set,
	.setAt=&GLL_SetAt,
	.add=&GLL_Add,
	.addAt=&GLL_AddAt,
	.get=&GLL_Get,
	.contains=&GLL_Contains,
	.getFirstIndexOf=&GLL_GetFirstIndexOf,
	.getLastIndexOf=&GLL_GetLastIndexOf,
	.remove=&GLL_Remove,
	.removeAt=&GLL_RemoveAt,
	.removeBetween=&GLL_RemoveBetween,
	.trimToSize=&GLL_TrimToSize,
	.copyOtherBetween=&GLL_CopyOtherBetween,
	.clear=&GLL_Clear,
	.isEmpty=&GLL_IsEmpty,
	.equals=&GLL_Equals,
};

const struct GenericLinkedList_t GenericLinkedListClass={
	.new=_GenericLinkedListConstructor,
	.delete=_GenericLinkedListDestructor,
	.print=_GenericLinkedListPrint,
	.debug=_GenericLinkedListDebug,
	.objSize=sizeof(GenericLinkedList)
};

//Object Methods================================================================
//Public Methods
static void GLL_SetElementSize(GenericLinkedList *self, const size_t newSize){
	if (newSize!=self->elementSize){
		if (!self->methods->isEmpty(self)){
			self->methods->clear(self);
		}
		self->elementSize=newSize;
	}
}

static void GLL_Set(GenericLinkedList *self, const void *newElements, const int numElements){
	if (GLL_IsInitalized(self) && !self->methods->isEmpty(self)){
		self->methods->setAt(self,newElements,numElements,0);
	} else if (GLL_IsInitalized(self) && self->methods->isEmpty(self)){
		self->methods->add(self,newElements,numElements);
	}
}

static void GLL_SetAt(GenericLinkedList *self, const void *newElements, const int numElements, const int index){
	if (GLL_IsInitalized(self) && !self->methods->isEmpty(self) && 
	    index>=0 && index<self->numElements){
		int minLen=(numElements<self->numElements-index)? numElements: self->numElements-index;
		for (int i=0; i<minLen; i++){
			void *tempElement=GLL_GetPointerToElement(self,newElements,i);
			struct GenericLinkedListNode *temp=GLL_GetNode(self,index+i);
			temp->element=tempElement;
		}
		if (numElements+index>self->numElements){
			void *tempElements=GLL_GetPointerToElement(self,newElements,minLen);
			self->methods->add(self,tempElements,numElements-minLen);
		} 
	}
}

static void GLL_Add(GenericLinkedList *self, const void *newElements, const int numElements){
	if (GLL_IsInitalized(self)){
		for (int i=0; i<numElements; i++){
			void *tempElement=GLL_GetPointerToElement(self,newElements,i);
			struct GenericLinkedListNode *tempNode=GLL_CreateNode(tempElement);
			if (self->tail!=NULL){
				tempNode->previous=self->tail;
				self->tail->next=tempNode;
			}
			(self->head==NULL && self->numElements<1)? self->head=tempNode: NULL;
			self->tail=tempNode;
			self->numElements++;
		}
	}
}

static void GLL_AddAt(GenericLinkedList *self, const void *newElements, const int numElements, const int index){
	if (GLL_IsInitalized(self) && index<self->numElements && index>=0){
		for (int i=0; i<numElements; i++){
			void *tempElement=GLL_GetPointerToElement(self,newElements,i);
			struct GenericLinkedListNode *new=GLL_CreateNode(tempElement);
			struct GenericLinkedListNode *startNode=GLL_GetNode(self,index+i-1);
			struct GenericLinkedListNode *endNode=GLL_GetNode(self,index+i);
			if (startNode!=NULL){
				new->previous=startNode;
				startNode->next=new;
			} else {
				self->head=new;
			}
			if (endNode!=NULL){
				new->next=endNode;
				endNode->previous=new;
			} else {
				self->tail=new;
			}
			self->numElements++;
		}
	} else if (GLL_IsInitalized(self) && index==self->numElements){
		self->methods->add(self,newElements,numElements);
	}
}

static void GLL_CopyOtherBetween(GenericLinkedList *self, const GenericLinkedList *other, const int startIndex, const int endIndex){
	if (GLL_IsInitalized(self) && GLL_IsInitalized(other) && 
	    GLL_AreListsCompatible(self,other)){
		int start=(startIndex<0)? 0: startIndex;
		int end=(endIndex>other->numElements)? other->numElements: endIndex;
		if (end>start){
			self->methods->clear(self);
			for (int i=start; i<end; i++){
				self->methods->add(self,other->methods->get(other,i),1);
			}
		}
	}
}

static void* GLL_Get(const GenericLinkedList *self, const int index){
	struct GenericLinkedListNode* rv=GLL_GetNode(self,index);
	return (rv!=NULL)? rv->element: NULL;
}

static bool GLL_Contains(const GenericLinkedList *self, const void *token){
	int rv=false;
	for (int i=0; i<self->numElements && !rv; i++){
		struct GenericLinkedListNode *node=GLL_GetNode(self,i);
		if (memcmp(node->element,token,self->elementSize)==0){
			rv=true;
		}
	}
	return rv;
}

static int GLL_GetFirstIndexOf(const GenericLinkedList *self, const void *token){
	int rv=-1;
	for (int i=0; i<self->numElements && rv==-1; i++){
		struct GenericLinkedListNode *node=GLL_GetNode(self,i);
		if (memcmp(node->element,token,self->elementSize)==0){
			rv=i;
		}
	}
	return rv;
}

static int GLL_GetLastIndexOf(const GenericLinkedList *self, const void *token){
	int rv=-1;
	for (int i=self->numElements-1; i>=0; i--){
		struct GenericLinkedListNode *node=GLL_GetNode(self,i);
		if (memcmp(node->element,token,self->elementSize)==0){
			rv=i;
		}
	}
	return rv;
}

static void GLL_Remove(GenericLinkedList *self, const void *token){
	if (GLL_IsInitalized(self) && !self->methods->isEmpty(self)){
		//The number of elements is updated in the call to removeAt
		for (int i=0; i<self->numElements; i++){
			struct GenericLinkedListNode *node=GLL_GetNode(self,i);
			if (memcmp(node->element,token,self->elementSize)==0){
				self->methods->removeAt(self,i);
				i--;
			}
		}
	}
}

static void GLL_RemoveAt(GenericLinkedList *self, const int index){
	if (GLL_IsInitalized(self) && index>=0 && index<self->numElements){
		self->methods->removeBetween(self,index,index+1);
	}
}

static void GLL_RemoveBetween(GenericLinkedList *self, const int startIndex, const int endIndex){
	if (GLL_IsInitalized(self) && startIndex>=0 && endIndex<=self->numElements &&
		startIndex<endIndex){
		struct GenericLinkedListNode *startNode=GLL_GetNode(self,startIndex-1);
		struct GenericLinkedListNode *endNode=GLL_GetNode(self,endIndex);
		struct GenericLinkedListNode *temp=NULL;
		for (int i=endIndex-1; i>=startIndex; i--){
			temp=GLL_GetNode(self,i);
			if (temp!=NULL){
				free(temp);
			}
		}
		(endNode!=NULL)? (endNode->previous=startNode): (self->tail=startNode);
		(startNode!=NULL)? (startNode->next=endNode): (self->head=endNode);
		self->numElements-=(endIndex-startIndex);
	}
}

static void GLL_TrimToSize(GenericLinkedList *self, const int  numElements){
	if (GLL_IsInitalized(self) && !self->methods->isEmpty(self)){
		int newSize=(numElements<self->numElements)? numElements: self->numElements;
		if (newSize!=self->numElements){
			self->methods->removeBetween(self,newSize,self->numElements);
		}
	}
}

static void GLL_Clear(GenericLinkedList *self){
	if (GLL_IsInitalized(self)){
		struct GenericLinkedListNode *node=GLL_GetNode(self,0);
		struct GenericLinkedListNode *next=NULL;
		while (node!=NULL){
			next=node->next;
			free(node);
			node=next;
		}
		self->head=NULL;
		self->tail=NULL;
		self->numElements=0;
	}
}

static bool GLL_IsEmpty(const GenericLinkedList *self){
	return (self->numElements==0)? true: false;
}

static bool GLL_Equals(const GenericLinkedList *self, const GenericLinkedList *other){
	bool rv=false;
	if (GLL_IsInitalized(self) && GLL_IsInitalized(other) &&
	    GLL_AreListsCompatible(self,other)){
		rv=true;
		for (int i=0; i<self->numElements && rv; i++){
			struct GenericLinkedListNode *selfNode=GLL_GetNode(self,i);
			struct GenericLinkedListNode *otherNode=GLL_GetNode(other,i);
			if (memcmp(selfNode->element,otherNode->element,self->elementSize)!=0){
				rv=false;
			}
		}
	}
	return rv;
}

//Private Methods
static bool GLL_IsInitalized(const GenericLinkedList *self){
	return (self->elementSize>0)? true: false;
}

static void* GLL_GetPointerToElement(const GenericLinkedList *self, const void *newElements, const int index){
	return (void*)((char*)newElements+(self->elementSize*index));
}

static struct GenericLinkedListNode* GLL_CreateNode(const void *element){
	struct GenericLinkedListNode *node=NULL;
	while (node==NULL){
		node=malloc(sizeof(struct GenericLinkedListNode));
	}
	node->element=(void*)element;
	node->previous=NULL;
	node->next=NULL;
	return node;
}

static struct GenericLinkedListNode* GLL_GetNode(const GenericLinkedList *self, const int index){
	struct GenericLinkedListNode *rv=NULL;
	if (GLL_IsInitalized(self) && index<self->numElements && index>=0){
		for (int i=0; i<index+1; i++){
			//Can set rv to head if null because index is checked above for max length
			rv=(rv!=NULL)? rv->next: self->head;
		}
	}
	return rv;
}

static bool GLL_AreListsCompatible(const GenericLinkedList *self, const GenericLinkedList *other){
	bool compatible=true;
	(!GLL_IsInitalized(self))? compatible=false: 0;
	(!GLL_IsInitalized(other))? compatible=false: 0;
	(other->elementSize!=self->elementSize)? compatible=false: 0;
	return compatible;
}

//Class Methods=================================================================
static GenericLinkedList* _GenericLinkedListConstructor(void){
	GenericLinkedList *rv=createObject(GenericLinkedListClass.objSize);
	rv->methods=&genericLinkedListMethods;
	rv->elementSize=0;
	rv->numElements=0;
	rv->head=NULL;
	rv->tail=NULL;
	return rv;
}

static void _GenericLinkedListPrint(const GenericLinkedList *obj){
	PrintClass.print("<GenericLinkedList Obj[Addr: %p]: NumElements: %d  ElementSize: %d>\n",
			obj,obj->numElements,obj->elementSize);
	PrintClass.incrementIndentLevel(1);
	for (int i=0; i<obj->numElements; i++){
		PrintClass.print(". [%d] Element[Addr: %p]  Data[Addr: %p]\n",
				i,GLL_GetNode(obj,i),obj->methods->get(obj,i));
	}
	PrintClass.incrementIndentLevel(-1);
}

static bool _GenericLinkedListDebug(void){
	bool successfull=true;
	double heapFloat[]={
		3.141595,1234.0,9999.99,0.00001,6.66
	};
	successfull&=_GenericLinkedListDebug_Add(heapFloat);
	successfull&=_GenericLinkedListDebug_Remove(heapFloat);
	successfull&=_GenericLinkedListDebug_Set(heapFloat);
	successfull&=_GenericLinkedListDebug_Search(heapFloat);
	successfull&=_GenericLinkedListDebug_Misc(heapFloat);
	PrintClass.objectDebug("GenericLinkedList",successfull);
	return successfull;
}

static bool _GenericLinkedListDebug_Add(double *heapFloat){
	bool rv=true;
	GenericLinkedList *test=GenericLinkedListClass.new();
	test->methods->setElementSize(test,sizeof(double));
	for (int i=0; i<5; i++){
		test->methods->add(test,heapFloat+i,1);
	}
	test->methods->addAt(test,heapFloat,1,1);
	test->methods->addAt(test,heapFloat,1,5);
	test->methods->addAt(test,heapFloat+4,1,0);
	(test->elementSize!=sizeof(double))? rv=false: 0;
	(test->numElements!=8)? rv=false: 0;
	(*(double*)test->methods->get(test,1)!=*heapFloat)? rv=false: 0;
	(*(double*)test->methods->get(test,2)!=*heapFloat)? rv=false: 0;
	(*(double*)test->methods->get(test,0)!=*(heapFloat+4))? rv=false: 0;
	(*(double*)test->methods->get(test,6)!=*heapFloat)? rv=false: 0;
	GenericLinkedListClass.delete(&test);
	return rv;
}

static bool _GenericLinkedListDebug_Remove(double *heapFloat){
	bool rv=true;
	GenericLinkedList *test=GenericLinkedListClass.new();
	test->methods->setElementSize(test,sizeof(double));
	for (int i=0; i<5; i++){
		test->methods->add(test,heapFloat+i,1);
		test->methods->add(test,heapFloat+i,1);
	}
	test->methods->removeAt(test,0);
	test->methods->removeAt(test,5);
	(test->numElements!=8)? rv=false: 0;
	(*(double*)test->methods->get(test,0)!=*heapFloat)? rv=false: 0;
	(*(double*)test->methods->get(test,5)!=*(heapFloat+3))? rv=false: 0;
	test->methods->removeBetween(test,3,5);
	(test->numElements!=6)? rv=false: 0;
	(*(double*)test->methods->get(test,3)!=*(heapFloat+3))? rv=false: 0;
	test->methods->removeBetween(test,test->numElements-1,test->numElements);
	(test->numElements!=5)? rv=false: 0;
	(*(double*)test->methods->get(test,test->numElements-1)!=*(heapFloat+4))? rv=false: 0;
	test->methods->remove(test,(void*)(heapFloat+1));
	(test->numElements!=3)? rv=false: 0;
	(*(double*)test->methods->get(test,0)!=*heapFloat)? rv=false: 0;
	(*(double*)test->methods->get(test,2)!=*(heapFloat+4))? rv=false: 0;
	(*(double*)test->methods->get(test,1)!=*(heapFloat+3))? rv=false: 0;
	GenericLinkedListClass.delete(&test);
	return rv;
}

static bool _GenericLinkedListDebug_Set(double *heapFloat){
	bool rv=true;
	GenericLinkedList *test=GenericLinkedListClass.new();
	test->methods->setElementSize(test,sizeof(double));
	for (int i=0; i<5; i++){
		test->methods->add(test,heapFloat+i,1);
		test->methods->add(test,heapFloat+i,1);
	}
	test->methods->set(test,heapFloat,5);
	(test->numElements!=10)? rv=false: 0;
	(*(double*)test->methods->get(test,0)!=*heapFloat)? rv=false: 0;
	(*(double*)test->methods->get(test,4)!=*(heapFloat+4))? rv=false: 0;
	test->methods->setAt(test,heapFloat,5,1);
	(test->numElements!=10)? rv=false: 0;
	(*(double*)test->methods->get(test,1)!=*heapFloat)? rv=false: 0;
	(*(double*)test->methods->get(test,5)!=*(heapFloat+4))? rv=false: 0;
	GenericLinkedListClass.delete(&test);
	return rv;
}

static bool _GenericLinkedListDebug_Search(double *heapFloat){
      	bool rv=true;
	//double **secondHeapFloat=malloc(sizeof(double*)*5);
	//*secondHeapFloat=(heapFloat);
	//*(secondHeapFloat+1)=(heapFloat+1);
	//*(secondHeapFloat+2)=(heapFloat+2);
	//*(secondHeapFloat+3)=(heapFloat+3);
	//*(secondHeapFloat+4)=(heapFloat+4);
	double secondHeapFloat[]={
		3.141595,1234.0,9999.99,0.00001,6.66
	};
      	GenericLinkedList *test=GenericLinkedListClass.new();
	GenericLinkedList *test2=GenericLinkedListClass.new();
      	test->methods->setElementSize(test,sizeof(double));
	test2->methods->setElementSize(test2,sizeof(double));
	test->methods->add(test,heapFloat,5);
	test2->methods->add(test2,secondHeapFloat,5);
	(!test->methods->contains(test,(void*)secondHeapFloat))? rv=false: 0;
	(!test->methods->contains(test,(void*)(secondHeapFloat+4)))? rv=false: 0;
	(test->methods->getFirstIndexOf(test,(void*)(secondHeapFloat+2))!=2)? rv=false: 0;
	(test->methods->getFirstIndexOf(test,(void*)(secondHeapFloat+4))!=4)? rv=false: 0;
	(test->methods->getFirstIndexOf(test,(void*)secondHeapFloat)!=0)? rv=false: 0;
	(test->methods->getLastIndexOf(test,(void*)(secondHeapFloat+2))!=2)? rv=false: 0;
	(test->methods->getLastIndexOf(test,(void*)(secondHeapFloat+4))!=4)? rv=false: 0;
	(test->methods->getLastIndexOf(test,(void*)secondHeapFloat)!=0)? rv=false: 0;
	(!test->methods->equals(test,test2))? rv=false: 0;
      	GenericLinkedListClass.delete(&test);
	GenericLinkedListClass.delete(&test2);
      	return rv;
}

static bool _GenericLinkedListDebug_Misc(double *heapFloat){
	bool rv=true;
	GenericLinkedList *test=GenericLinkedListClass.new();
	GenericLinkedList *test2=GenericLinkedListClass.new();
	test->methods->setElementSize(test,sizeof(double));
	test2->methods->setElementSize(test2,sizeof(double));
	test->methods->add(test,heapFloat,5);
	test2->methods->copyOtherBetween(test2,test,2,5);
	(test2->numElements!=3)? rv=false: 0;
	(*(double*)test2->methods->get(test2,0)!=*(heapFloat+2))? rv=false: 0;
	(*(double*)test2->methods->get(test2,2)!=*(heapFloat+4))? rv=false: 0;
	test2->methods->copyOtherBetween(test2,test,0,3);
	(test2->numElements!=3)? rv=false: 0;
	(*(double*)test2->methods->get(test2,0)!=*heapFloat)? rv=false: 0;
	(*(double*)test2->methods->get(test2,2)!=*(heapFloat+2))? rv=false: 0;
	test->methods->add(test,heapFloat+1,4);
	test->methods->trimToSize(test,5);
	(test->numElements!=5)? rv=false: 0;
	(*(double*)test->methods->get(test,0)!=*heapFloat)? rv=false: 0;
	(*(double*)test->methods->get(test,4)!=*(heapFloat+4))? rv=false: 0;
	GenericLinkedListClass.delete(&test);
	GenericLinkedListClass.delete(&test2);
	return rv;
}

static void _GenericLinkedListDestructor(GenericLinkedList **obj){
	(*obj)->methods->clear((*obj));
	deleteObject((void**)obj);
}
