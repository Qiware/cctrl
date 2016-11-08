include $(PROJ)/make/build.mak

CC = gcc
CFLAGS = -Wall -O2

INCLUDE = -I. -I$(PROJ)/src/incl
LIBS_PATH = -L$(PROJ)/lib -L/usr/local/lib
LIBS = -lcore

SRCS = ipstr.c
OBJS = ipstr.o
HEADS = $(call func_get_dep_head_list, $(SRCS))
TARGET = ipstr #recv
.PHONY: clean

all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LIBS_PATH)
	mv $@ $(PROJ_BIN)

$(OBJS): %.o : %.c $(HEADS)
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE)

clean:
	rm -fr $(TARGET) $(OBJS)
