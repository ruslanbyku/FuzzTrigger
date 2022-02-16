// A vulnerable piece of code in a standalone project
//
// Compile gcc -g -m32 -fno-stack-protector main.c -o manual_overflow
// A vulnerability with x86-64 exec file does not work

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#define LINE_MAX_LEN 36

bool is_valid(char* line) {
    if (line != NULL) {
        return true;
    }

    return false;
}

int get_line_len(char* line) {
    return strlen(line);
}

// Input to to make an overflow: aaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbAAAA
int un_init(char* line) {
    char buffer[32]; // 31 bytes + \0
    // Variable initialization order matters!
    int line_len = 0;
    int logged_in = 0;

    if (!is_valid(line)) {
        return logged_in;
    }

    line_len = get_line_len(line);

    if (line_len > LINE_MAX_LEN) {
        fprintf(stdout, "String too long!\n");
        return logged_in;
    } else {
        strncpy(buffer, line, line_len);
    }

    if (logged_in == 0x41414141) {
        fprintf(stdout, "Buffer overflow on stack occurred.\n");
    }

    // authentication process continues
    // success
    logged_in = 1;

    return logged_in;
}

/*
int function_main(int argc, char** argv) {
    if (argc != 2) {
        exit(1);
    }

    un_init(argv[1]);

    return 0;
}
*/

// Rewrite the project main() in such a way so that LLVMFuzzerTestOneInput can substitute one.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    un_init(reinterpret_cast<const char*>(data));

    return 0;
}

