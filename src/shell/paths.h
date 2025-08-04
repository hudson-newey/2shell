#ifndef PATHS_H_
#define PATHS_H_

char *expandPath(char *path, char *currentUser, bool suffixSlash);
char *shortenPath(char *path, char *currentUser);

#endif
