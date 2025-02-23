#include <string.h>

char*
expandPath(char *path, char *currentUser)
{
	size_t pathLen = strlen(path);
	char lastPathChar = path[pathLen - 1];

	if (lastPathChar != '/') {
		strcat(path, "/");
	}

	return path;
}

char*
shortenPath(char *path, char *currentUser)
{
	return path;
}
