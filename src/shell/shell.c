#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "paths.c"
#include "errors.c"

#include "../built-ins/exit.c"
#include "../built-ins/cd.c"

#define INPUT_LEN 256
#define DIR_LEN 128
#define ARG_LEN 128

#define CMD_SEPARATOR " "

#define DEFAULT_CWD "~"
#define SHELL_NAME "2sh"

#define PATH_ENV_VAR "PATH"
#define PATH_SPLIT_CHAR ":"

#define USER_ENV_VAR "USER"

#define COLOR_RESET "\x1b[0m"
#define FAILURE_RED(string) "\x1b[31m" string COLOR_RESET
#define SUCCESS_GREEN(string) "\x1b[32m" string COLOR_RESET

int
runArgsCommand(char *command)
{
	if (strncmp(command, "exit", INPUT_LEN) == 0)
	{
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
	printf("\x1b[36m~\x1b[32m > \x1b[0m");

	char *pathEnv = getenv(PATH_ENV_VAR);
	char *currentUser = getenv(USER_ENV_VAR);

	char initialDir[DIR_LEN];
	strncpy(initialDir, "/home/", DIR_LEN);
	strncat(initialDir, currentUser, DIR_LEN);
	strncat(initialDir, "/", DIR_LEN);
	cdShell(initialDir);

	char *splitPaths[500] = {0};
	size_t pathsCount = 0;
	static size_t const maxTokenCount = sizeof(splitPaths) / sizeof(splitPaths[0]);
	for (char* path = strtok(pathEnv, PATH_SPLIT_CHAR); path != NULL && pathsCount != maxTokenCount; path = strtok(NULL, PATH_SPLIT_CHAR))
	{
	    splitPaths[pathsCount++] = path;
	}

	char currentDir[DIR_LEN] = DEFAULT_CWD;
	char input[INPUT_LEN];

	bool lastCommandSuccess = true;

	printf("\e[1;1H\e[2J");
	while (true)
	{
		char bufferlinePrompt[INPUT_LEN];
		strncpy(bufferlinePrompt, "\x1b[36m", INPUT_LEN);
		strncat(bufferlinePrompt, shortenPath(currentDir, currentUser), INPUT_LEN);
		strncat(bufferlinePrompt, "\x1b[0m", INPUT_LEN);

		if (lastCommandSuccess)
		{
			strncat(bufferlinePrompt, SUCCESS_GREEN(" > "), INPUT_LEN);
		}
		else
		{
			strncat(bufferlinePrompt, FAILURE_RED(" > "), INPUT_LEN);
		}

		char *input = readline(bufferlinePrompt);
		if (!strcmp(input, ""))
		{
			continue;
		}

		input[strcspn(input, "\n")] = 0;
		char unmodifiedInput[INPUT_LEN];
		strncpy(unmodifiedInput, input, INPUT_LEN);

		// this modified input is used to have inbuilt aliases
		// e.g. ls is mapped to ls --color
		// these inbuilt mappings are typically just visual
		char modifiedInput[INPUT_LEN];
		strncpy(modifiedInput, input, INPUT_LEN);

		char *userCommand[ARG_LEN] = {0};
		size_t argCount = 0;
		static size_t const maxCommandTokenCount = sizeof(userCommand) / sizeof(userCommand[0]);
		for (char* arg = strtok(input, CMD_SEPARATOR); arg != NULL && argCount != maxCommandTokenCount; arg = strtok(NULL, CMD_SEPARATOR))
		{
			userCommand[argCount++] = arg;
		}

		char *command = userCommand[0];
		bool foundCommand = false;

		if (!strncmp(command, "exit", INPUT_LEN) || !strncmp(command, "q", INPUT_LEN))
		{
			exitShell();
		}
		else if (!strncmp(command, "cd", INPUT_LEN))
		{
			if (argCount >= 2)
			{
				strncpy(currentDir, userCommand[1], DIR_LEN);

				// using the chdir() function communicates to
				// programs like "ls" and "pwd" what directory
				// we are currently looking at
				int respCode = cdShell(currentDir);
				lastCommandSuccess = !respCode;

				char *newPathValue = getenv("PWD");
				getcwd(currentDir, DIR_LEN);
			}
			else
			{
				lastCommandSuccess = false;
			}

			continue;
		}

		if (!strncmp(command, "ls", INPUT_LEN))
		{
			strncat(modifiedInput, " --color", INPUT_LEN);
		}
		else if (!strncmp(command, "grep", INPUT_LEN))
		{
			strncat(modifiedInput, " --color", INPUT_LEN);
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

		char *expandedLocalPath = expandPath(localQueryPath, currentUser, false);

		if (!access(localQueryPath, F_OK))
		{
			lastCommandSuccess = true;

			char executedCommand[INPUT_LEN];
			strncpy(executedCommand, currentDir, INPUT_LEN);
			strncpy(executedCommand, modifiedInput, INPUT_LEN);

			int status =  system(executedCommand);
			if (status != 0)
			{
				printError(command, "Thrown error");
				lastCommandSuccess = false;
			}

			foundCommand = true;
		}
		else
		{
			for (int i = 0; i < pathsCount; i++)
			{
				char *expandedQueriedPath = expandPath(splitPaths[i], currentUser, true);
				char queriedPath[DIR_LEN] = {};
				strncpy(queriedPath, expandedQueriedPath, DIR_LEN);
				strncat(queriedPath, command, DIR_LEN);

				if (access(queriedPath, F_OK) == 0)
				{
					lastCommandSuccess = true;

					char executedCommand[INPUT_LEN];
					strncpy(executedCommand, expandedQueriedPath, INPUT_LEN);
					strncat(executedCommand, modifiedInput, INPUT_LEN);

					int status = system(executedCommand);
					if (status != 0)
					{
						printError(command, "Thrown error");
						lastCommandSuccess = false;
					}

					foundCommand = true;
					break;
				}
			}
		}

		add_history(unmodifiedInput);

		if (!foundCommand)
		{
			printCommandNotFoundError(command);
			lastCommandSuccess = false;
		}
	}
}
