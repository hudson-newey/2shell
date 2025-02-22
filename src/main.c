#include "./shell.c"

int
main(int argc, char *argv[])
{
	if (argc > 1) {
		return runShellCommand(argv[1]);
	}

	return runShell();
}
