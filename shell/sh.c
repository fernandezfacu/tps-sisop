#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

char prompt[PRMTLEN] = { 0 };

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

static void
handler()
{
	pid_t pid_child;
	while ((pid_child = waitpid(0, &status, WNOHANG)) > 0) {
		char buf[BUFLEN] = { 0 };
		snprintf(buf, BUFLEN, "==> terminado: PID=%i\n", pid_child);
		if (write(STDOUT_FILENO, buf, BUFLEN) < 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}
	}
	// printf("==> terminado: PID=%i\n", pid_child);
}

static void
set_sigchld_handler()
{
	struct sigaction act;
	stack_t ss;

	ss.ss_sp = malloc(SIGSTKSZ);
	if (ss.ss_sp == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	ss.ss_size = SIGSTKSZ;
	ss.ss_flags = SS_DISABLE;
	if (sigaltstack(&ss, NULL) == -1) {
		perror("sigaltstack");
		exit(EXIT_FAILURE);
	}

	memset(&act, 0, sizeof(act));
	act.sa_handler = handler;
	act.sa_flags = SA_NOCLDSTOP | SA_RESTART | SA_ONSTACK;
	if (sigaction(SIGCHLD, &act, NULL) < 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	free(ss.ss_sp);
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	set_sigchld_handler();

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

int
main(void)
{
	init_shell();

	run_shell();

	return 0;
}
