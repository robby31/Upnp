#include "dlnamusictrack.h"

DlnaMusicTrack::DlnaMusicTrack(QObject *parent):
    DlnaItem(parent)
{
}

qint64 DlnaMusicTrack::bitrate() const
{
    // returns bitrate in bits/sec
    if (toTranscode())
    {
        if (format() == MP3 || format() == AAC)
            return 320000;

        if (format() == LPCM_S16BE || format() == ALAC)
        {
            if (samplerate() == 44100)
                return 1411200;

            return 1536000;
        }

        // invalid transcode format

        return -1;
    }

    return metaDataBitrate();
}

/*
* Returns XML (DIDL) representation of the DLNA node. It gives a
* complete representation of the item, with as many tags as available.
*
* Reference: http://www.upnp.org/specs/av/UPnP-av-ContentDirectory-v1-Service.pdf
*/
QDomElement DlnaMusicTrack::getXmlContentDirectory(QDomDocument *xml, QStringList properties)  {
    if (!xml)
        return QDomElement();

    QDomElement xml_obj = xml->createElement("item");

    updateXmlContentDirectory(xml, &xml_obj, properties);

    // properties optional of audioItem

    if (properties.contains("*") || properties.contains("upnp:genre")) {
        QDomElement upnpGenre = xml->createElement("upnp:genre");
        upnpGenre.appendChild(xml->createTextNode(metaDataGenre()));
        xml_obj.appendChild(upnpGenre);
    }

    if (properties.contains("*") || properties.contains("dc:description")) {

    }

    if (properties.contains("*") || properties.contains("upnp:longDescription")) {

    }

    if (properties.contains("*") || properties.contains("dc:publisher")) {

    }

    if (properties.contains("*") || properties.contains("dc:language")) {

    }

    if (properties.contains("*") || properties.contains("dc:relation")) {

    }

    if (properties.contains("*") || properties.contains("dc:rights")) {

    }

    // properties optional of musicTrack

    if (properties.contains("*") || properties.contains("upnp:artist")) {
        QDomElement upnpArtist = xml->createElement("upnp:artist");
        upnpArtist.appendChild(xml->createTextNode(metaDataPerformer()));
        xml_obj.appendChild(upnpArtist);
    }

    if (properties.contains("*") || properties.contains("upnp:album")) {
        QDomElement upnpAlbum = xml->createElement("upnp:album");
        upnpAlbum.appendChild(xml->createTextNode(metaDataAlbum()));
        xml_obj.appendChild(upnpAlbum);
    }

    if (properties.contains("*") || properties.contains("upnp:originalTrackNumber")) {
        QDomElement upnpTrackNumber = xml->createElement("upnp:originalTrackNumber");
        upnpTrackNumber.appendChild(xml->createTextNode(QString("%1").arg(metaDataTrackPosition())));
        xml_obj.appendChild(upnpTrackNumber);
    }

    if (properties.contains("*") || properties.contains("upnp:playlist")) {

    }

    if (properties.contains("*") || properties.contains("upnp:storageMedium")) {

    }

    if (properties.contains("*") || properties.contains("dc:contributor")) {
        QDomElement upnpCreator = xml->createElement("dc:contributor");
        upnpCreator.appendChild(xml->createTextNode(metaDataPerformer()));
        xml_obj.appendChild(upnpCreator);
    }

    if (properties.contains("*") || properties.contains("dc:date")) {
        QDomElement upnpDate = xml->createElement("dc:date");
        upnpDate.appendChild(xml->createTextNode(metaDataLastModifiedDate()));
        xml_obj.appendChild(upnpDate);
    }

    // properties optional of musicAlbum

    QUrl url(QString("http://%1:%2/get/thumbnail").arg(getHostUrl().host()).arg(getHostUrl().port()));
    QUrlQuery query;
    query.addQueryItem("id", getResourceId());
    url.setQuery(query);

    if (properties.contains("*") || properties.contains("upnp:albumArtURI")) {
        QImage picture = getAlbumArt();
        if (!picture.isNull()) {
            QDomElement upnpAlbumArtURI = xml->createElement("upnp:albumArtURI");
            upnpAlbumArtURI.setAttribute("xmlns:dlna", "urn:schemas-dlna-org:metadata-1-0/");
            upnpAlbumArtURI.setAttribute("dlna:profileID", "JPEG_TN");
            upnpAlbumArtURI.appendChild(xml->createTextNode(url.toEncoded()));
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
    if (properties.contains("*") || properties.contains("res@bitrate")) {
        // bitrate in bytes/sec
        res.setAttribute("bitrate", QString("%1").arg(qRound(double(bitrate())/8.0)));
    }

    if (properties.contains("*") || properties.contains("res@duration")) {
        res.setAttribute("duration", QString("%1").arg(duration.addSecs(getLengthInSeconds()).toString()));
    }

    if (properties.contains("*") || properties.contains("res@sampleFrequency")) {
        res.setAttribute("sampleFrequency", QString("%1").arg(samplerate()));
    }

    if (properties.contains("*") || properties.contains("res@nrAudioChannels")) {
        res.setAttribute("nrAudioChannels", QString("%1").arg(channelCount()));
    }

    if (properties.contains("*") || properties.contains("res@size")) {
        // size in bytes
        res.setAttribute("size", QString("%1").arg(size()));
    }

    QUrl content_url = QUrl(QString("http://%1:%2/get/content").arg(getHostUrl().host()).arg(getHostUrl().port()));
    QUrlQuery content_query;
    content_query.addQueryItem("id", getResourceId());
    content_url.setQuery(content_query);
    res.appendChild(xml->createTextNode(content_url.toEncoded()));

    xml_obj.appendChild(res);

    return xml_obj;
}

QFfmpegTranscoding *DlnaMusicTrack::getTranscodeProcess()
{
    auto  transcodeProcess = new QFfmpegTranscoding();
    transcodeProcess->setFormat(format());
    transcodeProcess->setBitrate(bitrate());
//    transcodeProcess->setVolumeInfo(volumeInfo());
    transcodeProcess->setUrl(getSystemName());

    connect(transcodeProcess, SIGNAL(openedSignal()), transcodeProcess, SLOT(startDemux()));

    return transcodeProcess;
}

Device *DlnaMusicTrack::getOriginalStreaming()
{
    return new StreamingFile(getSystemName());
}

QString DlnaMusicTrack::sourceMimeType() const
{
    QString format = metaDataFormat();
    if (format == "mp3" || format == "mp2" || format == "mp1")
        return AUDIO_MP3_TYPEMIME;

    if (format == "aac" || format == "alac")
        return AUDIO_MP4_TYPEMIME;

    if (format == "pcm_s16le")
        return "audio/wav";

    if (format == "pcm_s16be")
        return QString("%1;rate=%2;channels=%3").arg(AUDIO_LPCM_TYPEMIME).arg(samplerate()).arg(channelCount());

    qCritical() << QString("Unable to define mimeType of DlnaMusicTrack: %1 from format <%2>").arg(getSystemName(), format);

    // returns unknown mimeType
    return UNKNOWN_AUDIO_TYPEMIME;
}
