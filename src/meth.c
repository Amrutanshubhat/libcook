#include <stdio.h>
#include "libcook.h"
#include "meth.h"

float* mat_multiply(float* x, size_t rx, size_t cx, float* y, size_t ry, size_t cy) {
	assert(cx == ry);
	float* out = calloc(sizeof(*out), rx*cy);

	for (size_t col=0; col<cy; col++) {
		size_t colval = col*rx;
		for (size_t i=0; i<rx; i++) {
			size_t rowval = i*cx;
			for (size_t j=0; j<ry; j++) {
				out[colval+i] += x[rowval+j]*y[colval+j];
			}
		}
	}
	return out;
}

void mat_transform(float* mat, size_t r, size_t c) {
	for (size_t i=0; i<r; i++) {
		for (size_t j=i; j<c; j++) {
			float temp = mat[i*c+j];
			mat[i*c+j] = mat[j*c+i];
			mat[j*c+i] = temp;
		}
	}
}

void mat_print(float* mat, size_t r, size_t c) {
	for (size_t i=0; i<r; i++) {
		for (size_t j=0; j<c; j++) {
			printf("%.1f  ", mat[i*c+j]);
		}
		printf("\n");
	}
}
