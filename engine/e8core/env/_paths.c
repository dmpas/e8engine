#include "_paths.h"
#include <stdbool.h>
#include <string.h>

static bool inline __is_delimiter(char c)
{
    return c == '\\' || c == '/';
}

/*
static int __next_path_element_end(const char *path, int start)
{
    int r = start;
    while (path[r] != 0 && !__is_delimiter(path[r]))
        ++r;

    return r;
}
*/
void normalize_path(char *path)
{
    /* TODO: normalize_path
    int i = 0;
    int r;

    while ( (r = __next_path_element_end(path, i)) > i) {
        * char part[MAX_PATH]; *
        i = r;
    }
    */
}

void extract_directory(const char *path, char *dir)
{
    int r = strlen(path) - 1;
    while ( r > 0 && !__is_delimiter(path[r]) )
        --r;
    strncpy(dir, path, r);
    dir[r] = 0;
}

void add_path(char *path, const char *addition)
{
    if (!__is_delimiter( path[strlen(path) - 1]) )
        strcat(path, "/");

    strcat(path, addition);
    normalize_path(path);
}
