#include <QApplication>
#include "frontend/views/MainWindow.hpp"

int main(int argc, char** argv)
{
    QApplication application(argc, argv);
    frontend::MainWindow window;
    window.show();
    return application.exec();
}
