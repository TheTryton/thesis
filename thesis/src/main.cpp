#include <QApplication>
#include <TGMainWindow.hpp>

int main(int argc, char** argv)
{
    QApplication app{argc, argv};

    TGMainWindow mainWindow{};
    mainWindow.show();

    return app.exec();
}
