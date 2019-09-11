#include <stdio.h>
#include <stdbool.h>
#include "Math.h"

static bool _MathDebug(void);
static float _MathMin(float first, float second);
static float _MathMax(float first, float second);
static float _MathConstrain(float num, float min, float max);
static float _MathMap(float num, float min, float max, float newMin, float newMax);

const struct MathClass_t MathClass={
	.debug=&_MathDebug,
	.min=&_MathMin,
	.max=&_MathMax,
	.constrain=&_MathConstrain,
	.map=&_MathMap
};

//Class Methods=================================================================
static float _MathMin(float first, float second){
	if (first<second){
		return first;
	}
	return second;
}

static float _MathMax(float first, float second){
	if (first>second){
		return first;
	}
	return second;
}

static float _MathConstrain(float num, float min, float max){
	if (num<min){
		return min;
	} else if (num>max){
		return max;
	}
	return num;
}

static float _MathMap(float num, float min, float max, float newMin, float newMax){
	float range1=max-(min-1);
	float range2=newMax-(newMin-1);
	return (num*range2/range1);
}

static bool _MathDebug(void){
	bool successfull=true;
	(MathClass.min(3.0,5.0)!=3.0)? successfull=false: 0;
	(MathClass.max(3.0,5.0)!=5.0)? successfull=false: 0;
	(MathClass.map(5,1,10,1,20)!=10)? successfull=false: 0;
	printf("[%-5s]: Math class\n",(successfull)? "Ok": "Error");
	return successfull;
}
