#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <qqmlcontext.h>

#include "CommandRecords.h"
#include "QueryRecords.h"
#include "QMLSchedulerController.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QQmlContext *ctxt = engine.rootContext();
    QMLSchedulerController contr(ctxt);
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    return app.exec();
}
