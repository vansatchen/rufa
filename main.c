// ToDo:
// ^Configure
// ^Show option with show only accounts by context
// ^Data too long for column 'callerid' at row 1 -- locale problems

#include <stdio.h>
#include "functions.h"

int main(int argc, char *argv[]) {
    printf("Realtime users for asterisk\n");
    readConfig(); // Read config
    if(argc >= 2) action(argv[0], argv[1]);

    printf("Use \033[0;33m%s --help\033[0m for more information.\n\n", argv[0]);
    checkServices(); // Check that mysqldb and curl available
    do {
	mainChoice = menu();
	switch (mainChoice) {
	    case '1': // Quit
		printf("\n");
		printf("Bye!\n");
		break;
	    case '2': // Add
		add();
		break;
	    case '3': // Del
		del();
		break;
	    case '4': // Show
		showChoice = menuShow();
		switch (showChoice) {
		    case '1': // Quit
			printf("---------\n");
			printf("Back to main menu\n");
			printf("---------\n");
			break;
		    case '2': // Show by number
			showByOption("username");
			printf("\n");
			break;
		    case '3': // Show by callerid
			showByOption("callerid");
			printf("\n");
			break;
		    case '4': // Show by status
			showByOption("via_addr DESC");
			printf("\n");
			break;
		    case '5': // Show account
			showAccount();
			printf("\n");
			break;
		    default:
			printf("---------\n");
			printf("Operation not supported yet!\n");
			printf("---------\n");
		}
		break;
	    case '5': // Edit
		editChoice = menuEdit();
		switch (editChoice) {
		    case '1': // Quit
			printf("---------\n");
			printf("Back to main menu\n");
			printf("---------\n");
			break;
		    case '2': // Edit basic options
			baseEdit();
			break;
		    case '3': // Edit additional options
			additionalEdit();
			break;
		    default:
			printf("---------\n");
			printf("Operation not supported yet!\n");
			printf("---------\n");
		}
		break;
	   case '6': // Make config
		makeConfig();
		break;
	   case '7': // Reboot remote phone
		rebootRemotePhone();
		break;
	   default:
		printf("---------\n");
		printf("Operation not supported yet!\n");
		printf("---------\n");
	}
    } while(mainChoice != '1');
    return 0;
}
