#include <unistd.h>

int
cdShell(char *path)
{
    return chdir(path);
}
