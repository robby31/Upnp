#ifndef DLNASTORAGEFOLDER_H
#define DLNASTORAGEFOLDER_H

#include "dlnaresource.h"

class DlnaStorageFolder : public DlnaResource
{
    Q_OBJECT

public:
    explicit DlnaStorageFolder(QObject *parent = Q_NULLPTR);
    ~DlnaStorageFolder() Q_DECL_OVERRIDE;

    // Return upnp class
    QString getUpnpClass() const Q_DECL_OVERRIDE { return QString("object.container.storageFolder"); }

    bool isFolder() const Q_DECL_OVERRIDE { return true; }

    // Returns the XML (DIDL) representation of the DLNA node.
    QDomElement getXmlContentDirectory(QDomDocument *xml, QStringList properties) Q_DECL_OVERRIDE;

    // Returns album art in jpeg format
    QImage getAlbumArt() Q_DECL_OVERRIDE { return QImage(); }

public:
    static qint64 objectCounter;
};

#endif // DLNASTORAGEFOLDER_H
