SRCS=$(wildcard */*.c)
OBJS=$(SRCS:.c=.o)
CC=gcc
#CFLAGS+=-Wall -Werror -fsanitize=address
CFLAGS= -Wall -Werror

.PHONY:all
# all :$(OBJS)
# #'\'是续行符
# 	$(foreach obj,$(OBJS),\
# 	$(CC) $(obj) -o $(basename $(obj));)
all:$(OBJS)

%.o :%.c
	$(CC) $< -o $(basename $@) $(CFLAGS)

.PHONY:clean
clean:
# rm -f $(OBJS)
	rm -f $(basename $(OBJS))


	
