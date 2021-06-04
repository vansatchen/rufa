#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <string.h>
#include <ncursesw/ncurses.h>
#include <locale.h>
#include "functions.h"

#define QUOTE(...) #__VA_ARGS__

extern char *DB_HOST;
extern char *DB_USER;
extern char *DB_PASS;
extern char *DB_NAME;
extern char *DB_TABLE_ENDPOINTS;
extern char *DB_TABLE_AORS;
extern char *DB_TABLE_AUTHS;
extern char *DB_TABLE_CONTACTS;
extern char *defaultContext;

void finish_with_error(MYSQL *con) {
    fprintf(stderr, "%s\n", mysql_error(con));
    printf("\033[0;31mCheck connection or config\nExiting.\033[0m\n");
    mysql_close(con);
    exit(1);
}

void checkMysqlConnect() {
    MYSQL *con = mysql_init(NULL);
    if(con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }
    if(mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) finish_with_error(con);
    mysql_close(con);
}

int checkNumberExist(char *pjsipName) {
    MYSQL *con = mysql_init(NULL);
    if(con == NULL) {
	fprintf(stderr, "mysql_init() failed\n");
	exit(1);
    }
    if(mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) finish_with_error(con);
    char queryAors[1024];
    sprintf(queryAors, "SELECT id FROM %s WHERE id = '%s'", DB_TABLE_AORS, pjsipName);
    if(mysql_query(con, queryAors)) finish_with_error(con);
    MYSQL_RES *result = mysql_store_result(con);
    if(result == NULL) finish_with_error(con);
    int rows = mysql_num_rows(result);
    if(rows == 1) rows = 2;
    mysql_free_result(result);
    mysql_close(con);
    return rows;
}

void addToMysql(char *pjContext, char *pjsipName, char *pjPassword, char *pjCallerid) {
    MYSQL *con = mysql_init(NULL);
    if(con == NULL) {
	fprintf(stderr, "mysql_init() failed\n");
	exit(1);
    }
    if(mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) finish_with_error(con);
    mysql_set_character_set(con, "utf8");
    char queryAors[1024];
    sprintf(queryAors, "INSERT INTO %s (id,max_contacts) VALUES ('%s','1')", DB_TABLE_AORS, pjsipName);
    if(mysql_query(con, queryAors)) finish_with_error(con);
    char queryAuths[1024];
    sprintf(queryAuths, "INSERT INTO %s (id,auth_type,password,username) VALUES ('%s','userpass','%s','%s')", DB_TABLE_AUTHS, pjsipName, pjPassword, pjsipName);
    if(mysql_query(con, queryAuths)) finish_with_error(con);
    char queryEndpoints[1024];
    sprintf(queryEndpoints, "INSERT INTO %s (id,aors,auth,context,callerid,mailboxes) \
			     values ('%s','%s','%s','%s','%s','%s@%s')", DB_TABLE_ENDPOINTS, pjsipName, pjsipName, pjsipName, pjContext, pjCallerid, pjsipName, pjContext);
    if(mysql_query(con, queryEndpoints)) finish_with_error(con);
    mysql_close(con);
}

void delFromMysql(char *pjsipName) {
    MYSQL *con = mysql_init(NULL);
    if(con == NULL) {
	fprintf(stderr, "mysql_init() failed\n");
	exit(1);
    }
    if(mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) finish_with_error(con);
    char queryAors[1024];
    sprintf(queryAors, "DELETE FROM %s WHERE id = '%s'", DB_TABLE_AORS, pjsipName);
    if(mysql_query(con, queryAors)) finish_with_error(con);
    char queryAuths[1024];
    sprintf(queryAuths, "DELETE FROM %s WHERE id = '%s'", DB_TABLE_AUTHS, pjsipName);
    if(mysql_query(con, queryAuths)) finish_with_error(con);
    char queryEndpoints[1024];
    sprintf(queryEndpoints, "DELETE FROM %s WHERE id = '%s'", DB_TABLE_ENDPOINTS, pjsipName);
    if(mysql_query(con, queryEndpoints)) finish_with_error(con);
    mysql_close(con);
}

void showByOption(char *option) {
    MYSQL *con = mysql_init(NULL);
    if(con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }
    if(mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) finish_with_error(con);
    mysql_set_character_set(con, "utf8");
    char queryShow[1024];
/*    sprintf(queryShow, "mysql --login-path=asterisk -u " DB_USER " -p" DB_PASS " -h " DB_HOST " -D " DB_NAME \
                        " -e \"SELECT " DB_TABLE_ENDPOINTS ".context," DB_TABLE_AUTHS ".username," DB_TABLE_AUTHS ".password," \
                        DB_TABLE_ENDPOINTS ".callerid," DB_TABLE_CONTACTS ".user_agent," DB_TABLE_CONTACTS ".via_addr FROM " \
                        DB_TABLE_AUTHS " INNER JOIN " DB_TABLE_ENDPOINTS " USING(id) LEFT JOIN " DB_TABLE_CONTACTS " ON " \
                        DB_TABLE_AUTHS ".username = " DB_TABLE_CONTACTS ".endpoint ORDER BY %s;\" 2>/dev/null", option);*/
    sprintf(queryShow, "mysql --login-path=asterisk -u %s -p%s -h %s -D %s -e \
			\"SELECT %s.context,%s.username,%s.callerid,%s.via_addr FROM %s INNER JOIN %s USING(id) \
			LEFT JOIN %s ON %s.username = %s.endpoint ORDER BY %s;\" 2>/dev/null", \
			DB_USER, DB_PASS, DB_HOST, DB_NAME, DB_TABLE_ENDPOINTS, DB_TABLE_AUTHS, DB_TABLE_ENDPOINTS, \
			DB_TABLE_CONTACTS, DB_TABLE_AUTHS, DB_TABLE_ENDPOINTS, DB_TABLE_CONTACTS, DB_TABLE_AUTHS, DB_TABLE_CONTACTS, option);
    system(queryShow);
    mysql_close(con);
    return;
}

char *getCurrentOption(char *pjsipName, char *option) {
    MYSQL *con = mysql_init(NULL);
    if(con == NULL) {
	fprintf(stderr, "mysql_init() failed\n");
	exit(1);
    }
    if(mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) finish_with_error(con);
    mysql_set_character_set(con, "utf8");
    mysql_query(con, "SET NAMES utf8");
    char queryAors[1024];
    if(strcmp(option, "context") == 0) sprintf(queryAors, "SELECT context FROM %s WHERE id = '%s'", DB_TABLE_ENDPOINTS, pjsipName);
    if(strcmp(option, "password") == 0) sprintf(queryAors, "SELECT password FROM %s WHERE id = '%s'", DB_TABLE_AUTHS, pjsipName);
    if(strcmp(option, "callerid") == 0) sprintf(queryAors, "SELECT callerid FROM %s WHERE id = '%s'", DB_TABLE_ENDPOINTS, pjsipName);
    if(strcmp(option, "maxcontacts") == 0) sprintf(queryAors, "SELECT max_contacts FROM %s WHERE id = '%s'", DB_TABLE_AORS, pjsipName);
    if(strcmp(option, "transport") == 0) sprintf(queryAors, "SELECT transport FROM %s WHERE id = '%s'", DB_TABLE_ENDPOINTS, pjsipName);
    if(strcmp(option, "mediaencryption") == 0) sprintf(queryAors, "SELECT media_encryption FROM %s WHERE id = '%s'", DB_TABLE_ENDPOINTS, pjsipName);
    if(strcmp(option, "codecs") == 0) sprintf(queryAors, "SELECT allow FROM %s WHERE id = '%s'", DB_TABLE_ENDPOINTS, pjsipName);
    if(mysql_query(con, queryAors)) finish_with_error(con);
    MYSQL_RES *result = mysql_store_result(con);
    if(result == NULL) finish_with_error(con);
    MYSQL_ROW row;
    row = mysql_fetch_row(result);
    mysql_free_result(result);
    mysql_close(con);
    return row[0];
}

void updateBaseToMysql(char *pjContext, char *pjsipName, char *pjNewName, char *pjPassword, char *pjCallerid) {
    MYSQL *con = mysql_init(NULL);
    if(con == NULL) {
	fprintf(stderr, "mysql_init() failed\n");
	exit(1);
    }
    if(mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) finish_with_error(con);
    mysql_set_character_set(con, "utf8");
    char queryAors[1024];
    sprintf(queryAors, "UPDATE %s SET id = '%s' WHERE id = '%s'", DB_TABLE_AORS, pjNewName, pjsipName);
    if(mysql_query(con, queryAors)) finish_with_error(con);
    char queryAuths[1024];
    sprintf(queryAuths, "UPDATE %s SET id = '%s',username = '%s',password = '%s' WHERE id = '%s'", DB_TABLE_AUTHS, pjNewName, pjNewName, pjPassword, pjsipName);
    if(mysql_query(con, queryAuths)) finish_with_error(con);
    char queryEndpoints[1024];
    sprintf(queryEndpoints, "UPDATE %s SET context = '%s',mailboxes = '%s@%s',id = '%s',aors = '%s',auth = '%s',callerid = '%s' \
			     WHERE id = '%s'", DB_TABLE_ENDPOINTS, pjContext, pjNewName, pjContext, pjNewName, pjNewName, pjNewName, pjCallerid, pjsipName);
    if(mysql_query(con, queryEndpoints)) finish_with_error(con);
    mysql_close(con);
}

const char *truncate_query = QUOTE(
    TRUNCATE TABLE `cdr`;
    TRUNCATE TABLE `ps_aors`;
    TRUNCATE TABLE `ps_auths`;
    TRUNCATE TABLE `ps_contact_status`;
    TRUNCATE TABLE `ps_contacts`;
    TRUNCATE TABLE `ps_contacts`;
    TRUNCATE TABLE `ps_domain_aliases`;
    TRUNCATE TABLE `ps_endpoint_id_ips`;
    TRUNCATE TABLE `ps_endpoints`;
    TRUNCATE TABLE `ps_globals`;
    TRUNCATE TABLE `ps_registrations`;
    TRUNCATE TABLE `ps_subscription_persistence`;
    TRUNCATE TABLE `ps_systems`;
    TRUNCATE TABLE `ps_transports`;
);

void truncatedb() {
    MYSQL *con = mysql_init(NULL);
    if(con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }
    if(mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, CLIENT_MULTI_STATEMENTS) == NULL) finish_with_error(con);
    if(mysql_query(con, truncate_query)) finish_with_error(con);
    mysql_close(con);
}

void showAccountFromMysql(char *pjsipName) {
    MYSQL *con = mysql_init(NULL);
    if(con == NULL) {
	fprintf(stderr, "mysql_init() failed\n");
	exit(1);
    }
    if(mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) finish_with_error(con);
    mysql_set_character_set(con, "utf8");
    char queryShowAccount[1024];
    sprintf(queryShowAccount, "SELECT %s.context,%s.username,%s.password,%s.callerid,%s.user_agent,%s.via_addr,\
			%s.max_contacts,%s.transport,%s.allow,%s.media_encryption FROM %s INNER JOIN %s USING(id) \
			LEFT JOIN %s ON %s.username = %s.endpoint INNER JOIN %s ON %s.id = %s.username WHERE username = '%s'", \
			DB_TABLE_ENDPOINTS, DB_TABLE_AUTHS, DB_TABLE_AUTHS, DB_TABLE_ENDPOINTS, DB_TABLE_CONTACTS, \
			DB_TABLE_CONTACTS, DB_TABLE_AORS, DB_TABLE_ENDPOINTS,DB_TABLE_ENDPOINTS, DB_TABLE_ENDPOINTS, \
			DB_TABLE_AUTHS, DB_TABLE_ENDPOINTS, DB_TABLE_CONTACTS, DB_TABLE_AUTHS, DB_TABLE_CONTACTS, \
			DB_TABLE_AORS, DB_TABLE_AORS, DB_TABLE_AUTHS, pjsipName);
    if(mysql_query(con, queryShowAccount)) {
	finish_with_error(con);
    }
    MYSQL_RES *result = mysql_store_result(con);
    if(result == NULL) finish_with_error(con);
    int num_fields = mysql_num_fields(result); // 10
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result))) {
	for(int i = 0; i < num_fields / 10; i++) {
	    printf("------------\n");
	    printf("\033[0;33mContext:\033[0m %s\n\033[0;33mNumber:\033[0m %s\n"\
		"\033[0;33mPassword:\033[0m %s\n\033[0;33mCallerID:\033[0m %s\n"\
		"\033[0;33mUser agent:\033[0m %s\n\033[0;33mAddress:\033[0m %s\n"\
		"\033[0;33mMax contacts:\033[0m %s\n\033[0;33mTransport:\033[0m %s\n"\
		"\033[0;33mCodecs:\033[0m %s\n\033[0;33mMedia encryption:\033[0m %s\n",
		row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7], row[8], row[9]);
	    printf("------------\n");
	}
    }

    mysql_free_result(result);
    mysql_close(con);
}

void showByContextMysql(char *context) {
    MYSQL *con = mysql_init(NULL);
    if(con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }
    if(mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) finish_with_error(con);
    printf("Using context \033[0;33m%s\033[0m\n\n", context);
    mysql_set_character_set(con, "utf8");
    char queryShow[1024];
    sprintf(queryShow, "mysql --login-path=asterisk -u %s -p%s -h %s -D %s -e \
                        \"SELECT %s.context,%s.username,%s.callerid,%s.via_addr FROM %s INNER JOIN %s USING(id) \
                        LEFT JOIN %s ON %s.username = %s.endpoint WHERE %s.context = '%s' ORDER BY username;\" 2>/dev/null", \
                        DB_USER, DB_PASS, DB_HOST, DB_NAME, DB_TABLE_ENDPOINTS, DB_TABLE_AUTHS, DB_TABLE_ENDPOINTS, \
                        DB_TABLE_CONTACTS, DB_TABLE_AUTHS, DB_TABLE_ENDPOINTS, DB_TABLE_CONTACTS, DB_TABLE_AUTHS, DB_TABLE_CONTACTS, DB_TABLE_ENDPOINTS, context);
    system(queryShow);
    mysql_close(con);
    return;
}

char *getAvailContexts() {
    char context[1024], field[16356];

    MYSQL *con = mysql_init(NULL);
    if(con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }
    if(mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) finish_with_error(con);
    mysql_set_character_set(con, "utf8");
    mysql_query(con, "SET NAMES utf8");
    char querySelect[1024];
    sprintf(querySelect, "SELECT context FROM %s ORDER BY context", DB_TABLE_ENDPOINTS);
    if(mysql_query(con, querySelect)) finish_with_error(con);

    MYSQL_RES *result = mysql_store_result(con);
    if(result == NULL) finish_with_error(con);

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
	if(strcmp(field, row[0])) {
	    strcpy(field, row[0]);
	    strcat(context, row[0]);
	    strcat(context, " ");
	}
    }

    char *availContexts = malloc(strlen(context) + 1);
    strcpy(availContexts, context);

    mysql_free_result(result);
    mysql_close(con);
    strcpy(context, ""); // Clear context variable

    return availContexts;
}

void showForWins(WINDOW *contextWin, WINDOW *numberWin, WINDOW *calleridWin, WINDOW *addressWin) {
    MYSQL *con = mysql_init(NULL);
    if(con == NULL) fprintf(stderr, "mysql_init() failed\n");
    if(mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) finish_with_error(con);
    mysql_set_character_set(con, "utf8");
    mysql_query(con, "SET NAMES utf8");
    char queryShow[1024];
    sprintf(queryShow, "SELECT %s.context,%s.username,%s.callerid,%s.via_addr FROM %s INNER JOIN %s USING(id) \
                        LEFT JOIN %s ON %s.username = %s.endpoint ORDER BY %s", DB_TABLE_ENDPOINTS, DB_TABLE_AUTHS, DB_TABLE_ENDPOINTS, \
                        DB_TABLE_CONTACTS, DB_TABLE_AUTHS, DB_TABLE_ENDPOINTS, DB_TABLE_CONTACTS, DB_TABLE_AUTHS, DB_TABLE_CONTACTS, "username");
    if(mysql_query(con, queryShow)) finish_with_error(con);

    MYSQL_RES *result = mysql_store_result(con);
    if(result == NULL) finish_with_error(con);

    MYSQL_ROW row;
    int countRow = 0;
    setlocale(0, "");
    wchar_t* temp = calloc(sizeof(wchar_t), 1000);
    wattron(contextWin, A_BOLD);
    wattron(numberWin, A_BOLD);
    wattron(calleridWin, A_BOLD);
    wattron(addressWin, A_BOLD);
    while((row = mysql_fetch_row(result))) {
        for(int i = 0; i < 4; i++) {
            swprintf(temp, 100, L"%s", row[i]);
            if(i == 0) mvwaddwstr(contextWin, countRow + 1, 2, temp);
            if(i == 1) mvwaddwstr(numberWin, countRow + 1, 2, temp);
            if(i == 2) mvwaddwstr(calleridWin, countRow + 1, 2, temp);
            if(i == 3) mvwaddwstr(addressWin, countRow + 1, 2, temp);
        }
        countRow++;
    }
    wattroff(contextWin, A_BOLD);
    wattroff(numberWin, A_BOLD);
    wattroff(calleridWin, A_BOLD);
    wattroff(addressWin, A_BOLD);

    free(temp);
    mysql_free_result(result);
    mysql_close(con);
}
