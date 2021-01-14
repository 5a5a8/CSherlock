bin/csherlock: obj/main.o obj/verbose.o obj/help.o obj/csv.o obj/argparser.o obj/regexcheck.o obj/webrequest.o
	gcc -g -Wextra -o bin/csherlock obj/main.o obj/verbose.o obj/help.o obj/csv.o obj/argparser.o obj/regexcheck.o -L/usr/local/lib -lpcre2-8 obj/webrequest.o -lcurl

obj/main.o: src/main.c src/verbose.h
	gcc -g -Wextra -c src/main.c -o obj/main.o

obj/verbose.o: src/verbose.c src/verbose.h
	gcc -g -Wextra -c src/verbose.c -o obj/verbose.o

obj/help.o: src/help.c src/help.h
	gcc -g -Wextra -c src/help.c -o obj/help.o

obj/csv.o: src/csv.c src/csv.h
	gcc -g -Wextra -c src/csv.c -o obj/csv.o

obj/argparser.o: src/argparser.c src/argparser.h
	gcc -g -Wextra -c src/argparser.c -o obj/argparser.o

obj/regexcheck.o: src/regexcheck.c src/regexcheck.h
	gcc -g -Wextra -c src/regexcheck.c -o obj/regexcheck.o

obj/webrequest.o: src/webrequest.c src/webrequest.h
	gcc -g -Wextra -c src/webrequest.c -o obj/webrequest.o
