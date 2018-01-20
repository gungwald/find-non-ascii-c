CC=gcc
TARGET=find-non-ascii
OBJS=$(TARGET).o java-string-compat.o
INSTALL_DIR=/usr/local/bin
INSTALL=sudo install

# The definition of _XOPEN_SOURCE is needed to include strdup which is not
# part of the ISO C11 standard, which is being specified. Defining
# C_POSIX_SOURCE >= 200804 is supposed to work also but it does not.
CFLAGS=-std=c11 -Wall -D_XOPEN_SOURCE=500

ifdef RELEASE
	CFLAGS+=-O2
else	
	CFLAGS+=-g
endif

$(TARGET): $(OBJS)

$(TARGET).o: $(TARGET).c

java-string-compat.o: java-string-compat.c java-string-compat.h

install: $(TARGET)
	$(INSTALL) $(TARGET) $(INSTALL_DIR)

clean:
	$(RM) $(TARGET) $(OBJS)

