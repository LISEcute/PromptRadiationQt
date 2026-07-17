#include "MainWindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/Icons/lisepp_small.png")));

    MainWindow w;
    w.show();

    return app.exec();
}
//--------------------------------------------------------------------------------
