#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "functions.h"

// Defaults
extern char *defaultContext;
extern char defaultPass[10];
extern int numberLen;
extern char *pbxHost;
extern char *phoneAdminPass;
extern char *enableVlan;
extern char *vlan;
extern char *dispLang;
extern char *timeZone;
extern char *ntpServer;
extern char *phoneUserPass;
extern char checkPath[100];
extern char *checkAdminPass;
extern int useSimplePass;
int curlFound = 0;
extern char *curlPath;

void printDone() {
    printf("----------------\n");
    printf("Done!\n");
    printf("----------------\n");
}

char *checkNumberExists() {
    char *pjsipName;
    size_t n = 0;
    ssize_t nameLen;

    int flagCorrectName = 0;
    do {
        if(flagCorrectName == 1) printf("\033[0;31mName/number must have %d digitals only, please enter another: \033[0m", numberLen);
        if(flagCorrectName == 2) printf("\033[0;31mName/number \033[0;33m%s\033[0;31m already exists, please enter another: \033[0m", pjsipName);
        nameLen = getline(&pjsipName, &n, stdin);
        pjsipName[strcspn(pjsipName, "\n")] = 0;
        if(nameLen != numberLen + 1) { // Check number len
            flagCorrectName = 1;
        } else {
            flagCorrectName = 0;
            for(int i = 0; i < strlen(pjsipName) && pjsipName[i] != '\n'; ++i) {
                if(pjsipName[i] >= '0' && pjsipName[i] <= '9') { // Check that pjsipName is digit
                    flagCorrectName = 0;
                } else {
                    flagCorrectName = 1;
                    break;
                }
            }
            if(flagCorrectName == 0) flagCorrectName = checkNumberExist(pjsipName); // Check that pjsipName exists in mysql
        }
    } while(flagCorrectName == 1 || flagCorrectName == 2);
    return pjsipName;
}

char *checkNumberNotExists() {
    char *pjsipName;
    size_t n = 0;
    ssize_t nameLen;

    int flagCorrectName = 0;
    do {
        if(flagCorrectName == 1) printf("\033[0;31mName/number must have %d digitals only, please enter another: \033[0m", numberLen);
        if(flagCorrectName == 2) printf("\033[0;31mName/number \033[0;33m%s\033[0;31m not exists, please enter another: \033[0m", pjsipName);
        nameLen = getline(&pjsipName, &n, stdin);
        pjsipName[strcspn(pjsipName, "\n")] = 0;
        if(nameLen != numberLen + 1) { // Check number len
            flagCorrectName = 1;
        } else {
            flagCorrectName = 0;
            for(int i = 0; i < strlen(pjsipName) && pjsipName[i] != '\n'; ++i) {
                if(pjsipName[i] >= '0' && pjsipName[i] <= '9') { // Check that pjsipName is digit
                    flagCorrectName = 0;
                } else {
                    flagCorrectName = 1;
                    break;
                }
            }
            if(flagCorrectName == 0) flagCorrectName = checkNumberExist(pjsipName); // Check that pjsipName exists in mysql
            if(flagCorrectName == 0) {
                flagCorrectName = 2;
            } else {
                if(flagCorrectName == 2) flagCorrectName = 0;
            }
        }
    } while(flagCorrectName == 1 || flagCorrectName == 2);
    return pjsipName;
}

void add() {
    char *pjContext, *pjsipName, *pjPassword, *pjCallerid, *pjCalleridWONum;
    size_t n0 = 0, n2 = 0, n3 = 0;

    printf("Group(context)[%s]: ", defaultContext);
    getline(&pjContext, &n0, stdin);
    pjContext[strcspn(pjContext, "\n")] = 0;
    if(strlen(pjContext) == 0) strcpy(pjContext, defaultContext); // If pjContext empty, set to defaultContext

    printf("Name/number: ");
    pjsipName = checkNumberExists();

    if(useSimplePass == 1) {
	printf("Using default post-pass \033[0;33m%s\033[0m\n", defaultPass);
	pjPassword = calloc(strlen(pjsipName) + strlen(defaultPass) + 1, 1);
	strcat(pjPassword, pjsipName);
	strcat(pjPassword, defaultPass);
    } else {
	do {
	    printf("Password: ");
	    getline(&pjPassword, &n2, stdin);
	    pjPassword[strcspn(pjPassword, "\n")] = 0;
	    if(strlen(pjPassword) == 0) printf("\033[0;31mPassword cannot be empty!\033[0m\n");
	} while(strlen(pjPassword) == 0);
    }

    printf("CallerID[%s]: ", pjsipName);
    getline(&pjCalleridWONum, &n3, stdin);
    pjCalleridWONum[strcspn(pjCalleridWONum, "\n")] = 0;
    if(strlen(pjCalleridWONum) == 0) strcpy(pjCalleridWONum, pjsipName); // If pjCallerid empty, set to pjsipName
    char calleridIn[3] = " <", calleridOut[3] = ">";
    pjCallerid = calloc(strlen(pjCalleridWONum) + strlen(calleridIn) + strlen(pjsipName) + strlen(calleridOut) + 1, 1);
    strcat(pjCallerid, pjCalleridWONum);
    strcat(pjCallerid, calleridIn);
    strcat(pjCallerid, pjsipName);
    strcat(pjCallerid, calleridOut);

    addToMysql(pjContext, pjsipName, pjPassword, pjCallerid);

    free(pjPassword);
    free(pjCallerid);
    printDone();
}

void del() {
    char *pjsipName;
    size_t n = 0;

    printf("Name/number to delete: ");
    pjsipName = checkNumberNotExists();

    delFromMysql(pjsipName);

    free(pjsipName);
    printDone();
}

void baseEdit() {
    char *pjContext, *pjsipName, *pjNewName, *pjPassword, *pjCallerid, *pjCalleridWONum;
    size_t n0 = 0, n1 = 0, n2 = 0, n3 = 0, n4 = 0;

    printf("Name/number: ");
    pjsipName = checkNumberNotExists();

    char *currentContext = getCurrentOption(pjsipName, "context");
    printf("Group(context)[%s]: ", currentContext);
    getline(&pjContext, &n0, stdin);
    pjContext[strcspn(pjContext, "\n")] = 0;
    if(strlen(pjContext) == 0) strcpy(pjContext, currentContext); // If pjContext empty, set to currentContext

    printf("New name/number[%s]: ", pjsipName);
    getline(&pjNewName, &n1, stdin);
    pjNewName[strcspn(pjNewName, "\n")] = 0;
    if(strlen(pjNewName) == 0) {
	strcpy(pjNewName, pjsipName); // If pjNewName not empty, set to pjsipName
    } else {
	ssize_t nameLen; // Check that number digital, have needed length and not exists
	int flagCorrectName = 0;
	do {
	    if(flagCorrectName == 1) printf("\033[0;31mName/number must have %d digitals only, please enter another: \033[0m", numberLen);
	    if(flagCorrectName == 2) printf("\033[0;31mName/number \033[0;33m%s\033[0;31m already exists, please enter another: \033[0m", pjsipName);
	    if(flagCorrectName == 1 || flagCorrectName == 2) {
		nameLen = getline(&pjNewName, &n4, stdin);
		pjNewName[strcspn(pjNewName, "\n")] = 0;
	    } else {
		nameLen = strlen(pjNewName) + 1;
	    }
	    if(nameLen != numberLen + 1) { // Check number len
		flagCorrectName = 1;
	    } else {
		flagCorrectName = 0;
		for(int i = 0; i < strlen(pjNewName) && pjNewName[i] != '\n'; ++i) {
		    if(pjNewName[i] >= '0' && pjNewName[i] <= '9') { // Check that pjsipName is digit
			flagCorrectName = 0;
		    } else {
			flagCorrectName = 1;
			break;
		    }
		}
		if(flagCorrectName == 0) flagCorrectName = checkNumberExist(pjNewName); // Check that pjsipName exists in mysql
	    }
	} while(flagCorrectName == 1 || flagCorrectName == 2);
    }

    char *currentPassword = getCurrentOption(pjsipName, "password");
    printf("Password[%s]: ", currentPassword);
    getline(&pjPassword, &n2, stdin);
    pjPassword[strcspn(pjPassword, "\n")] = 0;
    if(strlen(pjPassword) == 0) strcpy(pjPassword, currentPassword); // If pjPassword empty, set to currentPassword

    char *currentCallerid = getCurrentOption(pjsipName, "callerid");
    printf("CallerID[%s]: ", currentCallerid);
    getline(&pjCalleridWONum, &n3, stdin);
    pjCalleridWONum[strcspn(pjCalleridWONum, "\n")] = 0;
    if(strlen(pjCalleridWONum) == 0) {
	pjCallerid = currentCallerid;
    } else {
	char calleridIn[3] = " <", calleridOut[3] = ">";
	pjCallerid = calloc(strlen(pjCalleridWONum) + strlen(calleridIn) + strlen(pjsipName) + strlen(calleridOut) + 1, 1);
	strcat(pjCallerid, pjCalleridWONum);
	strcat(pjCallerid, calleridIn);
	strcat(pjCallerid, pjsipName);
	strcat(pjCallerid, calleridOut);
    }

    updateBaseToMysql(pjContext, pjsipName, pjNewName, pjPassword, pjCallerid);

    printDone();
}

void additionalEdit() {
    char *pjsipName, *pjMaxContacts, *pjTransport, *pjMediaEncryption, *pjCodecs;
    size_t n0 = 0, n1 = 0, n2 = 0, n3 = 0;

    printf("Name/number: ");
    pjsipName = checkNumberNotExists();

    char *currentMaxContacts = getCurrentOption(pjsipName, "maxcontacts");
    printf("Max_contacts[%s]: ", currentMaxContacts);
    getline(&pjMaxContacts, &n0, stdin);
    pjMaxContacts[strcspn(pjMaxContacts, "\n")] = 0;
    if(strlen(pjMaxContacts) == 0) strcpy(pjMaxContacts, currentMaxContacts); // If pjContext empty, set to currentContext

    char *currentTransport = getCurrentOption(pjsipName, "transport");
    printf("Transport[%s]: ", currentTransport);
    getline(&pjTransport, &n1, stdin);
    pjTransport[strcspn(pjTransport, "\n")] = 0;
    if(strlen(pjTransport) == 0) strcpy(pjTransport, currentTransport); // If pjTransport empty, set to currentTransport

    char *currentMediaEncryption = getCurrentOption(pjsipName, "mediaencryption");
    printf("Media_encryption[%s]: ", currentMediaEncryption);
    getline(&pjMediaEncryption, &n2, stdin);
    pjMediaEncryption[strcspn(pjMediaEncryption, "\n")] = 0;
    if(strlen(pjMediaEncryption) == 0) strcpy(pjMediaEncryption, currentMediaEncryption); // If pjMediaEncryption empty, set to currentMediaEncryption

    char *currentCodecs = getCurrentOption(pjsipName, "codecs");
    printf("Codecs[%s]: ", currentCodecs);
    getline(&pjCodecs, &n3, stdin);
    pjCodecs[strcspn(pjCodecs, "\n")] = 0;
    if(strlen(pjCodecs) == 0) strcpy(pjCodecs, currentCodecs); // If pjCodecs empty, set to currentCodecs

    updateBaseToMysql(pjMaxContacts, pjTransport, pjMediaEncryption, pjCodecs);

    free(pjsipName);
    free(pjMaxContacts);
    free(pjTransport);
    free(pjMediaEncryption);
    free(pjCodecs);
    printDone();
}

void makeConfig() {
    char *pjsipName, *pjMacAddress, *currentPassword;
    size_t n1 = 0, n2 = 0;
    ssize_t macAddressLen;

    printf("Name/number: ");
    pjsipName = checkNumberNotExists();

    if(useSimplePass == 1) {
	currentPassword = getCurrentOption(pjsipName, "password"); // get pass from mysql
    } else {
        do {
            printf("Password: ");
            getline(&currentPassword, &n2, stdin);
            currentPassword[strcspn(currentPassword, "\n")] = 0;
            if(strlen(currentPassword) == 0) printf("\033[0;31mPassword cannot be empty!\033[0m\n");
        } while(strlen(currentPassword) == 0);
    }

    printf("Mac-address: ");
    int flagCorrectMac = 0;
    do {
        if(flagCorrectMac == 1) printf("\033[0;31mMac-address must have 12 characters only, please enter another: \033[0m");
	getline(&pjMacAddress, &n1, stdin);
	pjMacAddress[strcspn(pjMacAddress, "\n")] = 0;

	int f = 0, i = 0;
	while(pjMacAddress[f]) {
	    pjMacAddress[f] = tolower(pjMacAddress[f]); // All upper symbols to lower
	    if(pjMacAddress[f] != ':' && pjMacAddress[f] != '-') pjMacAddress[i++] = pjMacAddress[f]; // Delete all ":" and "-" from pjMacAddress
	    ++f;
	}
	pjMacAddress[i] = '\0';

	macAddressLen = strlen(pjMacAddress);
	printf("%ld\n", macAddressLen);
        if(macAddressLen <= 11 || macAddressLen >= 13) {
	    flagCorrectMac = 1;
	} else {
	    flagCorrectMac = 0;
	}
    } while(flagCorrectMac == 1);

    char scelConfig[2048];
    sprintf(scelConfig, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"\
			"<gs_provision version=\"1\">\n"\
			"<mac>%s</mac>\n"\
			"  <config version=\"1\">\n"\
			"    <P271>1</P271>\n\n"\
			"    <P270>%s</P270>\n\n"\
			"    <P47>%s</P47>\n\n"\
			"    <P35>%s</P35>\n\n"\
			"    <P36>%s</P36>\n\n"\
			"    <P34>%s</P34>\n\n"\
			"    <P3>%s</P3>\n\n"\
			"    <P2>%s</P2>\n\n"\
			"<!--# vlan -->\n"\
			"    <P22174>%s</P22174>\n"\
			"    <P51>%s</P51>\n\n"\
			"<!--# Display language -->\n"\
			"    <P1362>%s</P1362>\n\n"\
			"<!--# TimeZone -->\n"\
			"    <P64>customize</P64>\n"\
			"    <P246>%s</P246>\n\n"\
			"<!--# NTP server -->\n"\
			"    <P30>%s</P30>\n\n"\
			"<!--# User pass -->\n"\
			"    <P196>%s</P196>\n\n"\
			"<!--# Disable SIP NOTIFY Authentication -->\n"\
			"    <P4428>1</P4428>\n"\
			"  </config>\n"\
			"</gs_provision>\n\n", pjMacAddress, pjsipName, pbxHost, pjsipName, pjsipName, currentPassword, pjsipName, \
			phoneAdminPass, enableVlan, vlan, dispLang, timeZone, ntpServer, phoneUserPass);

    char configNameStart[4] = "cfg", configNameEnd[5] = ".xml", configName[20] = "";
    strcat(configName, configNameStart);
    strcat(configName, pjMacAddress);
    strcat(configName, configNameEnd);
    strcat(checkPath, configName);

    FILE *fileStream;
    fileStream = fopen(checkPath, "r"); // Check config already exists
    int flagCheckConfig = 0;
    if(fileStream) {
	ssize_t answerCheckLen;
	size_t n0 = 0;
	char *answerCheck;
	printf("File exists. Continue?[Y/y]: ");
	answerCheckLen = getline(&answerCheck, &n0, stdin);
	answerCheck[strcspn(answerCheck, "\n")] = 0;
	if(answerCheckLen != 1 && strcmp(answerCheck, "Y") != 0 && strcmp(answerCheck, "y") != 0) {
	    printf("\033[0;33mCanceled!\033[0m\n");
	    flagCheckConfig = 1;
	}
        free(answerCheck);
	fclose(fileStream);
    }
    if(flagCheckConfig == 0) {
	fileStream = fopen(configName,"w");
	fputs(scelConfig, fileStream);
	fclose(fileStream);
	printf("Saved as %s\n", configName);
    }
    printf("\n");

    free(pjsipName);
    free(pjMacAddress);
}

void rebootRemotePhone(){
    char *ipAddress;
    size_t n0 = 0, n1 = 0;

    printf("Enter ip-address: ");
    getline(&ipAddress, &n0, stdin);
    ipAddress[strcspn(ipAddress, "\n")] = 0;
    if(strlen(ipAddress) == 0) {
	printf("No credentials? Abort.\n\n");
    } else {
	char queryReboot[1024];

	char *splitAnswer;
	do {
	    char *curlAnswer = curlFunc(ipAddress, checkAdminPass);

	    splitAnswer = strtok(curlAnswer, ",");
	    splitAnswer = strtok(splitAnswer, ":");
	    splitAnswer = strtok (NULL, ":");

            int f = 0, i = 0;
            while(splitAnswer[f]) {
		if(splitAnswer[f] != '\"') splitAnswer[i++] = splitAnswer[f]; // Delete all "
		++f;
	    }
	    splitAnswer[i] = '\0';
	    printf("Result: \033[0;33m%s\033[0m\n", splitAnswer);
	    if(strcmp(splitAnswer, "error") == 0) {
		printf("\033[0;31mPassword \033[0;33m%s\033[0;31m is wrong. Please enter another password:\033[0m ", checkAdminPass);
		getline(&checkAdminPass, &n1, stdin);
		checkAdminPass[strcspn(checkAdminPass, "\n")] = 0;
		if(strlen(checkAdminPass) == 0) {
		    printf("No credentials? Abort.\n\n");
		    break;
		}
	    }

	    free(curlAnswer);
	} while(strcmp(splitAnswer, "error") == 0);
    }
}

void truncateAnswer() {
    char *answer;
    size_t n = 0;

    printf("Truncate will destroy all data in database. Continue?(yes|no): ");
    getline(&answer, &n, stdin);
    answer[strcspn(answer, "\n")] = 0;
    if(strcmp(answer, "yes") == 0) truncatedb();
}

void showAccount() {
    printf("Name/number: ");
    char *pjsipName = checkNumberNotExists();
    showAccountFromMysql(pjsipName);
}

void checkServices() {
    checkMysqlConnect();
    FILE *curlfp;
    if ((curlfp = fopen(curlPath, "r")) == NULL) {
	printf("\033[0;33mCurl not found. Correct valid path to config or install it for option \"Reboot remote phone\"\033[0m\n");
	curlFound = 0;
    } else curlFound = 1;
    fclose(curlfp);
}

void showByContext() {
    char *pjContext;
    size_t n = 0;

    printf("Group(context)[%s]: ", defaultContext);
    getline(&pjContext, &n, stdin);
    pjContext[strcspn(pjContext, "\n")] = 0;
    if(strlen(pjContext) == 0) strcpy(pjContext, defaultContext); // If pjContext empty, set to currentContext

    showByContextMysql(pjContext);
}
