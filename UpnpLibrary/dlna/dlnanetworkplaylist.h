#ifndef DLNANETWORKPLAYLIST_H
#define DLNANETWORKPLAYLIST_H

#include "dlnastoragefolder.h"
#include "dlnanetworkvideo.h"
#include "mediastreaming.h"

class DlnaNetworkPlaylist : public DlnaStorageFolder
{
    Q_OBJECT

public:
    explicit DlnaNetworkPlaylist(const QUrl &url, QObject *parent = Q_NULLPTR);

    bool isValid() const;

    QUrl url() const;
    const AbstractPlaylist::T_URL *getMediaInfo(const int &index);

    DlnaResource* getChild(int index, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;
    int getChildrenSize() const Q_DECL_OVERRIDE;

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE;

    QString getSystemName() const Q_DECL_OVERRIDE;

    // Returns the DisplayName that is shown to the Renderer.
    QString getDisplayName() const Q_DECL_OVERRIDE;

private:
    AbstractPlaylist *m_playlist = Q_NULLPTR;
    QHash<int, DlnaNetworkVideo*> l_children;
};

#endif // DLNANETWORKPLAYLIST_H
