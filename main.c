// ToDo:
// ^Configure
// ^Use argument as default context

#include <stdio.h>
#include "functions.h"

int main(int argc, char *argv[]) {
    readConfig(); // Read config
    if(argc >= 2) {
	action(argv[0], argv[1]);
	return 0;
    }

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
