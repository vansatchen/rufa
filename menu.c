#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "functions.h"

extern char *defaultContext;

char menu() {
    char *mainChoice = NULL;
    size_t n = 0;
    printf("1) Quit\n");
    printf("2) Add\n");
    printf("3) Delete\n");
    printf("4) Show\n");
    printf("5) Edit\n");
    printf("6) Make config\n");
    extern int *curlFound;
    if(curlFound) printf("7) Reboot remote phone\n");
    printf("Enter your choice: ");
    getline(&mainChoice, &n, stdin);
    return *mainChoice;
}

char menuEdit() {
    char *editChoice = NULL;
    size_t n = 0;
    printf("1) Quit\n");
    printf("2) Edit basic options\n");
    printf("3) Edit additional options\n");
    printf("Enter your choice: ");
    getline(&editChoice, &n, stdin);
    return *editChoice;
}

char menuShow() {
    char *showChoice = NULL;
    size_t n = 0;
    printf("1) Quit\n");
    printf("2) Show by number\n");
    printf("3) Show by callerid\n");
    printf("4) Show by status\n");
    printf("5) Show account\n");
    printf("Enter your choice: ");
    getline(&showChoice, &n, stdin);
    return *showChoice;
}

void action(char *selfName, char *actionItem) {
    extern char *rufaVersion;
    if(!strcmp(actionItem, "--help")) {
	printf("Usage: %s [OPTION]\n", selfName);
	printf("Options:\n");
	printf("\t--help\t\tdisplay this help and exit\n");
	printf("\t--createdb\tCreate database and exit\n");
	printf("\t--truncatedb\tTruncate database and exit\n");
	printf("\t--version\tShow version and exit\n\n");
	printf("\tsomecontext\tUse 'somecontext' as default context\n\n");
    } else if(!strcmp(actionItem, "--createdb")) {
	createdb();
    } else if(!strcmp(actionItem, "--truncatedb")) {
	truncateAnswer();
    } else if(!strcmp(actionItem, "--version")) {
	printf("Version: %s\n", rufaVersion);
	printf("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n"
		"This is free software: you are free to change and redistribute it.\n"
		"There is NO WARRANTY, to the extent permitted by law.\n\n"
		"Written by Nikolay Pavlov A.\n");
    } else {
	defaultContext = actionItem;
	printf("Using \033[0;33m%s\033[0m as default context\n\n", defaultContext);
    }
}
