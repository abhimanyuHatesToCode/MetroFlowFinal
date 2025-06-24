#include "mainwindow.h" // Your main window header
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // Creates the Qt application object

    // For high DPI scaling (optional but good practice)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    // For Qt 6, high DPI is often enabled by default or via environment variables.

    MainWindow w; // Creates an instance of your main window
    w.show();     // Shows the main window

    return a.exec(); // Starts the Qt event loop
}
