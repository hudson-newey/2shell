#include <stdio.h>

void
printError(char *program, char *error)
{
	printf("%s: %s\n", program, error);
}

void
printCommandNotFoundError(char *program)
{
	printf("2sh: command not found: %s\n", program);
}
