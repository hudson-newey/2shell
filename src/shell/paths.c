#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// copied from https://stackoverflow.com/a/31775567
// TODO: replace this with a correct custom implementation
/* returns number of strings replaced */
int
strReplace(char *line, const char *search, const char *replace)
{
	int count;
	char *sp; // start of pattern

	//printf("replacestr(%s, %s, %s)\n", line, search, replace);
	if ((sp = strstr(line, search)) == NULL)
	{
		return(0);
	}
	count = 1;
	int sLen = strlen(search);
	int rLen = strlen(replace);
	if (sLen > rLen)
	{
		// move from right to left
		char *src = sp + sLen;
		char *dst = sp + rLen;
		while((*dst = *src) != '\0')
		{
			dst++;
			src++;
		}
	}
	else if (sLen < rLen)
	{
		// move from left to right
		int tLen = strlen(sp) - sLen;
		char *stop = sp + rLen;
		char *src = sp + sLen + tLen;
		char *dst = sp + rLen + tLen;
		while(dst >= stop)
		{
			*dst = *src;
			dst--; src--;
		}
	}
	memcpy(sp, replace, rLen);

	count += strReplace(sp + rLen, search, replace);

	return(count);
}

void
expandPath(char *path, char *currentUser, bool suffixSlash)
{
	size_t pathLen = strlen(path);
	char lastPathChar = path[pathLen - 1];

	if (suffixSlash && lastPathChar != '/')
	{
		strcat(path, "/");
	}

	char homePath[128] = "/home/";
	strcat(homePath, currentUser);
	strcat(homePath, "/");

	strReplace(path, "~/", homePath);
}

char*
shortenPath(char *path, char *currentUser)
{
	char homePath[128] = "/home/";
	strcat(homePath, currentUser);

	// TODO: remove this special case
	if (!strncmp(homePath, path, strlen(path)))
	{
		return "~";
	}

	strcat(homePath, "/");

	strReplace(path, homePath, "~/");

	return path;
}
