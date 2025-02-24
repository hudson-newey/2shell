#include <unistd.h>

int
cdShell(char *path)
{
    chdir(path);
    return 0;
}
