// ToDo:
// ^Configure

#include <stdio.h>
#include "functions.h"

extern int pseudoGui;

int main(int argc, char *argv[]) {
    printf("Realtime users for asterisk\n");
    readConfig(); // Read config
    if(argc >= 2) action(argv[0], argv[1]);
    printf("Use \033[0;33m%s --help\033[0m for more information.\n\n", argv[0]);

    checkServices(); // Check that mysqldb available
    if(pseudoGui == 0) menuDriver();

    return 0;
}
