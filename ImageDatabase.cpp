///////////////////////////////////////////////////////////////////////////////
/// (C) kindid ltd 2018+ //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "ImageDatabase.h"
#include <QImage>
#include <QBuffer>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QStandardPaths>
#include <here.h>

// 2 objects - one is the database stuff which is exposed to the app
// the other is the image provider which is bound to the DB but
// only visible as an "image" thingy...
// lots of stuff to do...

void dumpQueryError(const QSqlQuery & query)
{
    here << query.lastError().text();
    here << query.lastError().databaseText();
    here << query.lastError().driverText();
}

ImageDatabaseImageProvider::ImageDatabaseImageProvider(ImageDatabase * idb)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , idb(idb)
{

}

QImage ImageDatabaseImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    // interesting - document this... the id passed in here is truncated AFTER the user_images bit
    //
    here << '[' << id << ']' << requestedSize << size << *size;

    int split = id.indexOf('/');
    if (split > 0) {
        QString user_key = id.mid(0, split);
        here << user_key;
        return idb->getImage(user_key);
    } else {
        // return some sort of error or default image
    }
//    idb->getImage(id);

    QSize sizeQ;
    if (requestedSize.isEmpty()) {
        sizeQ = QSize(512, 256);
    } else {
        sizeQ = requestedSize;
    }
    QImage image(sizeQ, QImage::Format_ARGB32);
    if (size) {
        *size = sizeQ;
    }
    image.fill(0xff00ff00);
    return image;
}

////////////////////////////////////////////////////////////////////////////////

ImageDatabase::ImageDatabase(QObject * parent)
    : QObject(parent)
    , imageProvider(this)
    , tableName("user_images")

{
    // Rationale on schema
    //  firstly there is a "simple" index - this is the index that you, the user, provide
    //   example - user name or user id
    //  then there must be a cache key so that the URL is updated (it is returned after an update)
    //   this is essential as image caching (QML side) is based on the URL
    //   could use...
    //    time (if accurate enough e.g. nanoseconds or better)
    //    a monotonically incrementing counter
    //   in truth only a single image is actually stored so the DB disregards the
    //   time/cache key passed in - it's only interested in the actual ID
    database = QSqlDatabase::addDatabase("QSQLITE");

    QString schema =
        "CREATE TABLE IF NOT EXISTS images ("
            "key string primary key, "
            "cachekey string, "
            "image blob"
        ")";

    QDir dirAppData(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!dirAppData.exists()) {
        if (dirAppData.mkpath(".")) {
            folder = dirAppData.path();
        }
    } else {
        folder = dirAppData.path();
    }

    here << folder;

    database.setDatabaseName(folder + QDir::separator() + "app.sqlite");

    if (database.open()) {
        here << "database open";
    } else {
        here << "fucked";
    }

    QSqlQuery query(database);
    if (query.exec(schema)) {
        here << query.executedQuery();
        _valid = true;
//        setTable("images)";
//        here << "success";
//        return true;
    } else {
        _valid = false;
        //here << tableName();
        here << query.lastError().text();
        here << query.lastError().databaseText();
        here << query.lastError().driverText();
    }

    // all dead
    QImage image1("/Users/kuiash/Downloads/grass.jpg");
    QImage image2("/Users/kuiash/Downloads/2548a.jpg");
    QImage image3("/Users/kuiash/Downloads/2748af47b6b24f76410adb8bdaf1edf1.jpg");
    QImage image4("/Users/kuiash/Downloads/2etmxk.jpg");

    here << insertImage("first_key", image1);
    here << insertImage("seconds_key", image2);
    here << insertImage("third_key", image3);
//    insertImage("fourth_key", image4);

    here << updateImage("first_key", image3);

    if (!query.exec("SELECT image from images WHERE key='first_key'")) {
        here << "Error getting image from table:\n" << query.lastError();
    } else {
        query.first();
        QByteArray outByteArray = query.value(0).toByteArray();
        QFile file("/Users/kuiash/Downloads/blah.jpg");
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            file.write(outByteArray);
//            outPixmap.loadFromData(outByteArray );
        }
//        QPixmap outPixmap = QPixmap();
//        outPixmap.loadFromData( outByteArray );
//        db.close();
    }
//    return false;
}

bool ImageDatabase::valid() const
{
    return _valid;
}

QString ImageDatabase::insertImage(const QString & key, const QImage & image)
{
    QByteArray inByteArray;
    QBuffer inBuffer(&inByteArray);
    inBuffer.open(QIODevice::WriteOnly);
    image.save(&inBuffer, "JPG");

    QSqlQuery query(database);
    query.prepare( "INSERT INTO images (key, cacheKey, image) VALUES (:key, 0, :image)" );
    query.bindValue(":key", key);
    query.bindValue(":image", inByteArray);
    if (query.exec()) {
        here << "YOU DID IT!!!";
        return QString("images://users/%1/%2").arg(key).arg(0);
    } else {
        dumpQueryError(query);
        return QString();
    }
//    return "hi";
}


// split into "insert" and "update" as they do different things
QString ImageDatabase::updateImage(const QString & key, const QImage & image)
{
    QByteArray inByteArray;
    QBuffer inBuffer(&inByteArray);
    inBuffer.open(QIODevice::WriteOnly);
    image.save(&inBuffer, "JPG");

    QSqlQuery query(database);
    query.prepare("UPDATE images SET image = :image, cacheKey = cacheKey + 1 WHERE key = :key");
    query.bindValue(":image", inByteArray);
    query.bindValue(":key", key);
    if (query.exec()) {
        here << "UPDATED";
        query.prepare("SELECT cacheKey FROM images WHERE key=:key");
        query.bindValue(":key", key);
        if (query.exec()) {
            here << "SELECTED";
            query.first();
            int cacheKey = query.value(0).toInt();
            return QString("image://users/%1/%2").arg(key).arg(cacheKey);
        } else  {
            dumpQueryError(query);
        }
    } else {
        dumpQueryError(query);
    }
    return "image://users/?/?";
}

QImage ImageDatabase::getImage(const QString & key)
{
    here << QString("[%1]").arg(key);
    // todo; check the number of results
    // todo; check loading succeeded
    // todo; return "failure" icon if it did not (can be static image - takes space but.. *meh*)
    QImage out;
    QSqlQuery query(database);
    query.prepare("SELECT image FROM images WHERE key=:key");
    query.bindValue(":key", key);
    if (query.exec()) {
        query.first();
        QByteArray binary_jpeg = query.value(0).toByteArray();
        here << out.loadFromData(binary_jpeg, "JPG");
    } else {
        dumpQueryError(query);
    }
    return out;
}


////////////////////////////////////////////////////////////////////////////////
