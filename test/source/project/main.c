#include "sanitize_cookie_path.h"
#include "copy_input.h"

#define LINE_MAX_LEN 36

int GLOBAL_VARIABLE = 666;
int* global_pointer;

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

    ++GLOBAL_VARIABLE;

    return collection;
}

int* initialize_array(int number_of_bytes) {
    if (number_of_bytes < 1) {
        return NULL;
    }

    int* array = (int*) malloc(number_of_bytes);
    global_pointer = array;

    return array;
}

void print_characters(char characters[3]) {
    struct A* collection = initialize_struct();
    int* array = initialize_array(10);
    free(collection);
    free(array);

    printf("%c%c%c\n", *characters, *(characters + 1), *(characters + 2));
}

void BufferOverRead() {
    char *items[] = {"boat", "car", "truck", "train"};
    int index = 10/*GetUntrustedOffset()*/;
    printf("You selected %s\n", items[index-1]);
}

bool is_valid(char* line) {
    if (line != NULL) {
        return true;
    }

    return false;
}

int get_line_len(char* line) {
    ++GLOBAL_VARIABLE;

    return strlen(line);
}

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

int main(int argc, char** argv) {
    if (argc != 2) {
        exit(1);
    }

    char characters[3] = {'x', 'y', 'z'};
    print_characters(characters);

    char* vuln_payload = "&&&&&&&&&";
    copy_input(vuln_payload);
    BufferOverRead();

    un_init(argv[1]);

    sanitize_cookie_path('\"');
    
    return 0;
}
