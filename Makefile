CC=gcc
TARGET=find-nonascii
#CFLAGS=-std=c11 -O2 -Wall
CFLAGS=-std=c11 -g -Wall
INSTALL_DIR=/usr/local/bin
INSTALL=sudo install

$(TARGET): $(TARGET).o

$(TARGET).o: $(TARGET).c

install: $(TARGET)
	$(INSTALL) $(TARGET) $(INSTALL_DIR)

clean:
	$(RM) $(TARGET) $(TARGET).o

