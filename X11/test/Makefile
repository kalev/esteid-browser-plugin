CPPFLAGS = `pkg-config --cflags gtkmm-2.4`
LIBS = `pkg-config --libs gtkmm-2.4`

testgtkui: testgtkui.o pininput.o whitelistdialog.o main.o
	$(CC) $(LDFLAGS) -o $@ $+ $(LIBS)

clean:
	rm -f testgtkui *.o
