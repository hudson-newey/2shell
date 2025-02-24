#include <unistd.h>

void
cdShell(char *path)
{
    chdir(path);
}
