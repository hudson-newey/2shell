#include <stdio.h>

#include "paths.c"

void
bufferline(char *currentDir, char *currentUser)
{
	printf("%s > ", shortenPath(currentDir, currentUser));
}
