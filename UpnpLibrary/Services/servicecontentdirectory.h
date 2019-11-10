#ifndef SERVICECONTENTDIRECTORY_H
#define SERVICECONTENTDIRECTORY_H

#include "abstractservice.h"
#include "dlna/cached/dlnacachedrootfolder.h"
#include "Devices/mediarenderer.h"
#include "Devices/mediarenderermodel.h"
#include "didllite.h"

class ServiceContentDirectory : public AbstractService
{
    Q_OBJECT

public:
    explicit ServiceContentDirectory(MediaRendererModel *model, UpnpDevice *upnpParent, QObject *parent = Q_NULLPTR);
    ~ServiceContentDirectory() Q_DECL_OVERRIDE;

    bool replyRequest(HttpRequest *request) Q_DECL_OVERRIDE;

private:
    void initDescription();
    void initActions();
    void initStateVariables();

    DlnaResource *getDlnaResource(const QString &hostaddress, const QString &objId);

protected:
    bool replyAction(HttpRequest *request, const SoapAction &action) Q_DECL_OVERRIDE;

signals:
    void addFolderSignal(QString folder);
    void folderAdded(QString folder);
    void error_addFolder(QString folder);
    void scanFolder(QString path);

    void addNetworkLinkSignal(const QString& url);
    void linkAdded(QString url);
    void error_addNetworkLink(QString url);
    void brokenLink(QString url, QString title);

    void reloadLibrarySignal(const QStringList& localFolder);

    void updateMediaData(const QString &filename, const QHash<QString, QVariant> &data);
    void updateMediaFromId(const int &id, const QHash<QString, QVariant> &data);
    void incrementCounterPlayedSignal(const QString &filename);

    void servingRendererSignal(QString ip, const QString &mediaName);

    // emit signal when serving is finished
    //   status = 0 if serving finished successfully
    //   status = 1 if error occurs
    void servingFinishedSignal(QString host, QString filename, int status);

private slots:
    void _addFolder(const QString &folder);

    void folderAddedSlot(const QString& folder);

    void reloadLibrary();

    void readTimeSeekRange(const QString &data, qint64 *start, qint64*end);

    void streamReadyToOpen();

    void servingMedia(const QString& filename, int playedDurationInMs);

    void servingFinished(const QString &host, const QString& filename, int status);

    void dlnaContentUpdated();

    void mediaRendererDestroyed(const QString &hostaddress);

private:
    DlnaCachedRootFolder rootFolder;

    // root folder containing DLNA nodes
    QStringList listFolderAdded;

    QHash<QString, DlnaResource*> m_dlnaresources;

    QThread *m_streamingThread = Q_NULLPTR;

    MediaRendererModel *m_renderersModel = Q_NULLPTR;
};

#endif // SERVICECONTENTDIRECTORY_H
