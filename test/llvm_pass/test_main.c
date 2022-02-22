// A vulnerable piece of code in a standalone project
//
// Compile gcc -g -m32 -fno-stack-protector main.c -o manual_overflow
// A vulnerability with x86-64 exec file does not work

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

#define LINE_MAX_LEN 36

//
// Additional stuff to test in IR (can be commented without breaking the
// logic)
//
int GLOBAL_VARIABLE = 666;

struct A {
    struct B {
        long* j;
        char** k;
        bool l;
        struct C {
           char n;
        } m;
    } a;
    char b;
    int* c;
    unsigned short int d;
    float e;
    double f;
    bool g;
    long h;
    int i;
};

//typedef struct A* (*struct_initialization)();

struct A* initialize_struct() {
    struct A* collection = (struct A*) malloc(sizeof(struct A));
    collection->a.j = NULL;
    collection->a.k = NULL;
    collection->a.l = false;
    collection->i = 228;
    collection->b = 'a';
    collection->c = &collection->i;
    collection->d = 1000;
    collection->e = 3.14;
    collection->f = 2.9;
    collection->g = true;
    collection->h = 5;
    collection->a.m.n = 'b';

    return collection;
}

/*
struct A* initialize_struct_through_function(struct_initialization pointer) {
    return pointer();
}
*/

int* initialize_array(int number_of_bytes) {
    if (number_of_bytes < 1) {
        return NULL;
    }

    int* array = (int*) malloc(number_of_bytes);

    return array;
}

void print_characters(char characters[3]) {
    struct A* collection = initialize_struct();
    int* array = initialize_array(10);
    free(collection);
    free(array);

    printf("%c%c%c\n", *characters, *(characters + 1), *(characters + 2));
}

//

//
// NIST CWE 119, Example 2 (https://cwe.mitre.org/data/definitions/119.html)
//
// Payload: &&&&&&&&&
char *copy_input(char *user_supplied_string){
#define MAX_SIZE 10
    int i, dst_index;
    char *dst_buf = (char*)malloc(4 * sizeof(char) * MAX_SIZE);
    if ( MAX_SIZE <= strlen(user_supplied_string) ) {
        printf("user string too long, die evil hacker!");
        exit(1);
        //die("user string too long, die evil hacker!");
    }
    dst_index = 0;
    for ( i = 0; i < strlen(user_supplied_string); i++ ){
        if ( '&' == user_supplied_string[i] ) {
            dst_buf[dst_index++] = '&';
            dst_buf[dst_index++] = 'a';
            dst_buf[dst_index++] = 'm';
            dst_buf[dst_index++] = 'p';
            dst_buf[dst_index++] = ';';
        } else if ('<' == user_supplied_string[i] ) {
        /* encode to &lt; */
        } else {
            dst_buf[dst_index++] = user_supplied_string[i];
        }
    }

    return dst_buf;
}
//

//
// NIST CWE 126, Example 3 (https://cwe.mitre.org/data/definitions/119.html)
//
void BufferOverRead() {
    char *items[] = {"boat", "car", "truck", "train"};
    int index = 10/*GetUntrustedOffset()*/;
    printf("You selected %s\n", items[index-1]);
}
//

//
// Main blocks of vulnerable piece of code
//
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

    int counter = 0;
    for (int ii = 0; ii < line_len; ++ii) {
        counter += ii;
        if (counter % 2 == 0) {
            counter /= 2;
        }
    }

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
//

//
// libcurl vulnerable function (https://habr.com/ru/post/259671/)
//
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
//

int main(int argc, char** argv) {
    if (argc != 2) {
        exit(1);
    }

    //
    // Additional stuff to test in IR (can be commented without breaking the
    // logic)
    //
    char characters[3] = {'x', 'y', 'z'};
    print_characters(characters);
    //

    //
    // NIST BO
    //
    char* vuln_payload = "&&&&&&&&&";
    copy_input(vuln_payload);
    //+
    BufferOverRead();
    //

    //
    // Main code with a vulnerable function (BO)
    //
    un_init(argv[1]);
    //

    //
    //
    //
    sanitize_cookie_path('\"');
    //

    return 0;
}