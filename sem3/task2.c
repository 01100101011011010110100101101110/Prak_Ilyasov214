:#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>	
#include <sys/wait.h>
#include <string.h>

//реализовать echo 1 2 3 ... n n
int num_len(int num) {
	int count = 0;
	while (num != 0) {
		num /= 10;
		count += 1;
	}
	return count;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "invalid count of params");
		exit(0);
	}
	int N = atoi(argv[1]);
	if (N < 1) {
		printf("Invalid N\n");
		exit(0);
	}
	char *inp = (char *)malloc(sizeof(char));
	inp = (char *)realloc(inp, (sizeof(char) + 1));
	strcpy(inp, "1\0");
	for (int i = 2; i <= N; ++i) {
		char str[num_len(i)];
		sprintf(str, " %d", i);
		inp = (char *)realloc(inp, (strlen(inp) + strlen(str) + 2));
		strcat(inp, str);
	}
	if (fork() == 0) {
		execlp("echo", "echo", inp, NULL);
		free(inp);
		exit(0);
	}
	wait(NULL);
	free(inp);
	return 0;
}
