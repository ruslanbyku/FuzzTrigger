#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

static char* sanitize_cookie_path(const char* cookie_path) {
    size_t len;
    char* new_path = strdup(cookie_path);
    if (!new_path) {
        return NULL;
    }

    if (new_path[0] == '\"') {
        memmove((void *)new_path, (const void*)(new_path + 1), strlen(new_path));
    }
    if (new_path[strlen(new_path) - 1] == '\"') {
        new_path[strlen(new_path) - 1] = 0x0;
    }

    if (new_path[0] !='/') {
        free(new_path);
        new_path = strdup("/");
        return new_path;
    }

    len = strlen(new_path);
    if (1 < len && new_path[len - 1] == '/') {
        new_path[len - 1] = 0x0;
    }

    return new_path;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        exit(1);
    }

    sanitize_cookie_path('\"');

    return 0;
}