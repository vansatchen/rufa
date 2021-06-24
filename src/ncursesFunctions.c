#define _GNU_SOURCE
#include <ncursesw/ncurses.h>
#include <ncursesw/form.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mysql.h>
#include "functions.h"

extern MYSQL_RES *result;
int rows, col;

FIELD *field[7], *cfield;
FORM  *addForm;
WINDOW *titlebar, *menubar, *mainWin, *contextWin, *numberWin, *calleridWin, *addressWin, *addPopup;

void draw_menubar(WINDOW *menubar);
void draw_titlebar();
void popupKey();
void helpMenu();
void addMenu();
static void driver();
void delPopup();
void show4wins();

int wins() {
    // Start ncurses finctionality
    initscr();
    start_color();
    init_pair(1, COLOR_WHITE,COLOR_BLUE);
    init_pair(2, COLOR_BLACK,COLOR_CYAN);
    init_pair(3, COLOR_BLACK,COLOR_WHITE);
    init_pair(4, COLOR_WHITE,COLOR_BLACK); // Shadow
    curs_set(0);
    noecho();
    keypad(stdscr, TRUE);
    // Screen size & background
    getmaxyx(stdscr, rows, col);
//    mvwprintw(stdscr, 0, col - 20, "Rows: %d, Col: %d", rows, col);
    // Panel & main window
    titlebar = subwin(stdscr, 1, col, 0, 0);
    menubar = subwin(stdscr, 1, col, rows - 2, 0);
    mainWin = subwin(stdscr, rows - 3, col, 1, 0);
    draw_menubar(menubar);
    draw_titlebar(titlebar, col);
    wbkgd(mainWin, COLOR_PAIR(1));
    // Columns and boards
    contextWin = derwin(mainWin, 0, 20, 0, 0);
    numberWin = derwin(mainWin, 0, 12, 0, 19);
    calleridWin = derwin(mainWin, 0, col - 50, 0, 30);
    addressWin = derwin(mainWin, 0, 20, 0, col - 20);
    box(contextWin, 0, 0);
    box(numberWin, 0, 0);
    box(calleridWin, 0, 0);
    wattron(mainWin, COLOR_PAIR(2));
    box(mainWin, 0, 0);
    wattroff(mainWin, COLOR_PAIR(2));
    mvwprintw(contextWin, 0, 2, " Context ");
    mvwprintw(numberWin, 0, 2, " Number ");
    mvwprintw(calleridWin, 0, 2, " CallerID ");
    mvwprintw(addressWin, 0, 2, " IP address ");

    // Fix view lines on only inside boxes
    contextWin = derwin(contextWin, rows - 4, 0, 0, 0);
    numberWin = derwin(numberWin, rows - 4, 0, 0, 0);
    calleridWin = derwin(calleridWin, rows - 4, 0, 0, 0);
    addressWin = derwin(addressWin, rows - 4, 0, 0, 0);

    show4wins(rows);
//    refresh();

    wchar_t key;
    do {
	get_wch(&key);
//	popupKey(key, rows, col);
	if(key == KEY_F(1)) helpMenu(rows, col); // Help popup
	if(key == KEY_F(2)) addMenu(rows, col); // Add popup
	if(key == KEY_F(3)) delPopup(rows, col); // Delete popup
	mvwprintw(stdscr, rows - 1, col - 15, "Selected: %d", key);
    } while(key != KEY_F(10));

    delwin(menubar);
    delwin(contextWin);
    delwin(numberWin);
    delwin(calleridWin);
    delwin(addressWin);
    delwin(mainWin);
    endwin();
    printf("Bye!\n");
    return 0;
}

void show4wins(int rows) {
    get4wins();
    MYSQL_ROW row;
    int countRow = 0;
    setlocale(0, "");
    wchar_t* temp = calloc(sizeof(wchar_t), 1000);
    wattron(contextWin, A_BOLD);
    wattron(numberWin, A_BOLD);
    wattron(calleridWin, A_BOLD);
    wattron(addressWin, A_BOLD);
    int num_rows = mysql_num_rows(result);
    char *numberVar[num_rows];
    while((row = mysql_fetch_row(result))) {
        for(int i = 0; i < 4; i++) {
            swprintf(temp, 100, L"%s", row[i]);
            if(i == 0) {
                mvwaddwstr(contextWin, countRow + 1, 2, temp);
            }
            if(i == 1) {
                mvwaddwstr(numberWin, countRow + 1, 2, temp);
                numberVar[countRow] = row[1];
            }
            if(i == 2) {
                mvwaddwstr(calleridWin, countRow + 1, 2, temp);
            }
            if(i == 3) {
                mvwaddwstr(addressWin, countRow + 1, 2, temp);
            }
        }
        countRow++;
    }
    wattroff(contextWin, A_BOLD);
    wattroff(numberWin, A_BOLD);
    wattroff(calleridWin, A_BOLD);
    wattroff(addressWin, A_BOLD);
    mvwprintw(stdscr, rows - 1, 1, "Accounts: %d\tNumber5: %s", num_rows, numberVar[5 - 1]);

    free(temp);
    refresh();
}

void helpMenu(int rows, int col) {
    int helpY = 13, helpX = 65; // Y - rows, X - cols
    WINDOW *helpPopup = subwin(mainWin, helpY, helpX, (rows - 3) / 2, (col - helpX) / 2);
//    wbkgd(helpPopup, COLOR_PAIR(3));
    wattron(helpPopup, COLOR_PAIR(3));
    box(helpPopup, 0, 0);
    // Help menu content
    mvwaddwstr(helpPopup, 0, (helpX - 6) / 2, L" Help ");
    mvwaddwstr(helpPopup, 1, 1, L" Realtime users for asterisk\t\t\t\t\t");
    mvwaddwstr(helpPopup, 2, 1, L" Usage: rufa [OPTION]\t\t\t\t\t\t");
    mvwaddwstr(helpPopup, 3, 1, L" Options:\t\t\t\t\t\t\t");
    mvwaddwstr(helpPopup, 4, 1, L"\t--help          display this help and exit\t\t");
    mvwaddwstr(helpPopup, 5, 1, L"\t--createdb      Create database and exit\t\t");
    mvwaddwstr(helpPopup, 6, 1, L"\t--truncatedb    Truncate database and exit\t\t");
    mvwaddwstr(helpPopup, 7, 1, L"\t--version       Show version and exit\t\t\t");
    mvwaddwstr(helpPopup, 8, 1, L"\t\t\t\t\t\t\t\t");
    mvwaddwstr(helpPopup, 9, 1, L"\tsomecontext     Use 'somecontext' as default context\t");
    mvwaddwstr(helpPopup, 10, 1, L"\t\t\t\t\t\t\t\t");
    mvwaddwstr(helpPopup, 11, 1, L"\t\t\t\t\t\t    [F10]Close  ");
    wrefresh(helpPopup);

    int closeHelpWin;
    do {
        get_wch(&closeHelpWin);
        if(closeHelpWin == KEY_F(10)) break;
    } while(1);
    wattroff(helpPopup, COLOR_PAIR(3));
    werase(helpPopup);
    delwin(helpPopup);
    touchwin(stdscr);
}

void addMenu(int rows, int col) {
    setlocale(0, "");
    curs_set(2);
    int helpY = 12, helpX = 65; // Y - rows, X - cols
    addPopup = subwin(mainWin, helpY, helpX, (rows - 3) / 2, (col - helpX) / 2);
    wattron(addPopup, COLOR_PAIR(3));
    box(addPopup, 0, 0);
    mvwprintw(addPopup, 0, (helpX - 5) / 2, " Add ");
    wrefresh(addPopup);

    // context, number, password, callerid
    field[0] = new_field(1, 20, 1, 10, 0, 0); // context
    field[1] = new_field(1, 20, 3, 10, 0, 0); // number
    field[2] = new_field(1, 20, 5, 10, 0, 0); // password
    field[3] = new_field(1, 40, 7, 10, 0, 0); // callerid
    field[4] = new_field(1, 10, 9, 40, 0, 0);  // cancel
    field[5] = new_field(1, 7, 9, 55, 0, 0);  // apply
    field[6] = NULL;

    set_field_fore(field[0], COLOR_PAIR(2));
    set_field_back(field[0], COLOR_PAIR(2));
//    set_field_type(field[0], TYPE_ALNUM, 40);
    set_field_fore(field[1], COLOR_PAIR(2));
    set_field_back(field[1], COLOR_PAIR(2));
    set_field_type(field[1], TYPE_NUMERIC, 0, 4, 4); // TYPE_INTEGER, TYPE_NUMERIC, TYPE_REGEXP       TYPE_IPV4
    set_field_fore(field[2], COLOR_PAIR(2));
    set_field_back(field[2], COLOR_PAIR(2));
    set_field_opts(field[2], O_VISIBLE);
    set_field_type(field[2], TYPE_ALNUM, 40);
    set_field_fore(field[3], COLOR_PAIR(2));
    set_field_back(field[3], COLOR_PAIR(2));
    field_opts_off(field[3], O_STATIC);
    set_max_field(field[3], 80);
    set_field_fore(field[4], COLOR_PAIR(2));
    set_field_back(field[4], COLOR_PAIR(2));
//    field_opts_off(field[4], O_EDIT);
    set_field_opts(field[4], O_VISIBLE);
    set_field_fore(field[5], COLOR_PAIR(2));
    set_field_back(field[5], COLOR_PAIR(2));
    field_opts_off(field[5], O_EDIT);

    addForm = new_form(field);
    set_form_win(addForm, addPopup);
    set_form_sub(addForm, derwin(addPopup, 10, 63, 1, 1));
    post_form(addForm);

    set_current_field(addForm, field[0]);
    mvwaddwstr(addPopup, 1, 1, L" \t\t\t\t\t\t\t\t");
    mvwaddwstr(addPopup, 2, 1, L"  Context:");
    mvwaddwstr(addPopup, 2, 31, L" \t\t\t\t");
    mvwaddwstr(addPopup, 3, 1, L" \t\t\t\t\t\t\t\t");
    mvwaddwstr(addPopup, 4, 1, L"   Number:");
    mvwaddwstr(addPopup, 4, 31, L" \t\t\t\t");
    mvwaddwstr(addPopup, 5, 1, L" \t\t\t\t\t\t\t\t");
    mvwaddwstr(addPopup, 6, 1, L" Password:");
    mvwaddwstr(addPopup, 6, 31, L" \t\t\t\t");
    mvwaddwstr(addPopup, 7, 1, L" \t\t\t\t\t\t\t\t");
    mvwaddwstr(addPopup, 8, 1, L" CallerID:");
    mvwaddwstr(addPopup, 8, 51, L" \t\t");
    mvwaddwstr(addPopup, 9, 1, L" \t\t\t\t\t\t\t\t");
    mvwaddwstr(addPopup, 10, 1, L" \t\t\t\t\t\t");
    mvwaddwstr(addPopup, 10, 39, L"[F10]Cancel \t");
    mvwaddwstr(addPopup, 10, 54, L"[ Apply ] ");

    wrefresh(addPopup);
    int ch;
    do {
	get_wch(&ch);
	if(ch == KEY_F(10)) break;
        driver(ch);
        if(ch == '\n') {
            cfield = current_field(addForm);
            if(cfield == field[5]) {
                driver(KEY_F(11));
                break;
            }
        }
    } while(1);

    unpost_form(addForm);
    free_form(addForm);
    free_field(field[0]);
    free_field(field[1]);
    free_field(field[2]);
    free_field(field[3]);
    free_field(field[4]);
    free_field(field[5]);

    curs_set(0);
    wattroff(addPopup, COLOR_PAIR(3));
    werase(addPopup);
    delwin(addPopup);
    touchwin(stdscr);
}

void delPopup(int rows, int col) {
    int helpY = 7, helpX = 30; // Y - rows, X - cols
    WINDOW *delPopup = subwin(mainWin, helpY, helpX, (rows - 3) / 2, (col - helpX) / 2);
    wattron(delPopup, COLOR_PAIR(3));
    box(delPopup, 0, 0);
    wrefresh(delPopup);

    int ch;
    mvwaddwstr(delPopup, 0, (helpX - 8) / 2, L" Delete ");
    mvwaddwstr(delPopup, 1, 1, L" \t\t\t     ");
    mvwaddwstr(delPopup, 2, 1, L" \tAre you shure?\t     ");
    mvwaddwstr(delPopup, 3, 1, L" \t\t\t     ");
    mvwaddwstr(delPopup, 4, 1, L" [F10]Cancel\t");
    mvwaddwstr(delPopup, 4, 16, L"[Enter]Apply ");
    mvwaddwstr(delPopup, 5, 1, L" \t\t\t     ");

    wrefresh(delPopup);
    while((ch = getch()) != KEY_F(10)) {
        if(ch == '\n') {
            driver(KEY_F(11));
            break;
        }
    }

    wattroff(delPopup, COLOR_PAIR(3));
    werase(delPopup);
    delwin(delPopup);
    touchwin(stdscr);
}

// This is useful because ncurses fill fields blanks with spaces.
static char* trim_whitespaces(char *str) {
    // trim leading space
    while(isspace(*str)) str++;
    if(*str == 0) return str; // all spaces?

    // trim trailing space
    char *end = str + strnlen(str, 128) - 1;
    while(end > str && isspace(*end)) end--;

    // write new null terminator
    *(end+1) = '\0';

    return str;
}

static void driver(int ch) {
    switch(ch) {
	case KEY_F(11):
	    // Or the current field buffer won't be sync with what is displayed
	    form_driver(addForm, REQ_NEXT_FIELD);
	    form_driver(addForm, REQ_PREV_FIELD);
	    move(LINES - 1, 2);
//	    mvwprintw(stdscr, rows - 1, 2, "Selected: %d");

	    for(int i = 0; field[i]; i++) {
		printw("\"%s", trim_whitespaces(field_buffer(field[i], 0)));
		if(field_opts(field[i]) & O_ACTIVE) printw("\"\t");
		else printw(": \"");
	    }
	return;

	refresh();
	pos_form_cursor(addForm);
	break;
	case '\t':
	    form_driver(addForm, REQ_NEXT_FIELD);
	    form_driver(addForm, REQ_END_LINE);
	    break;
	case '\n':
	    cfield = current_field(addForm);
	    if(cfield != field[5]) {
		form_driver(addForm, REQ_NEXT_FIELD);
		form_driver(addForm, REQ_END_LINE);
	    }
	    break;
	case KEY_DOWN:
	    form_driver(addForm, REQ_NEXT_FIELD);
	    form_driver(addForm, REQ_END_LINE);
	    break;
	case KEY_UP:
	    form_driver(addForm, REQ_PREV_FIELD);
	    form_driver(addForm, REQ_END_LINE);
	    break;
	case KEY_LEFT:
	    cfield = current_field(addForm);
	    if(cfield != field[4] && cfield != field[5]) form_driver(addForm, REQ_PREV_CHAR);
	    else {
		form_driver(addForm, REQ_PREV_FIELD);
		if(cfield == field[4]) form_driver(addForm, REQ_END_LINE);
	    }
	    break;
	case KEY_RIGHT:
	    cfield = current_field(addForm);
	    if(cfield != field[4] && cfield != field[5]) form_driver(addForm, REQ_NEXT_CHAR);
	    else {
		form_driver(addForm, REQ_NEXT_FIELD);
		if(cfield == field[5]) form_driver(addForm, REQ_END_LINE);
	    }
	    break;
	case KEY_BACKSPACE: // Delete the char before cursor
	case 127:
	    form_driver(addForm, REQ_DEL_PREV);
	    break;
	case KEY_DC: // Delete the char under the cursor
	    form_driver(addForm, REQ_DEL_CHAR);
	    break;
	default:
	    form_driver(addForm, ch);
	    break;
    }
    wrefresh(addPopup);
}

void popupKey(int key, int rows, int col) {
    WINDOW *popup = subwin(stdscr, 3, 17, (rows - 3) / 2, (col - 17) / 2);
    wattron(popup, COLOR_PAIR(3));
    box(popup, 0, 0);
    if(key == 273) { // F9
	wattroff(popup, COLOR_PAIR(3));
	werase(popup);
	delwin(popup);
	return;
    }
    mvwprintw(popup, 1, 1, "Selected: \"%d\"", key);
    wrefresh(popup);
}

void draw_menubar(WINDOW *menubar) {
    wbkgd(menubar, COLOR_PAIR(2));
    wattron(menubar, COLOR_PAIR(2) | A_BOLD);
    waddstr(menubar, "[F1]");
//    wprintw(menubar, "%s%s%s", "[F", "1", "]");
    wattroff(menubar, COLOR_PAIR(2) | A_BOLD);
    wattron(menubar, COLOR_PAIR(2));
    waddstr(menubar, "Help");
    wattroff(menubar, COLOR_PAIR(2));
    wmove(menubar, 0, 12);
    wattron(menubar, COLOR_PAIR(2) | A_BOLD);
    waddstr(menubar, "[F2]");
    wattroff(menubar, COLOR_PAIR(2) | A_BOLD);
    wattron(menubar, COLOR_PAIR(2));
    waddstr(menubar, "Add");
    wattroff(menubar, COLOR_PAIR(2));
    wmove(menubar, 0, 22);
    wattron(menubar, COLOR_PAIR(2) | A_BOLD);
    waddstr(menubar, "[F3]");
    wattroff(menubar, COLOR_PAIR(2) | A_BOLD);
    wattron(menubar, COLOR_PAIR(2));
    waddstr(menubar, "Delete");
    wattroff(menubar, COLOR_PAIR(2));
    wmove(menubar, 0, 35);
    wattron(menubar, COLOR_PAIR(2) | A_BOLD);
    waddstr(menubar, "[F4]");
    wattroff(menubar, COLOR_PAIR(2) | A_BOLD);
    wattron(menubar, COLOR_PAIR(2));
    waddstr(menubar, "Show");
    wattroff(menubar, COLOR_PAIR(2));
    wmove(menubar, 0, 45);
    wattron(menubar, COLOR_PAIR(2) | A_BOLD);
    waddstr(menubar, "[F5]");
    wattroff(menubar, COLOR_PAIR(2) | A_BOLD);
    wattron(menubar, COLOR_PAIR(2));
    waddstr(menubar, "Edit");
    wattroff(menubar, COLOR_PAIR(2));
    wmove(menubar, 0, 55);
    wattron(menubar, COLOR_PAIR(2) | A_BOLD);
    waddstr(menubar, "[F6]");
    wattroff(menubar, COLOR_PAIR(2) | A_BOLD);
    wattron(menubar, COLOR_PAIR(2));
    waddstr(menubar, "Make config");
    wattroff(menubar, COLOR_PAIR(2));
    wmove(menubar, 0, 72);
    wattron(menubar, COLOR_PAIR(2) | A_BOLD);
    waddstr(menubar, "[F7]");
    wattroff(menubar, COLOR_PAIR(2) | A_BOLD);
    wattron(menubar, COLOR_PAIR(2));
    waddstr(menubar, "Reboot phone");
    wattroff(menubar, COLOR_PAIR(2));
    wmove(menubar, 0, 90);
    wattron(menubar, COLOR_PAIR(2) | A_BOLD);
    waddstr(menubar, "[F10]");
    wattroff(menubar, COLOR_PAIR(2) | A_BOLD);
    wattron(menubar, COLOR_PAIR(2));
    waddstr(menubar, "Quit");
    wattroff(menubar, COLOR_PAIR(2));
}

void draw_titlebar(WINDOW *titlebar, int col) {
    wbkgd(titlebar, COLOR_PAIR(2));
    wmove(titlebar, 0, col / 2 - 15);
    wattron(titlebar, COLOR_PAIR(2) | A_BOLD);
    waddstr(titlebar, " Realtime Users for Asterisk ");
    wattroff(titlebar, COLOR_PAIR(2) | A_BOLD);
}
