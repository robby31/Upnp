#ifndef DLNACACHEDROOTFOLDER_H
#define DLNACACHEDROOTFOLDER_H

#include "../dlnarootfolder.h"

#include <qmimedatabase.h>
#include "medialibrary.h"

#include "../dlnamusictrackfile.h"
#include "../dlnavideofile.h"
#include "dlnacachedfolder.h"
#include "dlnacachedgroupedfoldermetadata.h"
#include "../dlnarootfolder.h"
#include "../dlnanetworkvideo.h"

class DlnaCachedRootFolder : public DlnaRootFolder
{
    Q_OBJECT

public:
    explicit DlnaCachedRootFolder(QObject *parent = Q_NULLPTR);

    QSqlQuery getAllNetworkLinks() { return library.getAllNetworkLinks(); }

    void setNetworkAccessManager(QNetworkAccessManager *nam);

    void readDirectory(const QDir& folder);

private:

    void addResource(const QFileInfo& fileinfo);
    void addResource(const QUrl& url);

signals:
    void linkAdded(QString url);
    void error_addNetworkLink(QString url);
    void scanFolder(QString path);

private slots:
    void updateLibrary(const QString &filename, const QHash<QString, QVariant> &data);
    void updateLibraryFromId(const int &id, const QHash<QString, QVariant> &data);
    void incrementCounterPlayed(const QString &filename);

    // returns true if the folder is added to Root.
    bool addFolderSlot(const QString& path) Q_DECL_OVERRIDE;

    void addNetworkLink(const QString &url);

    void networkLinkAnalyzed(const QList<QUrl> &urls);
    void networkLinkError(const QString &message);

public slots:
    void reloadLibrary(const QStringList &localFolder);

private:
    MediaLibrary library;
    QMimeDatabase mimeDb;
    DlnaRootFolder rootFolder;
    DlnaCachedFolder *recentlyPlayedChild = Q_NULLPTR;
    DlnaCachedFolder *resumeChild = Q_NULLPTR;
    DlnaCachedFolder *favoritesChild = Q_NULLPTR;
    DlnaCachedFolder *lastAddedChild = Q_NULLPTR;
    DlnaCachedGroupedFolderMetaData *youtube = Q_NULLPTR;
    QNetworkAccessManager *m_nam = Q_NULLPTR;
};

#endif // DLNACACHEDROOTFOLDER_H
