#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/errno.h>
#include <stdlib.h>

#define ARRAY_SIZE(a) ((sizeof (a)) / (sizeof (a)[0]))

enum { MAX_LEN=128 };

char *rufaVersion = "0.1";
//----- Defaults -----
// Mysql defaults
char *DB_HOST = "localhost";
char *DB_USER = "asteriskuser";
char *DB_PASS = "asteriskpass";
char *DB_NAME = "asteriskdb";
char *DB_TABLE_ENDPOINTS = "ps_endpoints";
char *DB_TABLE_AORS      = "ps_aors";
char *DB_TABLE_AUTHS     = "ps_auths";
char *DB_TABLE_CONTACTS  = "ps_contacts";
// Defaults
char *defaultContext = "somecontext"; // Context
int useSimplePass = 0; // Flag of using simple pass
char defaultPass[10] = "pass"; // Get password from pjsipName + defaultPass
int numberLen = 4; // Length of number
// Defaults for phone configs
char *pbxHost = "pbx.example.com"; // PBX-server for config
char *phoneAdminPass = "admin"; // Admin password for WUI phone for config
char *enableVlan = "0"; // Enable vlan for config
char *vlan = "0"; // Vlan for config
char *dispLang = "ru"; // Display language for config
char *timeZone = "MTZ-5MDT-5,M4.1.0,M11.1.0"; // Time zone for config
char *ntpServer = "pool.ntp.org"; // NTP server for config
char *phoneUserPass = "user123"; // User password for WUI phone for config
char checkPath[100] = "/configs/"; // Path to check exists configs
char *checkAdminPass = "admin1"; // Check current admin pass
// Other defaults
char *curlPath = "/usr/bin/curl"; // Path to curl

struct param {
    char name[MAX_LEN];
    char value[MAX_LEN];
};

void strtrim(char *s) {
    char *p = s + strlen(s);
    while (--p >= s && isspace(*p)) *p = '\0';
}

int read_params(FILE *in, struct param *p, int max_params) {
    int ln, n=0;
    char s[MAX_LEN];

    for (ln=1; max_params > 0 && fgets(s, MAX_LEN, in); ln++) {
	if (sscanf(s, " %[#\n\r]", p->name)) continue; // emty line or comment
	if (sscanf(s, " %[a-z_A-Z0-9] = %[^#\n\r]", p->name, p->value) < 2) {
	    fprintf(stderr, "Config parse error!\nError at line %d: %s\n", ln, s);
	    return -1;
	}
	strtrim(p->value);
//	printf("name='%s' value='%s'\n", p->name, p->value);

	if(strcmp(p->name, "dbhost") == 0) DB_HOST = p->value;
	if(strcmp(p->name, "dbuser") == 0) DB_USER = p->value;
	if(strcmp(p->name, "dbpass") == 0) DB_PASS = p->value;
	if(strcmp(p->name, "dbname") == 0) DB_NAME = p->value;
	if(strcmp(p->name, "dbtableendpoints") == 0) DB_TABLE_ENDPOINTS = p->value;
	if(strcmp(p->name, "dbtableaors") == 0) DB_TABLE_AORS = p->value;
	if(strcmp(p->name, "dbtableauths") == 0) DB_TABLE_AUTHS = p->value;
	if(strcmp(p->name, "dbtablecontacts") == 0) DB_TABLE_CONTACTS = p->value;

	if(strcmp(p->name, "defaultcontext") == 0) defaultContext = p->value;
	if(strcmp(p->name, "defaultpass") == 0) {
	    if(strcmp(p->value, "0") == 0) { // If defaultpass = 0, dont use simple pass
		useSimplePass = 0;
	    } else {
		strcpy(defaultPass, p->value);
		useSimplePass = 1;
	    }
	}
	if(strcmp(p->name, "numberlength") == 0) {
	    if(strcmp(p->value, "0") == 0 || strcmp(p->value, "1") == 0) sprintf(p->value, "%d", numberLen);
	}

	if(strcmp(p->name, "phonepbxserver") == 0) pbxHost = p->value;
	if(strcmp(p->name, "phoneadminpass") == 0) phoneAdminPass = p->value;
	if(strcmp(p->name, "phoneenablevlan") == 0) enableVlan = p->value;
	if(strcmp(p->name, "phonevlan") == 0) vlan = p->value;
	if(strcmp(p->name, "phonedisplang") == 0) dispLang = p->value;
	if(strcmp(p->name, "phonetimezone") == 0) timeZone = p->value;
	if(strcmp(p->name, "phonentpserver") == 0) ntpServer = p->value;
	if(strcmp(p->name, "phoneuserpass") == 0) phoneUserPass = p->value;
	if(strcmp(p->name, "phonecheckpath") == 0) strcpy(checkPath, p->value);
	if(strcmp(p->name, "phonecheckadminpass") == 0) checkAdminPass = p->value;

	if(strcmp(p->name, "curlpath") == 0) curlPath = p->value;

	p++, max_params--, n++;
    }
/*    printf("=== Finish variables from config file ===\n\n");
    printf("=== Current settings ===\n");
    printf("DB_HOST: %s\nDB_USER: %s\nDB_PASS: %s\nDB_NAME: %s\nDB_TABLE_ENDPOINTS: %s\nDB_TABLE_AORS: %s\nDB_TABLE_AUTHS: %s\nDB_TABLE_CONTACTS: %s\n"\
	"defaultContext: %s\nuseSimplePass: %d\ndefaultPass: %s\nnumberLen: %d\npbxHost: %s\nphoneAdminPass: %s\nenableVlan: %s\nvlan: %s\n"\
	"dispLang: %s\ntimeZone: %s\nntpServer: %s\nphoneUserPass: %s\ncheckPath: %s\ncheckAdminPass: %s\n\n", \
	DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_TABLE_ENDPOINTS, DB_TABLE_AORS, DB_TABLE_AUTHS, DB_TABLE_CONTACTS, \
	defaultContext, useSimplePass, defaultPass, numberLen, pbxHost, phoneAdminPass, enableVlan, vlan, dispLang, timeZone, \
	ntpServer, phoneUserPass, checkPath, checkAdminPass);*/

    return n;
}

int readConfig() {
    FILE *f;
    struct param p[32];

    char *homePath = getenv("HOME"), *rufarc = "/.rufarc"; // Config file may be in home directory as .rufarc
    char* buff = calloc(strlen(homePath) + strlen(rufarc) + 1, 1);
    strcat(buff, homePath);
    strcat(buff, rufarc);

    char *configPaths[] = {"rufa.conf", buff, "/etc/rufa.conf"}; // Search config file at current directory
    int c = 0;							 // as rufa.conf, home directory as .rufarc,
    for(c; c < 3; c++) {					 // and as /etc/rufa.conf
	f = fopen(configPaths[c], "r");
	if (f != NULL) break;
    }
    if (f == NULL) return 1;
    if (read_params(f, p, ARRAY_SIZE(p)) < 0) return 1;

    free(buff);
    return 0;
}
