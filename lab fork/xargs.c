#ifndef NARGS
#define NARGS 4
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define XARGS "./xargs"
#define XARGS_ARGS 2
#define FILENAME 0
#define XARGS_COMM 1
#define CHILD_PROC_ID 0
#define NEWLINE_CHAR "\n"

void
error(char *error_msg)
{
	perror(error_msg);
	exit(EXIT_FAILURE);
}

void
check_fork(int return_code)
{
	if (return_code < 0)
		error("Error in fork.\n");
}

void
check_exec(int return_code)
{
	if (return_code < 0)
		error("Error in exec.\n");
}

bool
reached_eof(size_t chars_read)
{
	return chars_read == (size_t) EOF;
}

void
save_arg(char *args[], int pos, char *arg, size_t arg_size)
{
	args[pos] = malloc(arg_size);
	if (!args[pos])
		exit(EXIT_FAILURE);
	strtok(arg, NEWLINE_CHAR);
	strcpy(args[pos], arg);
}

void
execute(char *command, char *args[], int args_size)
{
	char *exec_args[NARGS + 2];
	save_arg(exec_args, FILENAME, command, strlen(command) + 1);

	for (int i = 0; i < args_size; i++)
		exec_args[i + 1] = args[i];

	exec_args[args_size + 1] = NULL;

	pid_t child_id = fork();
	check_fork(child_id);
	if (child_id == CHILD_PROC_ID) {
		check_exec(execvp(exec_args[FILENAME], exec_args));
	} else {
		wait(NULL);
	}

	free(exec_args[FILENAME]);
}

void
nargs(char *command)
{
	bool args_left = true;
	while (args_left) {
		char *args[NARGS];
		int args_read = 0;

		char *arg = NULL;
		size_t arg_len = 0;
		size_t chars_read = 0;
		chars_read = getline(&arg, &arg_len, stdin);
		while (!reached_eof(chars_read) && (args_read < NARGS)) {
			save_arg(args, args_read, arg, chars_read);
			args_read++;

			if (args_read < NARGS)
				chars_read = getline(&arg, &arg_len, stdin);
		}

		execute(command, args, args_read);

		if (reached_eof(chars_read))
			args_left = false;

		for (int i = 0; i < args_read; i++)
			free(args[i]);
		free(arg);
	}
}

int
main(int argc, char *argv[])
{
	if (argc != XARGS_ARGS)
		error("La cantidad de argumentos es incorrecta.\n");

	char *command;
	command = argv[XARGS_COMM];

	nargs(command);

	exit(EXIT_SUCCESS);
}
