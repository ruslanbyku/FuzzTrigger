#include "copy_input.h"

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
