CC=gcc

all:
	$(CC) main.c menu.c functions.c mysqlFunctions.c createdb.c readConfig.c curlFunction.c -o rufa `mysql_config --cflags --libs` -lcurl

install:
	cp ./rufa /usr/bin/
	cp -n ./rufa.conf /etc/

uninstall:
	rm -f /usr/bin/rufa
