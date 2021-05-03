#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Hi there: %s", getenv("QUERY_STRING"));
    return 0;
}
