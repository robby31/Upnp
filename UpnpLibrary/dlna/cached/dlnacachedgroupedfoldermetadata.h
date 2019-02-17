#ifndef DLNACACHEDGROUPEDFOLDERMETADATA_H
#define DLNACACHEDGROUPEDFOLDERMETADATA_H

#include "../dlnastoragefolder.h"
#include "dlnacachedfoldermetadata.h"

#include "medialibrary.h"

class DlnaCachedGroupedFolderMetaData : public DlnaStorageFolder
{
    Q_OBJECT

public:
    DlnaCachedGroupedFolderMetaData(MediaLibrary* library, const QString& name, QObject *parent = Q_NULLPTR);
    ~DlnaCachedGroupedFolderMetaData() Q_DECL_OVERRIDE;

    DlnaResource* getChild(int index, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;
    int getChildrenSize() const Q_DECL_OVERRIDE { return children.size(); }

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE { return name; }

    QString getSystemName() const Q_DECL_OVERRIDE { return name; }

    // Returns the DisplayName that is shown to the Renderer.
    QString getDisplayName() const Q_DECL_OVERRIDE { return name; }

    void addFolder(const QString& stringQuery, const QString& stringQueryForChild, const QString& name);

private:
    MediaLibrary* library;
    QString name;
    QList<DlnaCachedFolderMetaData*> children;

public:
    static qint64 objectCounter;
};

#endif // DLNACACHEDGROUPEDFOLDERMETADATA_H
