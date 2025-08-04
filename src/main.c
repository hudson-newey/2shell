#include "shell/shell.h"

int
main(int argc, char *argv[])
{
	if (argc > 1)
	{
		return runArgsCommand(argv[1]);
	}

	return runShell();
}
