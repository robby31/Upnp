#ifndef DLNACACHEDFOLDERMETADATA_H
#define DLNACACHEDFOLDERMETADATA_H

#include "../dlnastoragefolder.h"
#include "dlnacachedfolder.h"

#include "medialibrary.h"

class DlnaCachedFolderMetaData : public DlnaStorageFolder
{
    Q_OBJECT

public:
    explicit DlnaCachedFolderMetaData(MediaLibrary* library,
                                      const QString& stringQuery,
                                      QString stringQueryForChild,
                                      QString name,
                                      QObject *parent = Q_NULLPTR);

    DlnaResource* getChild(int index, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;
    int getChildrenSize() const Q_DECL_OVERRIDE { return nbChildren; }

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE { return m_name; }

    QString getSystemName() const Q_DECL_OVERRIDE { return m_name; }

    // Returns the DisplayName that is shown to the Renderer.
    QString getDisplayName() const Q_DECL_OVERRIDE { return m_name; }

    void setNetworkAccessManager(QNetworkAccessManager *manager) { m_nam = manager; }


private:
    MediaLibrary* library;
    QString m_name;
    QSqlQuery query;
    QString stringQueryForChild;
    int nbChildren;
    QNetworkAccessManager *m_nam;
};

#endif // DLNACACHEDFOLDERMETADATA_H
