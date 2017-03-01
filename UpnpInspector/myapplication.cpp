#include "myapplication.h"

MyApplication::MyApplication(int &argc, char **argv):
    Application(argc, argv),
    netManager(),
    m_upnp(0),
    m_messageModel(0),
    m_timerDiscover(3, 600000, this)
{
    qRegisterMetaType<QHostAddress>("QHostAddress");
    qRegisterMetaType<SsdpMessage>("SsdpMessage");
    qRegisterMetaType<ListItem*>("CListItem");

    m_messageModel = new ListModel(new MessageItem, this);

    connect(&m_timerDiscover, SIGNAL(timeout()), this, SLOT(advertiseSlot()));
    m_timerDiscover.start(2000);

    m_upnp = new UpnpControlPoint(this);
    connect(m_upnp, SIGNAL(messageReceived(QHostAddress,int,SsdpMessage)), this, SLOT(messageReceived(QHostAddress,int,SsdpMessage)));
    m_upnp->setNetworkManager(&netManager);
}

void MyApplication::messageReceived(const QHostAddress &host, const int &port, const SsdpMessage &message)
{
    MessageItem *item = new MessageItem(m_messageModel);
    item->setHostAddress(host);
    item->setPort(port);
    item->setMessage(message);
    m_messageModel->insertRow(0, item);
}

void MyApplication::advertiseSlot()
{
    if (m_upnp)
        m_upnp->sendDiscover(UpnpRootDevice::UPNP_ROOTDEVICE);
}
