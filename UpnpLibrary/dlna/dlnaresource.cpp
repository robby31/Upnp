#include "dlnaresource.h"

qint64 DlnaResource::objectCounter = 0;

DlnaResource::DlnaResource(QObject *parent):
    QObject(parent),
    id(),
    dlnaParent(Q_NULLPTR),
    m_needRefresh(false),
    updateId(1)
{
    ++objectCounter;
    qRegisterMetaType<QList<DlnaResource*> >("QList<DlnaResource*>");

    connect(this, &DlnaResource::dlnaContentUpdated, this, &DlnaResource::contentUpdated);
}

DlnaResource::~DlnaResource() {
    --objectCounter;
}

QString DlnaResource::getResourceId() const {
    if (getId().isNull())
        return QString();

    if (getDlnaParent())
        return getDlnaParent()->getResourceId() + '$' + getId();

    return getId();
}

DlnaResource* DlnaResource::search(const QString &searchId, const QString &searchStr, const bool &refreshIfNeeded, QObject *parent)
{
    if (m_needRefresh && getResourceId() == searchId && (refreshIfNeeded or getChildrenSize() < 0))
        refreshContent();

    if (getResourceId() == searchId)
        return this;

    if (getResourceId().length() < searchId.length() && searchId.startsWith(getResourceId())) {

        int child_index = searchId.split("$").at(getResourceId().split("$").length()).toInt()-1;

        if ((child_index >= 0) && (child_index < getChildrenSize())) {
            DlnaResource *child = getChild(child_index, parent);
            if (child)
                return child->search(searchId, searchStr, refreshIfNeeded, parent);
        }
    }

    return Q_NULLPTR;
}

QList<DlnaResource*> DlnaResource::getDLNAResources(const QString &objectId, bool returnChildren, int start, int count, const QString &searchStr, QObject *parent) {
    QList<DlnaResource*> resources;
    DlnaResource* dlna = search(objectId, searchStr, !returnChildren, parent);
    if (dlna)
    {
        if (!returnChildren) {
            resources.append(dlna);
        } else {
            int nbChildren = dlna->getChildrenSize();
            if (count <= 0)
                count = nbChildren - start;  // return all children
            for (int i = start; i < start + count; i++) {
                if (i < nbChildren) {
                    DlnaResource* child = dlna->getChild(i, parent);
                    if (child)
                    {
                        if (child->m_needRefresh)
                            child->refreshContent();

                        resources.append(child);
                    }
                }
            }
        }
    }

    return resources;
}

QString DlnaResource::getDlnaParentId() const
{
    if (getDlnaParent())
        return getDlnaParent()->getResourceId();

    return "-1";
}

QString DlnaResource::getStringContentDirectory(const QStringList &properties)  {
    QDomDocument xml;
    xml.appendChild(getXmlContentDirectory(&xml, properties));

    QDomDocument res;
    QDomElement result = res.createElement("Result");
    result.appendChild(res.createTextNode(xml.toString().replace("\n", "")));
    res.appendChild(result);

    QString strRes = res.toString();

    // remove <Result> from beginning
    strRes.chop(10);

    // remove </Result> from ending
    strRes.remove(0, 8);

    return strRes;
}

QByteArray DlnaResource::getByteAlbumArt() const {
    QImage picture = getAlbumArt();

    if (!picture.isNull()) {
        QByteArray result;
        QBuffer buffer(&result);
        if (buffer.open(QIODevice::WriteOnly)) {
            if (picture.save(&buffer, "JPEG")) {
                buffer.close();
                return result;
            }
        }
    }
    return QByteArray();
}

void DlnaResource::updateXmlContentDirectory(QDomDocument *xml, QDomElement *xml_obj, QStringList properties) const {
    Q_UNUSED(properties);

    if (xml && xml_obj) {
        xml_obj->setAttribute("id", getResourceId());

        xml_obj->setAttribute("parentID", getDlnaParentId());

        QDomElement dcTitle = xml->createElement("dc:title");
        dcTitle.appendChild(xml->createTextNode(getDisplayName()));
        xml_obj->appendChild(dcTitle);

        QDomElement upnpClass = xml->createElement("upnp:class");
        upnpClass.appendChild(xml->createTextNode(getUpnpClass()));
        xml_obj->appendChild(upnpClass);

        xml_obj->setAttribute("restricted", "true");
    }
}

void DlnaResource::change_parent(QObject *old_parent, QObject *new_parent)
{
    if (new_parent && parent() == old_parent)
    {
        // set parent to null
        setParent(Q_NULLPTR);

        // change the thread
        moveToThread(new_parent->thread());
        connect(new_parent->thread(), SIGNAL(finished()), this, SLOT(deleteLater()));

        if (getDlnaParent())
            getDlnaParent()->change_parent(old_parent, new_parent);
    }
}

void DlnaResource::requestDlnaResources(QObject *sender, const QString &objectId, bool returnChildren, int start, int count, const QString &searchStr)
{
    QObject context;
    QList<DlnaResource*> res = getDLNAResources(objectId, returnChildren, start, count, searchStr, &context);

    for (int index=0;index<res.count();++index)
    {
        DlnaResource *item = res.at(index);
        if (item)
            item->change_parent(&context, sender);
    }

    emit dlnaResources(sender, res);
}

QUrl DlnaResource::getHostUrl() const
{
    if (!m_hostUrl.isEmpty())
        return m_hostUrl;

    if (dlnaParent)
        return dlnaParent->getHostUrl();

    qCritical() << "invalid host url" << m_hostUrl << this;
    return QString();
}

void DlnaResource::setHostUrl(const QUrl &url)
{
    m_hostUrl = url;
}

void DlnaResource::contentUpdated()
{
    ++updateId;
    m_needRefresh = false;
}
