#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

/* #include "sds.h" */

#include "paths.c"
#include "errors.c"

#include "../built-ins/exit.c"
#include "../built-ins/cd.c"

#define ARG_LEN 128
#define BUFFER_LINE_LEN 256
#define MAX_PATHS 512

#define CMD_SEPARATOR " "

#define DEFAULT_CWD "~"
#define SHELL_NAME "2sh"

#define PATH_ENV_VAR "PATH"
#define PATH_SPLIT_CHAR ":"

#define USER_ENV_VAR "USER"

#define COLOR_RESET "\x1b[0m"
#define FAILURE_RED(string) "\x1b[31m" string COLOR_RESET
#define SUCCESS_GREEN(string) "\x1b[32m" string COLOR_RESET

#define SUCCESS_PROMPT SUCCESS_GREEN(" > ")
#define FAILURE_PROMPT FAILURE_RED(" > ")

static volatile int sigint_received = 0;

// this is a signal handler that does nothing
// it is used to prevent the shell from exiting
// when the user presses ctrl+c in the interactive terminal
void
intHandler()
{
	sigint_received = 1;
	rl_replace_line("", 0);
	rl_refresh_line(0, 0);
}

int
runArgsCommand(char *command)
{
	if (!strncmp(command, "exit", strlen("exit")))
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

	signal(SIGINT, intHandler);

	char *pathEnv = getenv(PATH_ENV_VAR);
	setenv("PATH", pathEnv, true);
	char *currentUser = getenv(USER_ENV_VAR);

	char initialDir[sizeof("/home/") + sizeof(currentUser) + sizeof("/") + 1];
	strlcpy(initialDir, "/home/", strlen(initialDir) + strlen("/home/") + 1);
	strlcat(initialDir, currentUser, strlen(initialDir) + strlen(currentUser) + 1);
	strlcat(initialDir, "/", strlen(initialDir) + strlen("/") + 1);
	cdShell(initialDir);

	char *splitPaths[MAX_PATHS] = {0};
	size_t pathsCount = 0;
	static size_t const maxTokenCount = sizeof(splitPaths) / sizeof(splitPaths[0]);
	for (char* path = strtok(pathEnv, PATH_SPLIT_CHAR); path != NULL && pathsCount != maxTokenCount; path = strtok(NULL, PATH_SPLIT_CHAR))
	{
	    splitPaths[pathsCount++] = path;
	}

	char currentDir[1024] = DEFAULT_CWD;

	bool lastCommandSuccess = true;

	printf("\e[1;1H\e[2J");
	while (true)
	{
		char bufferlinePrompt[BUFFER_LINE_LEN];
		strncpy(bufferlinePrompt, "\x1b[36m", sizeof(bufferlinePrompt));
		strcat(bufferlinePrompt, shortenPath(currentDir, currentUser));
		strncat(bufferlinePrompt, "\x1b[0m", strlen(bufferlinePrompt));

		if (lastCommandSuccess)
		{
			strcat(bufferlinePrompt, SUCCESS_PROMPT);
		}
		else
		{
			strcat(bufferlinePrompt, FAILURE_PROMPT);
		}

		char *input = readline(bufferlinePrompt);
		if (!strcmp(input, ""))
		{
			free(input);
			continue;
		}

		input[strcspn(input, "\n")] = 0;
		char unmodifiedInput[strlen(input)];
		strncpy(unmodifiedInput, input, strlen(input));

		// this modified input is used to have inbuilt aliases
		// e.g. ls is mapped to ls --color
		// these inbuilt mappings are typically just visual
		char modifiedInput[sizeof(input) + 255];
		strlcpy(modifiedInput, input, strlen(modifiedInput) + strlen(input) + 1);

		char *userCommand[ARG_LEN] = {0};
		size_t argCount = 0;
		static size_t const maxCommandTokenCount = sizeof(userCommand) / sizeof(userCommand[0]);
		for (char* arg = strtok(input, CMD_SEPARATOR); arg != NULL && argCount != maxCommandTokenCount; arg = strtok(NULL, CMD_SEPARATOR))
		{
			userCommand[argCount++] = arg;
		}

		char *command = userCommand[0];
		bool foundCommand = false;

		if (!strncmp(command, "exit", strlen("exit")) || !strncmp(command, "q", strlen("q")))
		{
			exitShell();
		}
		else if (!strncmp(command, "cd", strlen("cd")))
		{
			// for some reason, the bash behavior is to not throw
			// an error if the cd command is run without a path
			// I replicate this behavior in my shell
			if (argCount < 2) {
				add_history(unmodifiedInput);
				free(input);
				lastCommandSuccess = false;
				continue;
			}

			// using the chdir() function communicates to
			// programs like "ls" and "pwd" what directory
			// we are currently looking at
			int respCode = cdShell(userCommand[1]);
			lastCommandSuccess = !respCode;

			if (respCode == -1)
			{
				char errorMsg[256];
				strcpy(errorMsg, userCommand[1]);
				strcat(errorMsg, ": No such file or directory");
				printError("cd", errorMsg);
			}
			else
			{
				// Note: Not updating the directory here is not in line with
				// the behavior of other shells.
				// In other shells, if the directory that you were in gets
				// deleted and a cd commands fails, your working directory will
				// be updated.
				//
				// However, changing the working directory is on a failure is
				// "expensive" and doesn't seem to provide much benefit for a
				// command line; not scripting shell (which 2sh is targeting).
				//
				// Therefore, to make the shell slightly faster in this use case
				// I have purposely broken compatibility with shells like bash
				// and zsh.
				// char *newPathValue = getenv("PWD");

				char newDir[255];
				getcwd(newDir, sizeof(newDir));

				strlcpy(currentDir, newDir, strlen(currentDir) + strlen(newDir) + 1);
			}

			// Because we short-circut the cd command so that we
			// don't end up searching PATH, we do not hit the
			// condition to add_history after the PATH command
			// was executed.
			//
			// I didn't want to de-duplicate code by adding to the
			// history before the command was executed so that the
			// command the user requested is executed as soon as
			// possible.
			add_history(unmodifiedInput);
			free(input);
			continue;
		}
		else if (!strncmp(command, "export", strlen("export")))
		{
			if (argCount < 3)
			{
				lastCommandSuccess = false;
				add_history(unmodifiedInput);
				printf("3\n");
				free(input);
				continue;
			}

			// TODO: Check this can be inlined by the optimizer
			// I have only assigned userCommand[1] and [2] to a
			// variable so that the code reads better, and I
			// assume that the compiler can optimize away this
			// pointer allocation.
			char *envVarName = userCommand[1];
			char *envVarValue = userCommand[2];

			setenv(envVarName, envVarValue, 1);

			add_history(unmodifiedInput);
			free(input);
			continue;
		}

		if (!strncmp(command, "ls", strlen("ls")))
		{
			strlcat(modifiedInput, " --color", strlen(modifiedInput) + strlen(" --color") + 1);
		}
		else if (!strncmp(command, "grep", strlen("grep")))
		{
			strlcat(modifiedInput, " --color", strlen(modifiedInput) + strlen(" --color") + 1);
		}

		// I think that the local path is the most likely to contain
		// the requested exectable.
		// Therefore, I perform the O(n) operation for the local path
		// query first.
		//
		// TODO: This ordering should probably be conditional on if
		// the command starts with a ./
		char localQueryPath[sizeof(currentDir) + sizeof(command)];
		strlcpy(localQueryPath, currentDir, strlen(localQueryPath) + strlen(currentDir) + 1);
		strlcat(localQueryPath, command, strlen(localQueryPath) + strlen(command) + 1);

		char expandedLocalPath[1024];
		strcpy(expandedLocalPath, localQueryPath);
		expandPath(expandedLocalPath, currentUser, false);

		if (!access(localQueryPath, F_OK))
		{
			lastCommandSuccess = true;

			int status =  system(modifiedInput);
			if (status != 0)
			{
				char errorMessage[1028];
				snprintf(errorMessage, sizeof(errorMessage), "Thrown error (%d)", status);

				printError(command, errorMessage);
				lastCommandSuccess = false;
			}

			foundCommand = true;
			break;
		}
		else
		{
			for (unsigned int i = 0; i < pathsCount; i++)
			{
				char expandedQueriedPath[1024] = {};
				strlcpy(expandedQueriedPath, splitPaths[i], strlen(expandedQueriedPath) + strlen(splitPaths[i]) + 1);
				expandPath(expandedQueriedPath, currentUser, true);

				char queriedPath[strlen(expandedQueriedPath) + strlen(command) + 1] = {};
				strlcpy(queriedPath, expandedQueriedPath, strlen(queriedPath) + strlen(expandedQueriedPath) + 1);
				strlcat(queriedPath, command, strlen(queriedPath) + strlen(command) + 1);

				if (!access(queriedPath, F_OK))
				{
					lastCommandSuccess = true;

					char executedCommand[sizeof(expandedQueriedPath) + sizeof(modifiedInput)];
					strlcpy(executedCommand, expandedQueriedPath, strlen(executedCommand) + strlen(expandedQueriedPath) + 1);
					strlcat(executedCommand, modifiedInput, strlen(executedCommand) + strlen(modifiedInput) + 1);

					int status = system(executedCommand);
					if (status != 0)
					{
						char errorMessage[1028];
						snprintf(errorMessage, sizeof(errorMessage), "Thrown error (%d)", status);

						printError(command, errorMessage);
						lastCommandSuccess = false;
					}

					foundCommand = true;
					break;
				}
			}
		}

		if (!foundCommand)
		{
			printCommandNotFoundError(command);
			lastCommandSuccess = false;
		}

		// we could add_history first, to de-duplicate logic
		// however, I wanted to execute the command that the user
		// requested as fast as possible, so that there is feedback to
		// provide to the user as soon as they press the enter key
		add_history(unmodifiedInput);

		// Same with add_history, I could de-duplicate the free() logic by
		// freeing the input at the top of the while loop.
		// However, freeing heap memory is a relatively expensive operation
		// and I want to print output to the screen as soon as possible.
		// Therefore, I free the input last so that the user gets feedback
		// as soon as possible.
		free(input);
	}

	// Do not bother freeing the sds currentDir variable because this is the
	// end of the program, and the operating system will take care of
	// reclaiming the memory once we terminate.
	// Not freeing the memory here makes the program slightly faster when
	// exiting.

	return 0;
}
