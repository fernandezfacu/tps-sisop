#include "builtin.h"

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	return !strcmp(cmd,
	               "exit");  // Sólo devuelve true si se recibe "exit" sin ningún caracter extra.
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	char *aux_cmd = calloc(BUFLEN, sizeof(char));
	strcpy(aux_cmd, cmd);
	char *right = split_line(aux_cmd, ' ');

	if ((strcmp(aux_cmd, "cd") == 0) && (block_contains(right, ' ') != 0)) {
		if (strlen(right) == 0) {
			if (chdir(getenv("HOME")) < 0) {
				perror("cd");
			}
		} else {
			if (chdir(right) < 0) {
				char buf[BUFLEN];
				snprintf(buf, sizeof buf, "cd: %s", right);
				perror(buf);
			}
		}
		if (!getcwd(prompt, PRMTLEN))
			perror("getcwd");


		free(aux_cmd);
		return 1;
	}

	free(aux_cmd);
	return 0;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, "pwd") == 0) {
		char actual_directory[BUFLEN];
		if (!getcwd(actual_directory, BUFLEN))
			perror("getcwd");
		printf("%s\n", actual_directory);
		return 1;
	}

	return 0;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
