#include <gtkmm/main.h>
#include "testgtkui.h"

int main(int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);

    TestGtkUI window;
    Gtk::Main::run(window);

    return 0;
}
