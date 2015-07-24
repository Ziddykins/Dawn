#include "include/util.h"

double randd() {
    return ((double)(rand()))/INT_MAX;
}

double gaussrand() {
	static double V1, V2, S;
	static int phase = 0;
	double X;

	if(phase == 0) {
		do {
			V1 = 2 * randd() - 1;
			V2 = 2 * randd() - 1;
			S = V1 * V1 + V2 * V2;
			} while(S >= 1 || !(S < 0 || S > 0)); //S must be != 0
		X = V1 * sqrt(-2 * log(S) / S);
	} else {
		X = V2 * sqrt(-2 * log(S) / S);
    }
	phase = !phase;

	return X;
}

double ABS(double a) {
    return a > -a ? a : -a;
}

int compareFloatAsc(void const * a, void const * b) {
    return (*(float const*)a) > (*(float const*)b) ? 1 : (*(float const*)a) < (*(float const*)b) ? -1 : 0;
}
