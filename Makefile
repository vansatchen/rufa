CC=gcc

all:
	$(CC) main.c menu.c functions.c mysqlFunctions.c createdb.c readConfig.c -o rufa `mysql_config --cflags --libs`

install:
	cp -a ./rufa /usr/bin/
	cp -a ./rufa.conf /etc/
