#include "videoplayer.hpp"

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qdesktopwidget.h>

//#include <QtCore/qcommandlineparser.h>
//#include <QtCore/qcommandlineoption.h>
//#include <QtCore/qdir.h>



int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("Video Widget Example");
    QCoreApplication::setOrganizationName("QtProject");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QGuiApplication::setApplicationDisplayName(QCoreApplication::applicationName());

    /*QCommandLineParser parser;
    parser.setApplicationDescription("Qt Video Widget Example");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("url", "The URL to open.");
    parser.process(app);*/

    VideoPlayer player;
    /*if (parser.positionalArguments().isEmpty() == false) {
        const auto url = QUrl::fromUserInput(parser.positionalArguments().constFirst(),
                                             QDir::currentPath(), QUrl::AssumeLocalFile);
        player.SetUrl(url);
    }*/

    const auto availableGeometry = QApplication::desktop()->availableGeometry(&player);
    player.resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
    player.show();

    return app.exec();
}
