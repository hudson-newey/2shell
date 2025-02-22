#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "errors.c"
#include "bufferline.c"
#include "built-ins/exit.c"

#define INPUT_LENGTH 100
#define DEFAULT_CWD "~"
#define SHELL_NAME "2sh"

#define PATH_ENV_VAR "PATH"
#define PATH_SPLIT_CHAR ":"

int
runArgsCommand(char *command)
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

	char *splitPaths[500] = {0};
	size_t pathsCount = 0;
	static size_t const max_token_count = sizeof(splitPaths) / sizeof(splitPaths[0]);
	for (char* path = strtok(pathEnv, PATH_SPLIT_CHAR); path != NULL && pathsCount != max_token_count; path = strtok(NULL, PATH_SPLIT_CHAR))
	{
	    splitPaths[pathsCount++] = path;
	}

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

		if (strncmp(command, "exit", INPUT_LENGTH) == 0) {
			exitShell();
		}

		bool foundCommand = false;
		for (int i = 0; i < pathsCount; i++)
		{
			char queriedPath[500] = {};
			strcat(queriedPath, splitPaths[i]);
			strcat(queriedPath, command);

			if (access(queriedPath, F_OK) == 0) {
				int status = system(queriedPath);
				if (status != 0) {
					printError(command, "Thrown error");
				}

				foundCommand = true;
				break;
			}
		}

		if (!foundCommand) {
			printCommandNotFoundError(command);
		}

		bufferline(current_dir);
	}
}
