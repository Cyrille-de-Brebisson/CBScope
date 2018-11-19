#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QIcon>
#include "model.h"

static QGuiApplication *qtapp;
QString getAppPath()
{
    return qtapp->applicationDirPath();
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QIcon iconApplication;
#ifdef WINDOWS
    iconApplication = readIcoFile("qrc:/Resources/cbscope.ico");
#else
    iconApplication.addFile("qrc:/Resources/cbscope.icns");
#endif

    qmlRegisterSingletonType<CBSModel>("CBSModel", 1, 0, "CBSModel", CBSModel::SingletonProvider);
    qmlRegisterType<CBSModelHoggingWork>("CBSModelHoggingWork", 1, 0, "CBSModelHoggingWork");
    qmlRegisterType<CBSModelParabolizingWork>("CBSModelParabolizingWork", 1, 0, "CBSModelParabolizingWork");
    qmlRegisterType<CBSModelScope>("CBSModelScope", 1, 0, "CBSModelScope");
    qmlRegisterType<CBScopeIlumination>("CBScopeIlumination", 1, 0, "CBScopeIlumination");
    qmlRegisterType<CBScopeMesure>("CBScopeMesure", 1, 0, "CBScopeMesure");
    qmlRegisterType<CBScopeCouder>("CBScopeCouder", 1, 0, "CBScopeCouder");
    qmlRegisterType<CBScopeMes>("CBScopeMes", 1, 0, "CBScopeMes");

    QApplication app(argc, argv); qtapp= &app;
    CBSModel::SingletonProvider(nullptr, nullptr); // Create the model and load files...

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    int ret= app.exec();
    CBSModel::singleton->saveFile();
    return ret;
}
