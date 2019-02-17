#include "myapplication.h"

MyApplication::MyApplication(int &argc, char **argv):
    Application(argc, argv),
    m_upnp(Q_NULLPTR),
    m_messageModel(Q_NULLPTR),
    m_mDnsBrowser(&m_mDnsServer, QMdnsEngine::MdnsBrowseType, &m_mDnsCache)
{
    qRegisterMetaType<QHostAddress>("QHostAddress");
    qRegisterMetaType<SsdpMessage>("SsdpMessage");
    qRegisterMetaType<ListItem*>("CListItem");
    qRegisterMetaType<UpnpActionReply*>("UpnpActionReply");

    m_messageModel = new ListModel(new MessageItem, this);

    m_upnp = new UpnpControlPoint(this);
    connect(m_upnp, &UpnpControlPoint::messageReceived, this, &MyApplication::messageReceived);
    m_upnp->startDiscover();

    connect(&m_mDnsBrowser, &QMdnsEngine::Browser::serviceAdded, this, &MyApplication::mDnsServiceAdded);
}

void MyApplication::messageReceived(const QHostAddress &host, const quint16 &port, const SsdpMessage &message)
{
    auto item = new MessageItem(m_messageModel);
    item->setHostAddress(host);
    item->setPort(port);
    item->setMessage(message);
    m_messageModel->insertRow(0, item);
}

void MyApplication::mDnsServiceAdded(const QMdnsEngine::Service &service)
{
    qWarning() << "mDNS service discovered" << service.name() << service.type() << service.hostname() << service.port() << service.attributes();
}
