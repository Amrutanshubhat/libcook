#include <assert.h>
#include <stdlib.h>
#include "meth.h"

int main(void) {
	float x[] = {
		1, 2, 4,
		-2, 3, 1,
		-4, 1, 2
	};
	float y[] = { 
		4, 
		5, 
		6 };

	float* out = mat_multiply(x, 3, 3, y, 3, 1);
	mat_print(out, 3, 1);
	mat_transform(x, 3, 3);
	mat_print(x, 3, 3);
	free(out);
}
