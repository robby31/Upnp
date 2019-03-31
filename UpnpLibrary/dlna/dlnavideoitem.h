#ifndef DLNAVIDEOITEM_H
#define DLNAVIDEOITEM_H

#include "dlnaitem.h"

class DlnaVideoItem : public DlnaItem
{
    Q_OBJECT

public:
    DlnaVideoItem(QObject *parent = Q_NULLPTR);

    // Return upnp class
    QString getUpnpClass() const Q_DECL_OVERRIDE { return QString("object.item.videoItem"); }

    // Returns the XML (DIDL) representation of the DLNA node.
    QDomElement getXmlContentDirectory(QDomDocument *xml, QStringList properties) Q_DECL_OVERRIDE;

    // Returns the mimeType of the source.
    QString sourceMimeType() const Q_DECL_OVERRIDE;

    // returns the bitrate of the movie
    qint64 bitrate() const Q_DECL_OVERRIDE;

    // return true if the track shall be transcoded
    bool toTranscode() const Q_DECL_OVERRIDE { return true; }

    virtual QString resolution() const = 0;
    virtual QStringList subtitleLanguages() const = 0;
    virtual QStringList audioLanguages() const = 0;
    virtual QString framerate() const = 0;
};

#endif // DLNAVIDEOITEM_H
