#ifndef MATH_CLASS
#define MATH_CLASS

#include <stdbool.h>

struct MathClass_t {
	bool (*debug)(void);
	float (*min)(float first, float second);
	float (*max)(float first, float second);
	float (*constrain)(float num, float min, float max);
	float (*map)(float num, float min, float max, float newMin, float newMax);
};

extern const struct MathClass_t MathClass;

#endif
