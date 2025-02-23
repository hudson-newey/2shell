#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "errors.c"
#include "bufferline.c"
#include "built-ins/exit.c"

#define INPUT_LEN 128
#define DIR_LEN 128
#define ARG_LEN 128

#define CMD_SEPARATOR " "

#define DEFAULT_CWD "~"
#define SHELL_NAME "2sh"

#define PATH_ENV_VAR "PATH"
#define PATH_SPLIT_CHAR ":"

#define USER_ENV_VAR "USER"

int
runArgsCommand(char *command)
{
	if (strncmp(command, "exit", INPUT_LEN) == 0) {
		exitShell();
	}

	printCommandNotFoundError(command);

	return 0;
}

int
runShell()
{
	// before we even initialize the shell, I print an inital prompt so that it
	// appears to start up faster
	bufferline(DEFAULT_CWD, "");

	char *pathEnv = getenv(PATH_ENV_VAR);
	char *currentUser = getenv(USER_ENV_VAR);

	char *splitPaths[500] = {0};
	size_t pathsCount = 0;
	static size_t const maxTokenCount = sizeof(splitPaths) / sizeof(splitPaths[0]);
	for (char* path = strtok(pathEnv, PATH_SPLIT_CHAR); path != NULL && pathsCount != maxTokenCount; path = strtok(NULL, PATH_SPLIT_CHAR))
	{
	    splitPaths[pathsCount++] = path;
	}

	char currentDir[DIR_LEN] = DEFAULT_CWD;
	char input[INPUT_LEN];

	while (true)
	{
		char input[256];
		fgets(input, INPUT_LEN, stdin);
		if (strcmp(input, "\n") == 0) {
			bufferline(currentDir, currentUser);
			continue;
		}

		input[strcspn(input, "\n")] = 0;

		char *commandArgs[ARG_LEN] = {0};
		size_t argCount = 0;
		static size_t const maxCommandTokenCount = sizeof(commandArgs) / sizeof(commandArgs[0]);
		for (char* arg = strtok(input, CMD_SEPARATOR); arg != NULL && argCount != maxCommandTokenCount; arg = strtok(NULL, CMD_SEPARATOR))
		{
		    commandArgs[argCount++] = arg;
		}

		char *command = commandArgs[0];
		bool foundCommand = false;

		if (!strncmp(command, "exit", INPUT_LEN)) {
			exitShell();
		} else if (!strncmp(command, "cd", INPUT_LEN)) {
			if (argCount >= 2) {
				strncpy(currentDir, commandArgs[1], DIR_LEN);
			}

			bufferline(currentDir, currentUser);
			continue;
		}

		// I think that the local path is the most likely to contain
		// the requested exectable.
		// Therefore, I perform the O(n) operation for the local path
		// query first.
		//
		// TODO: This ordering should probably be conditional on if
		// the command starts with a ./
		char localQueryPath[DIR_LEN];
		strncpy(localQueryPath, currentDir, DIR_LEN);
		strncat(localQueryPath, command, DIR_LEN);
		char *expandeddLocalPath = expandPath(localQueryPath, currentUser);

		if (!access(localQueryPath, F_OK)) {
			int status = system(localQueryPath);
			if (status != 0) {
				printError(command, "Thrown error");
			}

			foundCommand = true;
		} else {
			for (int i = 0; i < pathsCount; i++)
			{
				char queriedPath[500] = {};
				strncpy(queriedPath, expandPath(splitPaths[i], currentUser), DIR_LEN);
				strncat(queriedPath, command, DIR_LEN);

				if (access(queriedPath, F_OK) == 0) {
					int status = system(queriedPath);
					if (status != 0) {
						printError(command, "Thrown error");
					}

					foundCommand = true;
					break;
				}
			}
		}

		if (!foundCommand) {
			printCommandNotFoundError(command);
		}

		bufferline(currentDir, currentUser);
	}
}
