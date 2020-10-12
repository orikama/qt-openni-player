#include "OniPlayer.hpp"

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qdesktopwidget.h>


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("Oni Player");
    QGuiApplication::setApplicationDisplayName("Oni Player");

    OniPlayer player;

    const auto availableGeometry = QApplication::desktop()->availableGeometry(&player);
    player.resize(availableGeometry.width() / 2, availableGeometry.height() / 2);
    player.show();

    return app.exec();
}
