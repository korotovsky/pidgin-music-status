CFLAGS=`pkg-config --cflags glib-2.0`
CFLAGS+=`pkg-config --cflags gtk+-2.0`
CFLAGS+=`pkg-config --cflags pidgin`
CFLAGS+=`pkg-config --cflags dbus-1 dbus-glib-1`
CFLAGS+= -fPIC
LDFLAGS=`pkg-config --libs glib-2.0`
LDFLAGS+=`pkg-config --libs pidgin`
LDFLAGS+=`pkg-config --libs dbus-1 dbus-glib-1`

mstatus.so: mstatus.o
	gcc $(LDFLAGS) -shared -g -o $@ $?

mstatus.o: mstatus.c
	gcc -c $(CFLAGS) -g -o $@ $?

clean:
	rm -f mstatus.so
	rm -f mstatus.o

install:
	cp mstatus.so ~/.purple/plugins/
