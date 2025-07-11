#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1
#define CHILD_PROC_ID 0
#define PRIMES_ARGS 2
#define PRIMES_N 1
#define EOF_READ 0


void
error(char *error_msg)
{
	perror(error_msg);
	exit(EXIT_FAILURE);
}

void
check_read(int return_code)
{
	if (return_code < 0)
		error("Error in read.\n");
}

void
check_write(int return_code)
{
	if (return_code < 0)
		error("Error in write.\n");
}

void
check_fork(int return_code)
{
	if (return_code < 0)
		error("Error in fork.\n");
}

void
check_pipe(int return_code)
{
	if (return_code < 0)
		error("Error in pipe.\n");
}

void print_prime_and_filter(int fd_left);

void
send_ints_to_next_filter(int fd_left, int p, int value, int fd_right, int res)
{
	while (res != EOF_READ) {
		if (value % p != 0) {
			res = write(fd_right, &value, sizeof value);
			check_write(res);
		}
		res = read(fd_left, &value, sizeof value);
		check_read(res);
	}
}

void
filter(int fd_left, int p)
{
	int res;
	int value;

	res = read(fd_left, &value, sizeof value);
	check_read(res);
	if (res == EOF_READ) {
		close(fd_left);
	} else {
		int fds_child[2];
		check_pipe(pipe(fds_child));
		pid_t child_id = fork();
		check_fork(child_id);

		if (child_id == CHILD_PROC_ID) {
			close(fd_left);
			close(fds_child[WRITE]);
			print_prime_and_filter(fds_child[READ]);
		} else {
			close(fds_child[READ]);
			send_ints_to_next_filter(
			        fd_left, p, value, fds_child[WRITE], res);
			close(fd_left);
			close(fds_child[WRITE]);
			wait(NULL);
		}
	}
}

void
print_prime_and_filter(int fd_left)
{
	int p;
	int res;

	res = read(fd_left, &p, sizeof p);
	check_read(res);

	printf("primo %i\n", p);
	fflush(stdout);

	filter(fd_left, p);
}

void
primes(int n)
{
	int fds[2];
	check_pipe(pipe(fds));

	pid_t child_id = fork();
	check_fork(child_id);

	if (child_id == CHILD_PROC_ID) {
		close(fds[WRITE]);
		print_prime_and_filter(fds[READ]);
	} else {
		int res;

		close(fds[READ]);

		for (int i = 2; i <= n; i++) {
			res = write(fds[WRITE], &i, sizeof i);
			check_write(res);
		}

		close(fds[WRITE]);

		wait(NULL);
	}
}

int
main(int argc, char *argv[])
{
	if (argc != PRIMES_ARGS)
		error("La cantidad de argumentos es incorrecta.\n");

	int n = atoi(argv[PRIMES_N]);
	if (n < 2)
		error("El número pasado como argumento debe ser mayor o igual "
		      "a 2.\n");

	primes(n);

	exit(EXIT_SUCCESS);
}