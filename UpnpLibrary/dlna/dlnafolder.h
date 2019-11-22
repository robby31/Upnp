#ifndef DLNAFOLDER_H
#define DLNAFOLDER_H

#include "dlnastoragefolder.h"
#include "dlnamusictrackfile.h"
#include "dlnavideofile.h"
#include <QFileInfo>
#include <QDir>

class DlnaFolder : public DlnaStorageFolder
{
    Q_OBJECT

public:
    explicit DlnaFolder(const QString& filename, QObject *parent = Q_NULLPTR);

    DlnaResource* getChild(int index, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;
    int getChildrenSize() const Q_DECL_OVERRIDE { return children.size(); }

    // returns the file path of children
    QList<QFileInfo> getChildrenFileInfo() const { return children; }

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE { return fileinfo.fileName(); }

    QString getSystemName() const Q_DECL_OVERRIDE { return fileinfo.absoluteFilePath(); }

    // Returns the DisplayName that is shown to the Renderer.
    QString getDisplayName() const Q_DECL_OVERRIDE { return fileinfo.completeBaseName(); }

private:
    QFileInfo fileinfo;
    QList<QFileInfo> children;
};

#endif // DLNAFOLDER_H
