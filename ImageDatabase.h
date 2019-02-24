///////////////////////////////////////////////////////////////////////////////
/// (C) kindid ltd 2018+ //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef ImageDatabase_h
#define ImageDatabase_h

#include <QQuickImageProvider>
#include <QObject>
#include <QSqlDatabase>

class ImageDatabase;

class ImageDatabaseImageProvider : public QQuickImageProvider
{
public:
    ImageDatabaseImageProvider(ImageDatabase * idb);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

    ImageDatabase * idb;
};

// I think multiple inheritance has fucked this up in ways I don't understand
class ImageDatabase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString tableName READ getTableName)
public:
    ImageDatabase(QObject * parent = nullptr);
public slots:
    QString insertImage(const QString & key, const QImage & image);
    QString updateImage(const QString & key, const QImage & image);
    QString getTableName() const { return tableName; }
    QString getUrl(const QString & key) const;
public:
    bool valid() const;

    ImageDatabaseImageProvider * getImageProvider() { return &imageProvider; }

    QImage getImage(const QString & key);

    ImageDatabaseImageProvider imageProvider;
    QSqlDatabase database;
    QString folder;
    bool _valid;
    QString tableName;
};

#endif
