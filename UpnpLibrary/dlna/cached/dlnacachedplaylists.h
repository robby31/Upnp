#ifndef DLNACACHEDPLAYLISTS_H
#define DLNACACHEDPLAYLISTS_H

#include "../dlnastoragefolder.h"
#include "dlnacachedfolder.h"
#include "medialibrary.h"

class DlnaCachedPlaylists : public DlnaStorageFolder
{
    Q_OBJECT

public:
    DlnaCachedPlaylists(MediaLibrary* library, QObject *parent = Q_NULLPTR);

    DlnaResource* getChild(int index, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;
    int getChildrenSize() const Q_DECL_OVERRIDE;

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE;

    QString getSystemName() const Q_DECL_OVERRIDE;

    // Returns the DisplayName that is shown to the Renderer.
    QString getDisplayName() const Q_DECL_OVERRIDE;

private:
    void refreshContent() Q_DECL_OVERRIDE;

private:
    MediaLibrary* library = Q_NULLPTR;
    QSqlQuery query;
    int nbChildren = 0;
};

#endif // DLNACACHEDPLAYLISTS_H
