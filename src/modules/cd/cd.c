#include <stdio.h>

int
execute(char *path)
{
    printf("Executing %s\n", path);
    return 0;
}

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <path>\n", argv[0]);
        return 1;
    }

    char *path = argv[1];

    return execute(path);
}

