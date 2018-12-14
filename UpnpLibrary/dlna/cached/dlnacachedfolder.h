#ifndef DLNACACHEDFOLDER_H
#define DLNACACHEDFOLDER_H

#include "../dlnastoragefolder.h"
#include "dlnacachedvideo.h"
#include "dlnacachednetworkvideo.h"
#include "dlnacachedmusictrack.h"

#include "medialibrary.h"

#include <QDir>

class DlnaCachedFolder : public DlnaStorageFolder
{
    Q_OBJECT

public:
    explicit DlnaCachedFolder(MediaLibrary* library,
                              const QSqlQuery& query,
                              const QString& name, bool cacheEnabled = false, int maxSize = -1, QObject *parent = Q_NULLPTR);

    DlnaResource* getChild(int index, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;
    int getChildrenSize() const Q_DECL_OVERRIDE;

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE { return name; }

    QString getSystemName() const Q_DECL_OVERRIDE { return name; }

    // Returns the DisplayName that is shown to the Renderer.
    QString getDisplayName() const Q_DECL_OVERRIDE { return name; }

    void setLimitSizeMax(const int &size) { limitSizeMax = size; refreshContent(); }

    void setNetworkAccessManager(QNetworkAccessManager *manager) { m_nam = manager; }

private:
    void refreshContent() Q_DECL_OVERRIDE;

private:
    MediaLibrary* library = Q_NULLPTR;
    QString name;
    QSqlQuery query;
    int nbChildren;
    bool cacheEnabled;
    QList<int> cache;
    int limitSizeMax;
    QNetworkAccessManager *m_nam = Q_NULLPTR;
};

#endif // DLNACACHEDFOLDER_H
