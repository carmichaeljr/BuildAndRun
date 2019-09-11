#include <string.h>
#include <stdbool.h>
#include "IntList.h"
#include "Object.h"
#include "Print.h"
#include "GenericList.h"

static IntList* _IntListConstructor(void);
static void _IntListPrintObj(IntList *obj);
static bool _IntListDebug(void);
static bool _IntListDebugAdd(void);
static bool _IntListDebugSet(void);
static bool _IntListDebugRemove(void);
static bool _IntListDebugSearch(void);
static bool _IntListDebugMisc(void);
static void _IntListDestructor(IntList **obj);
static void IntList_Add(IntList *self, int num);
static void IntList_Splice(IntList *self, const IntList *other);
static void IntList_SetSize(IntList *self, const int len);
static void IntList_Sort(IntList *self);
static int IntList_GetFirstIndexOf(const IntList *self, const int num);
static int IntList_GetLastIndexOf(const IntList *self, const int num);
static int IntList_GetNumOccurences(const IntList *self, const int searchNum);
static void IntList_CopyOtherBetween(IntList *self, const IntList *other, const int startIndex, const int endIndex);
static void IntList_Remove(IntList *self, const int unwantedNum);
static void IntList_RemoveNumRange(IntList *self, const int rangeStart, const int rangeEnd);
static void IntList_RemoveAt(IntList *self, const int index);
static void IntList_RemoveBetween(IntList *self, const int startIndex, const int endIndex);
static void IntList_TrimSubList(IntList *self, const int startIndex, const int endIndex);
static IntList* IntList_GetIndexesOf(const IntList *self, const int token);
static bool IntList_Contains(const IntList *self, const int token);
static bool IntList_Equals(const IntList *self, const int *other);
static void IntList_Clear(IntList *self);
static void IntList_ConvertFromGenericListToIntList(IntList *self);
static void IntList_QuickSort(IntList *self, const int startIndex, const int endIndex);
static void IntList_Swap(IntList *self, const int one, const int two);

static const struct IntListMethods intListMethods={
	.add=&IntList_Add,
	.splice=&IntList_Splice,
	.setSize=&IntList_SetSize,
	.sort=&IntList_Sort,
	.getFirstIndexOf=&IntList_GetFirstIndexOf,
	.getLastIndexOf=&IntList_GetLastIndexOf,
	.getNumOccurences=&IntList_GetNumOccurences,
	.copyOtherBetween=&IntList_CopyOtherBetween,
	.remove=&IntList_Remove,
	.removeNumRange=&IntList_RemoveNumRange,
	.removeAt=&IntList_RemoveAt,
	.removeBetween=&IntList_RemoveBetween,
	.trimSubList=&IntList_TrimSubList,
	.getIndexesOf=&IntList_GetIndexesOf,
	.contains=&IntList_Contains,
	.equals=&IntList_Equals,
	.clear=&IntList_Clear,
};

const struct IntListClass_t IntListClass={
	.new=&_IntListConstructor,
	.delete=&_IntListDestructor,
	.print=&_IntListPrintObj,
	.debug=&_IntListDebug,
	.objSize=sizeof(IntList)
};

//Object Methods===============================================================
//Public Methods
static void IntList_Add(IntList *self, const int num){
	self->genericList->methods->add(self->genericList,&num,1);
	IntList_ConvertFromGenericListToIntList(self);
}

static void IntList_Splice(IntList *self, const IntList *other){
	self->genericList->methods->add(self->genericList,other->list,other->length);
	IntList_ConvertFromGenericListToIntList(self);
}

static void IntList_SetSize(IntList *self, const int len){
	int blank=0;
	GenericList *temp=self->genericList;
	temp->methods->setListSize(temp,len);
	for (int i=0; i<temp->numElements; i++){
		temp->methods->setAt(temp,&blank,1,i);
	}
	IntList_ConvertFromGenericListToIntList(self);
}

static void IntList_Sort(IntList *self){
	if (self->length>1){
		IntList_QuickSort(self,0,self->length-1);
	}
}

static int IntList_GetFirstIndexOf(const IntList *self, const int num){
	GenericList *temp=self->genericList;
	return temp->methods->getFirstIndexOf(temp,&num);
}

static int IntList_GetLastIndexOf(const IntList *self, const int num){
	GenericList *temp=self->genericList;
	return temp->methods->getLastIndexOf(temp,&num);
}

static int IntList_GetNumOccurences(const IntList *self, const int searchNum){
	int rv=0;
	for (int i=0; i<self->length; i++){
		if (self->list[i]==searchNum){
			rv++;
		}
	}
	return rv;
}

static void IntList_CopyOtherBetween(IntList *self, const IntList *other, const int startIndex, const int endIndex){
	GenericList *selfTemp=self->genericList;
	GenericList *otherTemp=other->genericList;
	selfTemp->methods->copyOtherBetween(selfTemp,otherTemp,startIndex,endIndex);
	IntList_ConvertFromGenericListToIntList(self);
}

static void IntList_Remove(IntList *self, const int unwantedNum){
	GenericList *temp=self->genericList;
	temp->methods->remove(temp,&unwantedNum);
	IntList_ConvertFromGenericListToIntList(self);
}

static void IntList_RemoveNumRange(IntList *self, const int rangeStart, const int rangeEnd){
	GenericList *temp=self->genericList;
	for (int i=rangeStart; i<rangeEnd; i++){
		temp->methods->remove(temp,&i);
	}
	IntList_ConvertFromGenericListToIntList(self);
}

static void IntList_RemoveAt(IntList *self, const int index){
	GenericList *temp=self->genericList;
	temp->methods->removeAt(temp,index);
	IntList_ConvertFromGenericListToIntList(self);
}

static void IntList_RemoveBetween(IntList *self, const int startIndex, const int endIndex){
	GenericList *temp=self->genericList;
	temp->methods->removeBetween(temp,startIndex,endIndex);
	IntList_ConvertFromGenericListToIntList(self);
}

static void IntList_TrimSubList(IntList *self, const int startIndex, const int endIndex){
	GenericList *temp=self->genericList;
	temp->methods->removeBetween(temp,startIndex,endIndex);
	IntList_ConvertFromGenericListToIntList(self);
}

static IntList* IntList_GetIndexesOf(const IntList *self, const int token){
	IntList *rv=IntListClass.new();
	for (int i=0; i<self->length; i++){
		if (self->list[i]==token){
			rv->methods->add(rv,i);
		}
	}
	return rv;
}

static bool IntList_Contains(const IntList *self, const int token){
	return (self->methods->getFirstIndexOf(self,token)>=0)? true: false;
}

static bool IntList_Equals(const IntList *self, const int *other){
	if (memcmp(self->list,other,self->length*sizeof(int))==0){
		return true;
	}
	return false;
}

static void IntList_Clear(IntList *self){
	self->genericList->methods->clear(self->genericList);
	IntList_ConvertFromGenericListToIntList(self);
}

//Private Methods
static void IntList_ConvertFromGenericListToIntList(IntList *self){
	GenericList *temp=self->genericList;
	bool hasElements=(temp->numElements>0)? true: false;
	if (hasElements){
		self->list=(int*)temp->list;
	} else {
		self->list=NULL;
	}
	self->length=temp->numElements;
}

static void IntList_QuickSort(IntList *self, const int startIndex, const int endIndex){
	if (startIndex>=0 && startIndex<endIndex && endIndex<self->length){
		int piviotElement=self->list[endIndex];
		int piviotBoundary=startIndex;
		for (int i=startIndex; i<endIndex; i++){
			if (self->list[i]<piviotElement){
				IntList_Swap(self,i,piviotBoundary);
				piviotBoundary++;
			}
		}
		IntList_Swap(self,piviotBoundary,endIndex);
		IntList_QuickSort(self,startIndex,piviotBoundary-1);
		//PrintClass.print("%d: %d\n",piviotBoundary,endIndex);
		IntList_QuickSort(self,piviotBoundary,endIndex);
	}
}

static void IntList_Swap(IntList *self, const int one, const int two){
	if (one!=two && one>=0 && two>=0 && one<self->length && two<self->length){
		int temp=self->list[one];
		self->list[one]=self->list[two];
		self->list[two]=temp;
	}
}

//Class Methods================================================================
static IntList* _IntListConstructor(void){
	IntList *rv=(IntList*)createObject(IntListClass.objSize);
	rv->genericList=GenericListClass.new();
	rv->genericList->methods->setElementSize(rv->genericList,sizeof(int));
	rv->list=NULL;
	rv->length=0;
	rv->methods=&intListMethods;
	return rv;
}

static void _IntListPrintObj(IntList *obj){
	PrintClass.print("<IntList Obj[Addr: %p]: Values: below  Length: %d>\n",obj,obj->length);
	PrintClass.incrementIndentLevel(1);
	PrintClass.print("Values: [");
	for (int i=0; i<obj->length; i++){
		PrintClass.printWithoutIndent("%d, ",obj->list[i]);
	}
	PrintClass.printWithoutIndent("]\n");
	PrintClass.incrementIndentLevel(-1);
}

static bool _IntListDebug(void){
	bool successfull=true;
	successfull&=_IntListDebugAdd();
	successfull&=_IntListDebugSet();
	successfull&=_IntListDebugRemove();
	successfull&=_IntListDebugSearch();
	successfull&=_IntListDebugMisc();
	PrintClass.objectDebug("IntList",successfull);
	return successfull;
}

static bool _IntListDebugAdd(void){
	bool successfull=true;
	IntList *test=IntListClass.new();
	IntList *test2=IntListClass.new();
	test->methods->add(test,1);
	test->methods->add(test,2);
	test->methods->add(test,3);
	(test->length!=3 || test->list[0]!=1)? successfull=false: 0;
	test2->methods->copyOtherBetween(test2,test,0,2);
	(test2->length!=2)? successfull=false: 0;
	(test2->list[0]!=1)? successfull=false: 0;
	(test2->list[1]!=2)? successfull=false: 0;
	test->methods->splice(test,test2);
	(test->length!=5)? successfull=false: 0;
	(test->list[2]!=3)? successfull=false: 0;
	(test->list[3]!=1)? successfull=false: 0;
	(test->list[4]!=2)? successfull=false: 0;
	//IntListClass.print(test);
	//IntListClass.print(test2);
	IntListClass.delete(&test);
	IntListClass.delete(&test2);
	return successfull;
}

static bool _IntListDebugSet(void){
	bool successfull=true;
	IntList *test=IntListClass.new();
	test->methods->add(test,1);
	test->methods->add(test,2);
	test->methods->setSize(test,5);
	(test->length!=5)? successfull=false: 0;
	//IntListClass.print(test);
	IntListClass.delete(&test);
	return successfull;
}

static bool _IntListDebugRemove(void){
	bool successfull=true;
	IntList *test=IntListClass.new();
	IntList *test2=IntListClass.new();
	for (int i=0; i<9; i++){
		test->methods->add(test,i);
		test2->methods->add(test2,i);
	}
	test->methods->add(test,1);
	test->methods->remove(test,1);
	(test->length!=8)? successfull=false: 0;
	(test->list[1]!=2)? successfull=false: 0;
	test->methods->removeNumRange(test,0,3);
	(test->length!=6)? successfull=false: 0;
	(test->list[0]!=3)? successfull=false: 0;
	test->methods->removeAt(test,0);
	(test->length!=5)? successfull=false: 0;
	(test->list[0]!=4)? successfull=false: 0;
	test->methods->removeBetween(test,1,4);
	(test->length!=2)? successfull=false: 0;
	(test->list[0]!=4)? successfull=false: 0;
	(test->list[1]!=8)? successfull=false: 0;
	test->methods->clear(test);
	(test->length!=0)? successfull=false: 0;
	test2->methods->trimSubList(test2,1,test2->length);
	(test2->length!=1)? successfull=false: 0;
	(test2->list[0]!=0)? successfull=false: 0;
	//IntListClass.print(test);
	//IntListClass.print(test2);
	IntListClass.delete(&test);
	IntListClass.delete(&test2);
	return successfull;
}

static bool _IntListDebugSearch(void){
	bool successfull=true;
	IntList *test=IntListClass.new();
	IntList *test2=IntListClass.new();
	for (int i=0; i<3; i++){
		test->methods->add(test,i+1);
		test2->methods->add(test2,i+1);
	}
	for (int i=0; i<3; i++){
		test->methods->add(test,2);
		test2->methods->add(test2,2);
	}
	(test->methods->getFirstIndexOf(test,3)!=2)? successfull=false: 0;
	(test->methods->getLastIndexOf(test,1)!=0)? successfull=false: 0;
	(test->methods->getNumOccurences(test,2)!=4)? successfull=false: 0;
	(!test->methods->contains(test,3))? successfull=false: 0;
	(test->methods->contains(test,4))? successfull=false: 0;
	IntList *indexes=test->methods->getIndexesOf(test,2);
	(indexes->length!=4)? successfull=false: 0;
	(indexes->list[0]!=1)? successfull=false: 0;
	(indexes->list[3]!=5)? successfull=false: 0;
	(!test->methods->equals(test,test2->list))? successfull=false: 0;
	test2->methods->trimSubList(test2,3,test2->length);
	(test->methods->equals(test,test2->list))? successfull=false: 0;
	//IntListClass.print(test);
	//IntListClass.print(test2);
	IntListClass.delete(&indexes);
	IntListClass.delete(&test);
	IntListClass.delete(&test2);
	return successfull;
}

static bool _IntListDebugMisc(void){
	bool successfull=true;
	IntList *test=IntListClass.new();
	for (int i=0; i<100; i++){
		test->methods->add(test,rand());
	}
	test->methods->sort(test);
	for (int i=1; i<100 && successfull; i++){
		if (test->list[i-1]>test->list[i]){
			successfull=false;
		}
	}
	//IntListClass.print(test);
	IntListClass.delete(&test);
	return successfull;
}

static void _IntListDestructor(IntList **obj){
	GenericListClass.delete(&(*obj)->genericList);
	deleteObject((void**)obj);
}
