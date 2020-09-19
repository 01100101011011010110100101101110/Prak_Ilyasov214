#include <stdio.h>
#include <stdlib.h>

char *mas, *res;
int main() {
  int c;
  int idx = 0, max = -1, size = 1;
  while ((c = getchar()) != EOF) {
    if (idx + 1 >= size) {
      size = 2 * size + 1;
      mas = realloc(mas, size);
    }
    mas[idx] = c;
    mas[idx + 1] = '\0';
    if (c == '\n') {
      if (idx > max) {
        max = idx;
        free(res);
        res = mas;
        mas = (char *)malloc(size * sizeof(char));
      }
      idx = 0;
      continue;
    }
    idx++;
  }
  /* В случае, если EOF будет в конце строки */
  if (idx > max) {
    res = mas;
    mas = NULL;
  }
  printf("%s\n", res);
  free(mas);
  free(res);
  return 0;
}
  
