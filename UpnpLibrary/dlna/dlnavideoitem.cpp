#include "dlnavideoitem.h"

DlnaVideoItem::DlnaVideoItem(QObject *parent):
    DlnaItem(parent)
{
    setTranscodeFormat(H264_AC3);   // default transcode format
}

/*
* Returns XML (DIDL) representation of the DLNA node. It gives a
* complete representation of the item, with as many tags as available.
*
* Reference: http://www.upnp.org/specs/av/UPnP-av-ContentDirectory-v1-Service.pdf
*/
QDomElement DlnaVideoItem::getXmlContentDirectory(QDomDocument *xml, QStringList properties) {
    if (!xml)
        return QDomElement();

    QDomElement xml_obj = xml->createElement("item");

    updateXmlContentDirectory(xml, &xml_obj, properties);

    // properties optional of videoItem

    if (properties.contains("*") || properties.contains("upnp:genre")) {
        QDomElement upnpGenre = xml->createElement("upnp:genre");
        upnpGenre.appendChild(xml->createTextNode(metaDataGenre()));
        xml_obj.appendChild(upnpGenre);
    }

    if (properties.contains("*") || properties.contains("upnp:longDescription")) {

    }

    if (properties.contains("*") || properties.contains("upnp:producer")) {

    }

    if (properties.contains("*") || properties.contains("upnp:rating")) {

    }

    if (properties.contains("*") || properties.contains("upnp:actor")) {

    }

    if (properties.contains("*") || properties.contains("upnp:director")) {

    }

    if (properties.contains("*") || properties.contains("dc:description")) {

    }

    if (properties.contains("*") || properties.contains("dc:publisher")) {

    }

    if (properties.contains("*") || properties.contains("dc:language")) {

    }

    if (properties.contains("*") || properties.contains("dc:relation")) {

    }

    if (properties.contains("*") || properties.contains("upnp:albumArtURI"))
    {
        if (thumbnailUrl().isValid())
        {
            QDomElement upnpAlbumArtURI = xml->createElement("upnp:albumArtURI");
            upnpAlbumArtURI.setAttribute("xmlns:dlna", "urn:schemas-dlna-org:metadata-1-0/");
            upnpAlbumArtURI.setAttribute("dlna:profileID", "JPEG_TN");
//            upnpAlbumArtURI.appendChild(xml->createTextNode(QString("http://%1:%2/get/%3/thumbnail0000%4&").arg(getHostUrl().host()).arg(getHostUrl().port()).arg(getResourceId(), getDisplayName().toUtf8().toPercentEncoding().constData())));
            upnpAlbumArtURI.appendChild(xml->createTextNode(thumbnailUrl().url()));
            xml_obj.appendChild(upnpAlbumArtURI);
        }
    }

    // add <res> element

    QTime duration(0, 0, 0);
    QDomElement res = xml->createElement("res");
    res.setAttribute("xmlns:dlna", "urn:schemas-dlna-org:metadata-1-0/");

    // mandatory properties: protocolInfo
    res.setAttribute("protocolInfo", getProtocolInfo());

    // optional properties
    if ((properties.contains("*") || properties.contains("res@bitrate")) && bitrate() != -1) {
        // bitrate in bytes/sec
        res.setAttribute("bitrate", QString("%1").arg(qRound(double(bitrate())/8.0)));
    }

    if (properties.contains("*") || properties.contains("res@resolution")) {
        res.setAttribute("resolution", resolution());
    }

    if ((properties.contains("*") || properties.contains("res@duration")) && getLengthInSeconds() > 0) {
        res.setAttribute("duration", QString("%1").arg(duration.addSecs(getLengthInSeconds()).toString("hh:mm:ss")));
    }

    if (properties.contains("*") || properties.contains("res@sampleFrequency")) {
        res.setAttribute("sampleFrequency", QString("%1").arg(samplerate()));
    }

    if (properties.contains("*") || properties.contains("res@nrAudioChannels")) {
        res.setAttribute("nrAudioChannels", QString("%1").arg(channelCount()));
    }

    if ((properties.contains("*") || properties.contains("res@size")) && size() != -1) {
        // size in bytes
        res.setAttribute("size", QString("%1").arg(size()));
    }

    res.appendChild(xml->createTextNode(QString("http://%2:%3/get/%1/%4").arg(getResourceId(), getHostUrl().host()).arg(getHostUrl().port()).arg(getName().toUtf8().toPercentEncoding().constData())));

    xml_obj.appendChild(res);

    return xml_obj;
}

qint64 DlnaVideoItem::bitrate() const
{
    // returns bitrate in bits/sec
    if (toTranscode())
    {
        if (format() == H264_AC3 || format() == H264_AAC)
            return 2500000;

        return 4558800;
    }

    return metaDataBitrate();
}

QString DlnaVideoItem::sourceMimeType() const
{
    QString format = metaDataFormat();
    if (format == "avi")
        return AVI_TYPEMIME;

    if (format == "matroska,webm")
        return MATROSKA_TYPEMIME;

    if (format == "mov,mp4,m4a,3gp,3g2,mj2")
        return MP4_TYPEMIME;

    if (format == "asf")
        return ASF_TYPEMIME;

    if (format == "hls,applehttp")
        return M3U8_TYPEMIME;

    qCritical() << "Unable to define mimeType of DlnaVideoItem: " << format << getSystemName();

    // returns unknown mimeType
    return UNKNOWN_VIDEO_TYPEMIME;
}
