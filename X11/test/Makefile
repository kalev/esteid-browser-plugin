CPPFLAGS = `pkg-config --cflags gtkmm-2.4`
LIBS = `pkg-config --libs gtkmm-2.4`

testgtkui: testgtkui.o pininputdialog.o whitelistdialog.o main.o
	$(CXX) $(LDFLAGS) -o $@ $+ $(LIBS)

clean:
	rm -f testgtkui *.o
