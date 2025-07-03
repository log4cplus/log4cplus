#include <QtCore>
#include <QTimer>
#include <log4cplus/qt6messagehandler.h>
#include <log4cplus/configurator.h>
#include <log4cplus/initializer.h>

int
main (int argc, char *argv[])
{
    // Initialize log4cplus.
    log4cplus::Initializer initializer;
    log4cplus::BasicConfigurator config;
    config.configure();

    // Create a Qt application instance.
    QCoreApplication app(argc, argv);
    // Install the Qt message handler.
    qInstallMessageHandler(log4cplus::qt6_message_handler);

    QTimer::singleShot(0, [&app]() {
        qInfo() << "Qt6MessageHandler Test Application Started";
        app.quit();
    });

    return app.exec();
}