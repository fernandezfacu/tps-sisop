#include "exec.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		int idx = block_contains(
		        eargv[i],
		        '=');  // eargv solo contiene argumentos del tipo 'KEY=value"
		char key[BUFLEN];
		char value[BUFLEN];
		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, idx);

		setenv(key, value, 1);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	if (flags & O_CREAT) {
		return open(file, flags, S_IWUSR | S_IRUSR);
	}

	return open(file, flags);
}

static void
free_command_or_pipe(struct cmd *c)
{
	if (parsed_pipe) {
		free_command(parsed_pipe);
	} else {
		free_command(c);
	}
}

static void
free_and_exit(struct cmd *c, char *error_msg)
{
	free_command_or_pipe(c);
	perror(error_msg);
	exit(EXIT_FAILURE);
}

static int
redir(char *file, int new_fd, int flags)
{
	int old_fd = open_redir_fd(file, flags);
	if (old_fd < 0)
		return -1;

	if (dup2(old_fd, new_fd) < 0)
		return -1;

	return 0;
}

static void
exec_redir(struct cmd *c)
{
	struct execcmd *r = (struct execcmd *) c;

	if (strlen(r->in_file) == 0 && strlen(r->out_file) == 0 &&
	    strlen(r->err_file) == 0)
		free_and_exit(c, "redir failed");

	if (strlen(r->in_file) > 0) {
		if (redir(r->in_file, STDIN_FILENO, O_CLOEXEC | O_RDONLY) < 0)
			free_and_exit(c, "redir failed");
	}
	if (strlen(r->out_file) > 0) {
		if (redir(r->out_file,
		          STDOUT_FILENO,
		          O_CLOEXEC | O_WRONLY | O_TRUNC | O_CREAT) < 0)
			free_and_exit(c, "redir failed");
	}
	if (strlen(r->err_file) > 0) {
		if (strcmp(r->err_file, "&1") == 0) {
			int oldfd = STDOUT_FILENO;
			if (dup2(oldfd, STDERR_FILENO) < 0)
				free_and_exit(c, "dup2 failed");
		} else {
			if (redir(r->err_file,
			          STDERR_FILENO,
			          O_CLOEXEC | O_WRONLY | O_TRUNC | O_CREAT) < 0)
				free_and_exit(c, "redir failed");
		}
	}
	c->type = EXEC;
	exec_cmd(c);
}

static int
exec_child(struct cmd *cmd, int old_fd, int new_fd)
{
	if (dup2(old_fd, new_fd) < 0)
		return -1;

	close(old_fd);

	setpgid(0, 0);
	exec_cmd(cmd);
	return 0;
}

static void
exec_pipe(struct cmd *c)
{
	struct pipecmd *p = (struct pipecmd *) c;

	int fds[2];
	int res = pipe(fds);
	if (res < 0)
		free_and_exit(c, "pipe failed");

	pid_t pid_left_child = fork();
	if (pid_left_child < 0)
		free_and_exit(c, "fork failed");

	if (pid_left_child == 0) {
		close(fds[READ]);
		if (exec_child(p->leftcmd, fds[WRITE], STDOUT_FILENO) < 0)
			free_and_exit(c, "dup2 failed");
	}

	pid_t pid_right_child = fork();
	if (pid_right_child < 0)
		free_and_exit(c, "fork failed");

	if (pid_right_child == 0) {
		close(fds[WRITE]);
		if (exec_child(p->rightcmd, fds[READ], STDIN_FILENO) < 0)
			free_and_exit(c, "dup2 failed");
	}

	close(fds[WRITE]);
	close(fds[READ]);

	waitpid(pid_left_child, NULL, 0);
	waitpid(pid_right_child, NULL, 0);

	free_command(parsed_pipe);
	exit(EXIT_SUCCESS);
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;

	switch (cmd->type) {
	case EXEC: {
		// spawns a command
		e = (struct execcmd *) cmd;

		set_environ_vars(e->eargv, e->eargc);
		if (execvp(e->argv[0], e->argv) < 0)
			free_and_exit(cmd, "exec failed");
		break;
	}
	case BACK: {
		// runs a command in background
		b = (struct backcmd *) cmd;
		e = (struct execcmd *) b->c;

		set_environ_vars(e->eargv, e->eargc);
		if (execvp(e->argv[0], e->argv) < 0) {
			free_command(cmd);
			perror("exec failed");
			exit(EXIT_FAILURE);
		}  // nuestra shell no soporta pipes ejecutandose en background.
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		exec_redir(cmd);
		break;
	}

	case PIPE: {
		// pipes two commands
		exec_pipe(cmd);
		break;
	}
	}
}
