#include "myapplication.h"

MyApplication::MyApplication(int &argc, char **argv):
    Application(argc, argv),
    netManager(),
    m_upnp(0),
    m_messageModel(0)
{
    qRegisterMetaType<QHostAddress>("QHostAddress");
    qRegisterMetaType<SsdpMessage>("SsdpMessage");
    qRegisterMetaType<ListItem*>("CListItem");

    m_messageModel = new ListModel(new MessageItem, this);

    // start event for discovering, emit 3 times
    int eventDiscover = startTimer(2000);
    if (eventDiscover > 0)
    {
        setProperty("discover_event", eventDiscover);
        setProperty("discover_counter", 3);
    }
    else
    {
        qCritical() << "unable to start discover event";
    }

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

void MyApplication::timerEvent(QTimerEvent *event)
{
    if (event->type() == QTimerEvent::Timer && event->timerId() == property("discover_event"))
    {
        int counter = property("discover_counter").toInt();
        if (counter < 1)
        {
            setProperty("discover_event", QVariant::Invalid);
            setProperty("discover_counter", QVariant::Invalid);
            killTimer(event->timerId());
        }
        else
        {
            setProperty("discover_counter", --counter);
            if (m_upnp)
                m_upnp->sendDiscover(UpnpRootDevice::UPNP_ROOTDEVICE);
        }
    }
    else
    {
        qCritical() << "invalid event" << event->timerId();
    }
}
