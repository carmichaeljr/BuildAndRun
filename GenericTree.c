#include <string.h>
#include <stdbool.h>
#include "GenericTree.h"
#include "GenericLinkedList.h"
#include "Object.h"
#include "Print.h"


static GenericTree* _GenericTreeConstructor(void);
static void _GenericTreePrintObj(GenericTree *obj);
static bool _GenericTreeDebug(void);
static bool _GenericTreeDebugAdd(const int *vals);
static bool _GenericTreeDebugRemove(const int *vals);
static bool _GenericTreeDebugSet(const int *vals);
static bool _GenericTreeDebugSearch(const int *vals);
static bool _GenericTreeDebugPushQue(const int*vals);
static bool _GenericTreeDebugCopy(const int *vals);
static bool _GenericTreeDebugMisc(const int *vals);
static void _GenericTreeDestructor(GenericTree **obj);
static void GenericTree_SetElementSize(GenericTree *self, const size_t size);
static bool GenericTree_PlaceCurrentNode(GenericTree *self, const void *dataList, const int numNodes);
static void GenericTree_PlaceCurrentNodeAtHeadNode(GenericTree *self);
static bool GenericTree_MoveDown(GenericTree *self);
static bool GenericTree_MoveNext(GenericTree *self);
static bool GenericTree_MoveUp(GenericTree *self);
static void GenericTree_Add(GenericTree *self, const void *data);
static void GenericTree_AddAt(GenericTree *self, const void *dataList, const int numNodes, const void *data);
static void GenericTree_Set(GenericTree *self, const void *data);
static void GenericTree_SetAt(GenericTree *self, const void *dataList, const int numNodes, const void *data);
static void* GenericTree_Get(const GenericTree *self);
static void* GenericTree_GetAt(GenericTree *self, const void *dataList, const int numNodes);
static int GenericTree_GetNumChildNodes(const GenericTree *self);
static int GenericTree_GetNumChildNodesAt(GenericTree *self, const void *dataList, const int numNodes);
static bool GenericTree_BranchExists(const GenericTree *self, const void *dataList, const int numNodes);
static void GenericTree_CopyOtherBelowCurNode(GenericTree *self, GenericTree *other);
static void GenericTree_Remove(GenericTree *self);
static void GenericTree_Clear(GenericTree *self);
static bool GenericTree_StartIter(GenericTree *self);
static bool GenericTree_StartIterAtCurNode(GenericTree *self);
static bool GenericTree_HasNext(const GenericTree *self);
static void* GenericTree_GetNext(GenericTree *self);
static char GenericTree_GetIterAction(const GenericTree *self);
static bool GenericTree_Equals(GenericTree *self, GenericTree *other);
static bool GenericTree_IsEmpty(const GenericTree *self);
static void GenericTree_PushCurrentNode(GenericTree *self);
static void* GenericTree_PopCurrentNode(GenericTree *self);
static void GenericTree_ClearPushedNodes(GenericTree *self);
static bool GenericTree_IsInitalized(const GenericTree *self);
static void* GenericTree_GetPointerToElement(const GenericTree *self, const void *newElements, const int index);
static struct GenericTreeNode* GenericTree_CreateNode(const GenericTree *self, const void *data);
static void GenericTree_DeleteNode(GenericTree *self, struct GenericTreeNode *node);
static struct GenericTreeNode* GenericTree_PathExistsBelow(const GenericTree *self, const struct GenericTreeNode *startNode, const void *data, const int numNodes);
static struct GenericTreeNode* GenericTree_DataInChildNodes(const GenericTree *self, const struct GenericTreeNode *node, const void *data);
static void GenericTree_InitalizeIterData(GenericTree *self, const struct GenericTreeNode *startNode);
static struct GenericTreeNode* GenericTree_GetNextNode(GenericTree *self);
static struct GenericTreeNode* GenericTree_CanTraverseDown(const GenericTree *self);
static struct GenericTreeNode* GenericTree_CanTraverseUp(const GenericTree *self);
static struct GenericTreeNode* GenericTree_CanTraverseNext(const GenericTree *self);
static bool GenericTree_AreTreesCompatible(const GenericTree *self, const GenericTree *other);

static const struct GenericTreeMethods genericTreeMethods={
	.setElementSize=&GenericTree_SetElementSize,
	.placeCurrentNode=&GenericTree_PlaceCurrentNode,
	.placeCurrentNodeAtHeadNode=&GenericTree_PlaceCurrentNodeAtHeadNode,
	.moveDown=&GenericTree_MoveDown,
	.moveNext=&GenericTree_MoveNext,
	.moveUp=&GenericTree_MoveUp,
	.add=&GenericTree_Add,
	.addAt=&GenericTree_AddAt,
	.set=&GenericTree_Set,
	.setAt=&GenericTree_SetAt,
	.get=&GenericTree_Get,
	.getAt=&GenericTree_GetAt,
	.getNumChildNodes=&GenericTree_GetNumChildNodes,
	.getNumChildNodesAt=&GenericTree_GetNumChildNodesAt,
	.branchExists=&GenericTree_BranchExists,
	.copyOtherBelowCurNode=&GenericTree_CopyOtherBelowCurNode,
	.remove=&GenericTree_Remove,
	.clear=&GenericTree_Clear,
	.startIter=&GenericTree_StartIter,
	.startIterAtCurNode=&GenericTree_StartIterAtCurNode,
	.hasNext=&GenericTree_HasNext,
	.getNext=&GenericTree_GetNext,
	.getIterAction=&GenericTree_GetIterAction,
	.equals=&GenericTree_Equals,
	.isEmpty=&GenericTree_IsEmpty,
	.pushCurrentNode=&GenericTree_PushCurrentNode,
	.popCurrentNode=&GenericTree_PopCurrentNode,
	.clearPushedNodes=&GenericTree_ClearPushedNodes,
};
const struct GenericTreeClass_t GenericTreeClass={
	.new=&_GenericTreeConstructor,
	.delete=&_GenericTreeDestructor,
	.print=&_GenericTreePrintObj,
	.debug=&_GenericTreeDebug,
	.objSize=sizeof(GenericTree)
};

//Object Methods===============================================================
//Public Methods
static void GenericTree_SetElementSize(GenericTree *self, const size_t size){
	if (self->elementSize!=size && size>0){
		self->methods->clear(self);
		self->elementSize=size;
	}
}

static bool GenericTree_PlaceCurrentNode(GenericTree *self, const void *dataList, const int numNodes){
	bool rv=false;
	struct GenericTreeNode *temp[2]={
		GenericTree_PathExistsBelow(self,self->currentNode,dataList,numNodes),
		NULL,
	};
	(temp[0]==NULL)? temp[1]=GenericTree_PathExistsBelow(self,self->headNode,dataList,numNodes): 0;
	for (int i=0; i<2 && !rv; i++){
		if (temp[i]!=NULL){
			self->currentNode=temp[i];
			rv=true;
		}
	}
	return rv;
}

static void GenericTree_PlaceCurrentNodeAtHeadNode(GenericTree *self){
	if (GenericTree_IsInitalized(self)){
		self->currentNode=self->headNode;
	}
}

static bool GenericTree_MoveDown(GenericTree *self){
	bool rv=false;
	if (GenericTree_IsInitalized(self) && 
	    self->currentNode->childNodes->numElements>0){
		GenericLinkedList *gll=self->currentNode->childNodes;
		self->currentNode=gll->methods->get(gll,0);
		rv=true;
	}
	return rv;
}

static bool GenericTree_MoveNext(GenericTree *self){
	bool rv=false;
	if (GenericTree_IsInitalized(self) && self->currentNode!=self->headNode){
		GenericLinkedList *pNodes=self->currentNode->parrentNode->childNodes;
		int index=pNodes->methods->getFirstIndexOf(pNodes,self->currentNode);
		if (index<pNodes->numElements-1){
			self->currentNode=pNodes->methods->get(pNodes,index+1);
			rv=true;
		}
	}
	return rv;
}

static bool GenericTree_MoveUp(GenericTree *self){
	bool rv=false;
	if (GenericTree_IsInitalized(self) && self->currentNode!=self->headNode){
		self->currentNode=self->currentNode->parrentNode;
		rv=true;
	}
	return rv;
}

static void GenericTree_Add(GenericTree *self, const void *data){
	if (GenericTree_IsInitalized(self)){
		struct GenericTreeNode *temp=GenericTree_CreateNode(self,data);
		if (self->headNode==NULL){
			self->headNode=temp;
			self->currentNode=self->headNode;
		} else if (self->currentNode!=NULL){
			GenericLinkedList *gll=self->currentNode->childNodes;
			gll->methods->add(gll,temp,1);
		}
		self->numNodes++;
	}
}

//Note - repeated calls to addAt is computationally intensive, to avoid this
//	use placeCurrentNode and further calls should use the add method
static void GenericTree_AddAt(GenericTree *self, const void *dataList, const int numNodes, const void *data){
	struct GenericTreeNode *prevCurrentNode=self->currentNode;
	if (GenericTree_IsInitalized(self) && self->methods->placeCurrentNode(self,dataList,numNodes)){
		struct GenericTreeNode *temp=GenericTree_CreateNode(self,data);
		GenericLinkedList *gll=self->currentNode->childNodes;
		gll->methods->add(gll,temp,1);
		self->currentNode=prevCurrentNode;
		self->numNodes++;
	}
}

static void GenericTree_Set(GenericTree *self, const void *data){
	if (GenericTree_IsInitalized(self) && self->currentNode!=NULL){
		self->currentNode->data=(void*)data;
	}
}

static void GenericTree_SetAt(GenericTree *self, const void *dataList, const int numNodes, const void *data){
	struct GenericTreeNode *prevCurrentNode=self->currentNode;
	if (GenericTree_IsInitalized(self) && self->methods->placeCurrentNode(self,dataList,numNodes)){
		self->currentNode->data=(void*)data;
		self->currentNode=prevCurrentNode;
	}
}

static void* GenericTree_Get(const GenericTree *self){
	if (GenericTree_IsInitalized(self) && self->currentNode!=NULL){
		return self->currentNode->data;
	}
	return NULL;
}

static void* GenericTree_GetAt(GenericTree *self, const void *dataList, const int numNodes){
	void *rv=NULL;
	struct GenericTreeNode *prevCurrentNode=self->currentNode;
	if (self->methods->placeCurrentNode(self,dataList,numNodes)){
		rv=self->currentNode->data;
		self->currentNode=prevCurrentNode;
	}
	return rv;
}

static int GenericTree_GetNumChildNodes(const GenericTree *self){
	if (GenericTree_IsInitalized(self) && self->currentNode!=NULL){
		return self->currentNode->childNodes->numElements;
	}
	return -1;
}

static int GenericTree_GetNumChildNodesAt(GenericTree *self, const void *dataList, const int numNodes){
	int rv=-1;
	struct GenericTreeNode *prevCurrentNode=self->currentNode;
	if (self->methods->placeCurrentNode(self,dataList,numNodes)){
		rv=self->currentNode->childNodes->numElements;
		self->currentNode=prevCurrentNode;
	}
	return rv;
}

static bool GenericTree_BranchExists(const GenericTree *self, const void *dataList, const int numNodes){
	bool rv=false;
	if (GenericTree_PathExistsBelow(self,self->currentNode,dataList,numNodes)!=NULL ||
	    GenericTree_PathExistsBelow(self,self->headNode,dataList,numNodes)!=NULL) {
		rv=true;
	}
	return rv;
}

static void GenericTree_CopyOtherBelowCurNode(GenericTree *self, GenericTree *other){
	self->methods->pushCurrentNode(self);
	bool runOpp=(GenericTree_IsInitalized(self) && GenericTree_IsInitalized(other));
	struct GenericTreeNode *temp=other->currentNode;
	runOpp&=GenericTree_AreTreesCompatible(self,other);
	runOpp&=other->methods->startIterAtCurNode(other);
	while (runOpp && other->methods->hasNext(other)){
		void *data=other->methods->getNext(other);
		if (temp!=other->currentNode && 
		    strchr("nu",other->iterData.previousAction)!=NULL){
			self->methods->moveUp(self);
		}
		if (temp!=other->currentNode && 
		    strchr("dn",other->iterData.previousAction)!=NULL){
			self->methods->add(self,data);
			self->methods->placeCurrentNode(self,data,1);
		}
	}
	self->methods->popCurrentNode(self);
}

static void GenericTree_Remove(GenericTree *self){
	if (GenericTree_IsInitalized(self) && self->currentNode!=NULL && 
	    self->methods->startIterAtCurNode(self)){
		while (self->methods->hasNext(self)){
			self->methods->getNext(self);
			GenericLinkedList *gll=self->iterData.iterNode->childNodes;
			for (int i=gll->numElements-1; i>=0; i--){
				struct GenericTreeNode *iterNode=gll->methods->get(gll,i);
				if (iterNode->childNodes->numElements==0){
					GenericTree_DeleteNode(self,iterNode);
					gll->methods->removeAt(gll,i);
				}
			}
		}
		struct GenericTreeNode *temp=self->currentNode;
		if (self->methods->moveUp(self)){
			GenericLinkedList *gll=self->currentNode->childNodes;
			gll->methods->remove(gll,temp);
		}
		GenericTree_DeleteNode(self,temp);
	}
}

static void GenericTree_Clear(GenericTree *self){
	if (GenericTree_IsInitalized(self) && self->headNode!=NULL){
		self->currentNode=self->headNode;
		self->methods->remove(self);
		if (self->numNodes==1 && self->headNode!=NULL){
			GenericTree_DeleteNode(self,self->headNode);
		}
		self->headNode=NULL;
		self->currentNode=NULL;
	}
}

static bool GenericTree_StartIter(GenericTree *self){
	if (GenericTree_IsInitalized(self) && self->headNode!=NULL && self->numNodes>0){
		GenericTree_InitalizeIterData(self,self->headNode);
		return true;
	} 
	return false;
}

static bool GenericTree_StartIterAtCurNode(GenericTree *self){
	if (GenericTree_IsInitalized(self) && self->headNode!=NULL && self->numNodes>0){
		GenericTree_InitalizeIterData(self,self->currentNode);
		return true;
	}
	return false;
}

static bool GenericTree_HasNext(const GenericTree *self){
	if (GenericTree_IsInitalized(self) && 
	    self->iterData.iterNode==self->iterData.startNode && self->iterData.previousAction=='u') {
		return false;
	}
	return true;
}

static void* GenericTree_GetNext(GenericTree *self){
	struct GenericTreeNode *temp=NULL;
	if (GenericTree_IsInitalized(self) && self->headNode!=NULL && self->numNodes>0){
		temp=GenericTree_GetNextNode(self);
		self->currentNode=self->iterData.iterNode;
	}
	return (temp!=NULL)? temp->data: temp;
}

static char GenericTree_GetIterAction(const GenericTree *self){
	return self->iterData.previousAction;
}

static bool GenericTree_Equals(GenericTree *self, GenericTree *other){
	struct GenericTreeNode *prevNodes[2]={
		self->currentNode,
		other->currentNode,
	};
	bool rv=(GenericTree_AreTreesCompatible(self,other) && 
		 self->numNodes==other->numNodes &&
		 other->methods->startIter(other) && 
		 self->methods->startIter(self))? true: false;
	while (rv && other->methods->hasNext(other) && self->methods->hasNext(self)){
		void *otherData=other->methods->getNext(other);
		void *selfData=self->methods->getNext(self);
		if (memcmp(otherData,selfData,self->elementSize)!=0){
			rv=false;
		}
	}
	self->currentNode=prevNodes[0];
	other->currentNode=prevNodes[1];
	return rv;
}

static bool GenericTree_IsEmpty(const GenericTree *self){
	return (self->numNodes>0)? false: true;
}

static void GenericTree_PushCurrentNode(GenericTree *self){
	if (self->currentNode!=NULL){
		self->pushQue->methods->addAt(self->pushQue,self->currentNode,1,0);
	}
}

static void* GenericTree_PopCurrentNode(GenericTree *self){
	struct GenericTreeNode *temp=NULL;
	if (self->pushQue->numElements>0){
		temp=(struct GenericTreeNode*)self->pushQue->methods->get(self->pushQue,0);
		self->pushQue->methods->removeAt(self->pushQue,0);
		self->currentNode=temp;
	}
	return (temp!=NULL)? temp->data: NULL;
}

static void GenericTree_ClearPushedNodes(GenericTree *self){
	self->pushQue->methods->clear(self->pushQue);
}

//Private Methods
static bool GenericTree_IsInitalized(const GenericTree *self){
	return (self->elementSize>0)? true: false;
}

static void* GenericTree_GetPointerToElement(const GenericTree *self, const void *newElements, const int index){
	return (void*)((char*)newElements+(self->elementSize*index));
}

static struct GenericTreeNode* GenericTree_CreateNode(const GenericTree *self, const void *data){
	struct GenericTreeNode *rv=NULL;
	while (rv==NULL){
		rv=(struct GenericTreeNode*)malloc(sizeof(struct GenericTreeNode));
	}
	rv->childNodes=GenericLinkedListClass.new();
	rv->childNodes->methods->setElementSize(rv->childNodes,sizeof(struct GenericTreeNode));
	rv->parrentNode=self->currentNode;
	rv->data=(void*)data;
	return rv;
}

static void GenericTree_DeleteNode(GenericTree *self, struct GenericTreeNode *node){
	GenericLinkedListClass.delete(&node->childNodes);
	free(node);
	self->numNodes--;
}

static struct GenericTreeNode* GenericTree_PathExistsBelow(const GenericTree *self, const struct GenericTreeNode *startNode, const void *data, const int numNodes){
	struct GenericTreeNode *iterNode=(struct GenericTreeNode*)startNode;
	if (GenericTree_IsInitalized(self) && self->headNode!=NULL && startNode!=NULL && data!=NULL && numNodes>0){
		void *iterData;
		for (int i=0; i<numNodes && iterNode!=NULL; i++){
			iterData=GenericTree_GetPointerToElement(self,data,i);
			iterNode=GenericTree_DataInChildNodes(self,iterNode,iterData);
		}
	}
	return iterNode;
}

static struct GenericTreeNode* GenericTree_DataInChildNodes(const GenericTree *self, const struct GenericTreeNode *node, const void *data){
	struct GenericTreeNode *rv=NULL;
	struct GenericTreeNode *iterNode=NULL;
	if (GenericTree_IsInitalized(self) && self->headNode!=NULL){
		for (int i=0; i<node->childNodes->numElements && rv==NULL; i++){
			iterNode=node->childNodes->methods->get(node->childNodes,i);
			if (memcmp(iterNode->data,data,self->elementSize)==0){
				rv=iterNode;
			}
		}
	}
	return rv;
}

static void GenericTree_InitalizeIterData(GenericTree *self, const struct GenericTreeNode *startNode){
	self->iterData.startNode=(struct GenericTreeNode*)startNode;
	self->iterData.previousAction='d';
	self->iterData.iterNode=NULL;
}

static struct GenericTreeNode* GenericTree_GetNextNode(GenericTree *self){
	//Operations have precedence: down, next, up
	bool _break=false;
	char actions[3]="dnu";
	struct GenericTreeNode *vals[3]={
		GenericTree_CanTraverseDown(self),
		GenericTree_CanTraverseNext(self),
		GenericTree_CanTraverseUp(self),
	};
	for (int i=0; i<3 && !_break; i++){
		if (vals[i]!=NULL){
			self->iterData.iterNode=vals[i];
			self->iterData.previousAction=actions[i];
			_break=true;
		}
	}
	return self->iterData.iterNode;
}

static struct GenericTreeNode* GenericTree_CanTraverseDown(const GenericTree *self){
	struct GenericTreeNode* rv=NULL;
	struct GenericTreeIterData iterData=self->iterData;
	if (iterData.iterNode!=NULL){
		GenericLinkedList *cNodes=iterData.iterNode->childNodes;
		if (cNodes->numElements>0 && strchr("dn",iterData.previousAction)!=NULL){
			rv=cNodes->methods->get(cNodes,0);
		}
	} else {
		rv=self->iterData.startNode;
	}
	return rv;
}

static struct GenericTreeNode* GenericTree_CanTraverseUp(const GenericTree *self){
	struct GenericTreeNode *rv=NULL;
	struct GenericTreeIterData iterData=self->iterData;
	if (iterData.iterNode!=NULL){
		int index=-1;
		GenericLinkedList *pNodes=NULL;
		GenericLinkedList *cNodes=iterData.iterNode->childNodes;
		if (iterData.iterNode!=self->headNode && iterData.iterNode!=iterData.startNode){
			pNodes=iterData.iterNode->parrentNode->childNodes;
			index=pNodes->methods->getFirstIndexOf(pNodes,iterData.iterNode);
		}
		if (cNodes->numElements==0 && strchr("dn",iterData.previousAction)!=NULL){
			//Note - may move next if has no children, not only up, watch precedence
			rv=(pNodes!=NULL)? iterData.iterNode->parrentNode: iterData.iterNode;
		} else if (pNodes!=NULL && index==pNodes->numElements-1 && iterData.previousAction=='u'){
			rv=iterData.iterNode->parrentNode;
		}
	}
	return rv;
}

static struct GenericTreeNode* GenericTree_CanTraverseNext(const GenericTree *self){
	struct GenericTreeNode *rv=NULL;
	struct GenericTreeIterData iterData=self->iterData;
	if (iterData.iterNode!=NULL){
		if (iterData.iterNode!=self->headNode && iterData.iterNode!=iterData.startNode){
			struct GenericTreeIterData iterData=self->iterData;
			GenericLinkedList *pNodes=iterData.iterNode->parrentNode->childNodes;
			int index=pNodes->methods->getFirstIndexOf(pNodes,iterData.iterNode);
			if (index<pNodes->numElements-1){
				rv=pNodes->methods->get(pNodes,index+1);
			}
		}
	}
	return rv;
}

static bool GenericTree_AreTreesCompatible(const GenericTree *self, const GenericTree *other){
	bool compatible=true;
	(!GenericTree_IsInitalized(self))? compatible=false: 0;
	(!GenericTree_IsInitalized(other))? compatible=false: 0;
	(other->elementSize!=self->elementSize)? compatible=false: 0;
	return compatible;
}

//Class Methods================================================================
static GenericTree* _GenericTreeConstructor(void){
	GenericTree *rv=(GenericTree*)createObject(GenericTreeClass.objSize);
	rv->numNodes=0;
	rv->elementSize=0;
	rv->headNode=NULL;
	rv->currentNode=NULL;
	rv->iterData.iterNode=NULL;
	rv->iterData.previousAction=' ';
	rv->pushQue=GenericLinkedListClass.new();
	rv->pushQue->methods->setElementSize(rv->pushQue,sizeof(struct GenericTreeNode));
	rv->methods=&genericTreeMethods;
	return rv;
}

static void _GenericTreePrintObj(GenericTree *obj){
	PrintClass.print("<GenericTree Obj[Addr: %p]: NumNodes: %d>\n",obj,obj->numNodes);
	struct GenericTreeNode *temp=NULL;
	if (obj->methods->startIter(obj)){
		while (obj->methods->hasNext(obj)){
			temp=GenericTree_GetNextNode(obj);
			if (obj->iterData.previousAction=='u'){
				PrintClass.incrementIndentLevel(-1);
			} else if (obj->iterData.previousAction=='d'){
				PrintClass.incrementIndentLevel(1);
			}
			PrintClass.print("<Node [%p]: ChildNodes: %d data: %p>\n",
					temp,temp->childNodes->numElements,temp->data);
		}
	}
	PrintClass.incrementIndentLevel(-1);
}

static bool _GenericTreeDebug(void){
	bool successfull=true;
	int *vals=malloc(sizeof(int)*5);
	vals[0]=5;
	for (int i=1; i<5; i++){
		vals[i]=i;
	}

	successfull&=_GenericTreeDebugAdd(vals);
	successfull&=_GenericTreeDebugRemove(vals);
	successfull&=_GenericTreeDebugSet(vals);
	successfull&=_GenericTreeDebugSearch(vals);
	successfull&=_GenericTreeDebugPushQue(vals);
	successfull&=_GenericTreeDebugCopy(vals);
	successfull&=_GenericTreeDebugMisc(vals);
	
	free(vals);
	PrintClass.objectDebug("GenericTree",successfull);
	return successfull;
}

static bool _GenericTreeDebugAdd(const int *vals){
	bool successfull=true;
	GenericTree *test=GenericTreeClass.new();
	test->methods->setElementSize(test,sizeof(int));
	test->methods->add(test,vals);
	for (int i=0; i<5; i++){
		test->methods->add(test,(vals+i));
	}
	(test->numNodes!=6)? successfull=false: 0;
	if (test->methods->startIter(test)){
		for (int i=0; test->methods->hasNext(test); i++){
			int temp=*(int*)test->methods->getNext(test);
			if (i==0 || i>5){
				(temp!=vals[0])? successfull=false: 0;
			} else {
				(temp!=vals[i-1])? successfull=false: 0;
			}
		}
	}
	for (int i=0; i<5; i++){
		test->methods->addAt(test,vals,1,(vals+i));
	}
	test->methods->placeCurrentNode(test,vals,1);
	(test->numNodes!=11)? successfull=false: 0;
	(test->methods->getNumChildNodes(test)!=5)? successfull=false: 0;
	//GenericTreeClass.print(test);
	GenericTreeClass.delete(&test);
	return successfull;
}

static bool _GenericTreeDebugRemove(const int *vals){
	bool successfull=true;
	GenericTree *test=GenericTreeClass.new();
	test->methods->setElementSize(test,sizeof(int));
	for (int i=0; i<5; i++){
		test->methods->add(test,(vals+i));
	}
	test->methods->placeCurrentNode(test,vals+2,1);
	for (int i=0; i<5; i++){
		test->methods->add(test,(vals+i));
	}
	test->methods->remove(test);
	(test->numNodes!=4)? successfull=false: 0;
	if (test->methods->startIter(test)){
		for (int i=0; test->methods->hasNext(test); i++){
			int temp=*(int*)test->methods->getNext(test);
			if (i==0 || i>3){
				(temp!=vals[0])? successfull=false: 0;
			} else if (i>=2 && i<=3){
				(temp!=vals[i+1])? successfull=false: 0;
			} else {
				(temp!=vals[i])? successfull=false: 0;
			}
		}
	}
	test->methods->clear(test);
	(test->numNodes!=0)? successfull=false: 0;
	//GenericTreeClass.print(test);
	GenericTreeClass.delete(&test);
	return successfull;
}

static bool _GenericTreeDebugSet(const int *vals){
	bool successfull=true;
	GenericTree *test=GenericTreeClass.new();
	test->methods->setElementSize(test,sizeof(int));
	for (int i=0; i<5; i++){
		test->methods->add(test,(vals+i));
	}
	test->currentNode=test->headNode;
	test->methods->set(test,vals+3);
	test->methods->setAt(test,vals+2,1,vals);
	(*(int*)test->headNode->data!=*(int*)(vals+3))? successfull=false: 0;
	(!test->methods->branchExists(test,vals,1))? successfull=false: 0;
	//GenericTreeClass.print(test);
	GenericTreeClass.delete(&test);
	return successfull;
}

static bool _GenericTreeDebugSearch(const int *vals){
	bool successfull=true;
	int branch=*(vals+3);
	int branch2[2]={*(vals+3),*(vals+3)};
	GenericTree *test=GenericTreeClass.new();
	test->methods->setElementSize(test,sizeof(int));
	for (int i=0; i<5; i++){
		test->methods->add(test,(vals+i));
	}
	(!test->methods->branchExists(test,&branch,1))? successfull=false: 0;
	(!test->methods->moveDown(test))? successfull=false: 0;
	(*(int*)test->methods->get(test)!=vals[1])? successfull=false: 0;
	(!test->methods->moveNext(test))? successfull=false: 0;
	(*(int*)test->methods->get(test)!=vals[2])? successfull=false: 0;
	(!test->methods->moveUp(test))? successfull=false: 0;
	(*(int*)test->methods->get(test)!=vals[0])? successfull=false: 0;
	(!test->methods->placeCurrentNode(test,&branch,1))? successfull=false: 0;
	(*(int*)test->methods->get(test)!=branch)? successfull=false: 0;
	for (int i=0; i<5; i++){
		test->methods->add(test,(vals+i));
	}
	if (test->methods->startIterAtCurNode(test)){
		for (int i=0; test->methods->hasNext(test); i++){
			int temp=*(int*)test->methods->getNext(test);
			if (i==0 || i>5){
				(temp!=vals[3])? successfull=false: 0;
			} else {
				(temp!=vals[i-1])? successfull=false: 0;
			}
		}
	}
	(!test->methods->branchExists(test,branch2,2))? successfull=false: 0;
	(!test->methods->placeCurrentNode(test,branch2,2))? successfull=false: 0;
	(*(int*)test->methods->get(test)!=branch2[1])? successfull=false: 0;
	(*(int*)test->methods->getAt(test,&branch2,2)!=branch2[1])? successfull=false: 0;
	test->methods->placeCurrentNode(test,&branch,1);
	(test->methods->getNumChildNodes(test)!=5)? successfull=false: 0;
	(test->methods->getNumChildNodesAt(test,&branch2,2)!=0)? successfull=false: 0;
	//GenericTreeClass.print(test);
	GenericTreeClass.delete(&test);
	return successfull;
}

static bool _GenericTreeDebugPushQue(const int *vals){
	bool successfull=true;
	int branch=*(vals+3);
	GenericTree *test=GenericTreeClass.new();
	test->methods->setElementSize(test,sizeof(int));
	for (int i=0; i<5; i++){
		test->methods->add(test,(vals+i));
	}
	test->methods->pushCurrentNode(test);
	(!test->methods->placeCurrentNode(test,&branch,1))? successfull=false: 0;
	(test->pushQue->numElements!=1)? successfull=false: 0;
	(*(int*)test->methods->get(test)!=vals[3])? successfull=false: 0;
	test->methods->popCurrentNode(test);
	(*(int*)test->methods->get(test)!=vals[0])? successfull=false: 0;
	test->methods->clearPushedNodes(test);
	(test->pushQue->numElements!=0)? successfull=false: 0;
	GenericTreeClass.delete(&test);
	return successfull;
}

static bool _GenericTreeDebugCopy(const int *vals){
	bool successfull=true;
	int branch=*(vals+3);
	int branch2[2]={vals[3],vals[2]};
	GenericTree *test=GenericTreeClass.new();
	GenericTree *test2=GenericTreeClass.new();
	test->methods->setElementSize(test,sizeof(int));
	test2->methods->setElementSize(test2,sizeof(int));
	for (int i=0; i<5; i++){
		test->methods->add(test,(vals+i));
	}
	test2->methods->add(test2,vals);
	test2->methods->copyOtherBelowCurNode(test2,test);
	(test2->numNodes!=5)? successfull=false: 0;
	(!test2->methods->placeCurrentNode(test2,&branch,1))? successfull=false: 0;
	(*(int*)test2->methods->get(test2)!=vals[3])? successfull=false: 0;
	test2->methods->copyOtherBelowCurNode(test2,test);
	(test2->numNodes!=9)? successfull=false: 0;
	(!test2->methods->placeCurrentNode(test2,branch2,2))? successfull=false: 0;
	(*(int*)test2->methods->get(test2)!=vals[2])? successfull=false: 0;
	//GenericTreeClass.print(test);
	//GenericTreeClass.print(test2);
	GenericTreeClass.delete(&test);
	GenericTreeClass.delete(&test2);
	return successfull;
}
static bool _GenericTreeDebugMisc(const int *vals){
	bool successfull=true;
	GenericTree *test=GenericTreeClass.new();
	GenericTree *test2=GenericTreeClass.new();
	test->methods->setElementSize(test,sizeof(int));
	test2->methods->setElementSize(test2,sizeof(int));
	(!test->methods->isEmpty(test))? successfull=false: 0;
	for (int i=0; i<5; i++){
		test->methods->add(test,(vals+i));
		test2->methods->add(test2,(vals+i));
	}
	(test->methods->isEmpty(test))? successfull=false: 0;
	(!test->methods->equals(test,test2))? successfull=false: 0;
	test2->methods->placeCurrentNode(test2,vals+1,1);
	test2->methods->add(test2,vals);
	(test->methods->equals(test,test2))? successfull=false: 0;
	//GenericTreeClass.print(test);
	//GenericTreeClass.print(test2);
	GenericTreeClass.delete(&test);
	GenericTreeClass.delete(&test2);
	return successfull;
}

static void _GenericTreeDestructor(GenericTree **obj){
	GenericLinkedListClass.delete(&(*obj)->pushQue);
	(*obj)->methods->clear(*obj);
	deleteObject((void**)obj);
}
