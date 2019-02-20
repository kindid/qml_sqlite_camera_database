#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>  // why? this is really tedious!
#include <QDebug>
#include <ImageDatabase.h>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    ImageDatabase * idb = new ImageDatabase();
    QQmlApplicationEngine engine;
    qDebug() << engine.offlineStoragePath();
    engine.addImageProvider(QLatin1String("user_images"), idb->getImageProvider());
    engine.rootContext()->setContextProperty("user_images", idb);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
