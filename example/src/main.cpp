#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <qqml.h>
#include <QDebug>
#include "qopenvdbgrid.h"
#include "qopenvdbgridpointsurfacegeometry.h"
#include "qopenvdbreader.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<QOpenVDBGridReader>("openvdb", 1, 0, "OpenVDBReader");
    qmlRegisterType<QOpenVDBGrid>("openvdb", 1, 0, "OpenVDBGrid");
    qmlRegisterType<QOpenVDBGridPointSurfaceGeometry>("openvdb", 1, 0, "OpenVDBGridPointSurfaceGeometry");
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

    int result = app.exec();
    return result;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE, LPSTR argv, int argc)
{
//    int argc=1;
//    char *argv[] = {"temp"};
    return main(argc, &argv);
}
#endif
