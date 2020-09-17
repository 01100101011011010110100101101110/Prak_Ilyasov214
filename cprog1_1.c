#include <stdio.h>
#include <stdlib.h>

int *mas, *res;
int main() {
	int c;
	int  idx = 0, max = -1, size = 0;
	while ((c = getchar()) != EOF) {
		if (idx == size) {
			size = 2 * size + 1;
			mas = realloc(mas, size);	
		}
		mas[idx] = c;
		if (c == '\n') {
			if (idx > max) {
				int i;
				max = idx;
				free(res);
				res = realloc(res, size);
				for (i = 0; i <= idx; i++) {
					res[i] = mas[i];
				}				 	
			}
			idx = 0;
			free(mas);
			mas = realloc(mas, size);
		}	
		idx++;
	}
	for (idx = 0; idx <= max; idx++) {
		putchar(res[idx]);
	}
	free(mas);
	free(res);
	return 0;
}

