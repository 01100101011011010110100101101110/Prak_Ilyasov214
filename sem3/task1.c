#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//реализовать (pr1;pr2) | pr3 > fname
int main(int argc, char **argv) {
	if (argc != 5) {
		fprintf(stderr,  "not enough args provided\n");
		return 1;
	}
	int fd[2];
	pipe(fd);
	//реализовать (pr1;pr2)
	if (fork() == 0) {
		dup2(fd[1], 1);
		close(fd[1]);
		close(fd[0]);
		execlp(argv[1], argv[1], NULL);
		perror("exec err: ");
		exit(0);
	}
	if (fork() == 0) {
		dup2(fd[1], 1);
		close(fd[1]);
		close(fd[0]);
		execlp(argv[2], argv[2], NULL);
		perror("exec err: ");
		exit(0);
	}
	if (fork() == 0) {
		dup2(fd[0], 0);
		close(fd[0]);
		close(fd[1]);
		int fname = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0666);
		dup2(fname, 1);
		close(fname);
		execlp(argv[3], argv[3], NULL);
		perror("exec err");
		exit(0);
	}
	close(fd[0]);
	close(fd[1]);
	while(wait(NULL) != -1);
	return 0;
}
