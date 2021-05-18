CC=gcc

all:
	$(CC) main.c menu.c functions.c mysqlFunctions.c createdb.c readConfig.c -o rufa `mysql_config --cflags --libs`

install:
	cp ./rufa /usr/bin/
	cp -n ./rufa.conf /etc/
