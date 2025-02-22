#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "errors.c"
#include "bufferline.c"
#include "built-ins/exit.c"

#define INPUT_LENGTH 100
#define DEFAULT_CWD "~"
#define PATH_ENV_VAR "PATH"
#define SHELL_NAME "2sh"

int
runShellCommand(char *command)
{
	if (strncmp(command, "exit", INPUT_LENGTH) == 0) {
		exitShell();
	}

	printCommandNotFoundError(command);
}

int
runShell()
{
	// before we even initialize the shell, I print an inital prompt so that it
	// appears to start up faster
	bufferline(DEFAULT_CWD);

	char *pathEnv = getenv(PATH_ENV_VAR);
	char *splitPaths = strtok(pathEnv, ";");

	char current_dir[100] = DEFAULT_CWD;
	char input[INPUT_LENGTH];

	while (true)
	{
		char input[256];
		fgets(input, INPUT_LENGTH, stdin);
		if (strcmp(input, "\n") == 0) {
			bufferline(current_dir);
			continue;
		}

		input[strcspn(input, "\n")] = 0;

		char *command = strtok(input, " ");
		runShellCommand(command);

		bufferline(current_dir);
	}
}
