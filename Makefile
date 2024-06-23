PREFIX = /usr
CC = gcc
RM = rm -f
CP = cp

CFLAGS = -Wall -ggdb -D_GNU_SOURCE
PKGS = 
LIBS = -lc -lcurl

TARGET = m3u8_downloader.exe

.PHONY: clean all


all: $(TARGET)

$(TARGET): m3u8_downloader.o curl_service.o
	$(CC) -o $@ $^ $(LIBS)

m3u8_downloader.o: m3u8_downloader.c
	$(CC) $(CFLAGS) -c $^ $(PKGS)

curl_service.o: curl_service.c
	$(CC) $(CFLAGS) -c $^ $(PKGS)

clean:
	$(RM) *.o
