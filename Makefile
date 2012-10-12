OBJS := http-parser/http_parser.o main.o strutil.o lstutil.o

px: $(OBJS)
	gcc -o px $(OBJS)

%.o: %.c
	gcc -g -ggdb -std=c99 -Wall -Wextra -W -Wno-unused -Ihttp-parser -c -o $@ $<

