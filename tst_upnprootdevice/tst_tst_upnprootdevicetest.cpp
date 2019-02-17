#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include "upnpcontrolpoint.h"

class Tst_upnprootdeviceTest : public QObject
{
    Q_OBJECT

public:
    Tst_upnprootdeviceTest() = default;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void test_get_device_description();

private:
    UpnpControlPoint *m_upnp = Q_NULLPTR;
    UpnpRootDevice *m_root = Q_NULLPTR;
    quint16 UPNP_PORT = 0;
    quint16 EVENT_PORT = 0;
};

void Tst_upnprootdeviceTest::initTestCase()
{
    UPNP_PORT = 6100;
    EVENT_PORT = 6000;

    m_upnp = new UpnpControlPoint(EVENT_PORT);

    m_root = new UpnpRootDevice(m_upnp->macAddress(), QString());

    QUrl tmp(QString("http://%1:%2").arg(m_upnp->host().toString()).arg(UPNP_PORT));
    m_root->setUrl(tmp.resolved(QString("/description/fetch")));

    auto deviceDescription = new UpnpRootDeviceDescription();
    deviceDescription->setDeviceAttribute("deviceType", "urn:schemas-upnp-org:device:MediaServer:1");
    deviceDescription->setDeviceAttribute("friendlyName", "QT Media Server");
    deviceDescription->setDeviceAttribute("manufacturer", "G HIMBERT");
    deviceDescription->setDeviceAttribute("modelDescription", "UPnP/AV 1.0 Compliant Media Server");
    deviceDescription->setDeviceAttribute("modelName", "QMS");
    deviceDescription->setDeviceAttribute("modelNumber", "01");
    deviceDescription->setDeviceAttribute("presentationURL", QString("http://%1:%2/console/index.html").arg(m_root->url().host()).arg(m_root->url().port()));
    deviceDescription->addIcon("image/png", 512, 512, 24, "/images/icon-512.png");
    deviceDescription->addIcon("image/png", 256, 256, 24, "/images/icon-256.png");
    deviceDescription->addIcon("image/png", 128, 128, 24, "/images/icon-128.png");
    deviceDescription->addIcon("image/jpeg", 128, 128, 24, "/images/icon-128.jpg");
    deviceDescription->addIcon("image/png", 120, 120, 24, "/images/icon-120.png");
    deviceDescription->addIcon("image/jpeg", 120, 120, 24, "/images/icon-120.jpg");
    deviceDescription->addIcon("image/png", 48, 48, 24, "/images/icon-48.png");
    deviceDescription->addIcon("image/jpeg", 48, 48, 24, "/images/icon-48.jpg");
    deviceDescription->addIcon("image/png", 32, 32, 24, "/images/icon-32.png");
    deviceDescription->addIcon("image/png", 16, 16, 24, "/images/icon-16.png");

    deviceDescription->setDeviceAttribute("UDN", QString("uuid:%1").arg(m_root->id()));
    m_root->setDescription(deviceDescription);

    m_root->setServerName(QString("%1/%2 UPnP/%3 QMS/1.0").arg(QSysInfo::productType(), QSysInfo::productVersion(), m_root->version()));

    m_upnp->addLocalRootDevice(m_root);
    m_root->startServer();
}

void Tst_upnprootdeviceTest::cleanupTestCase()
{
    if (m_root)
    {
        m_root->deleteLater();
    }

    if (m_upnp)
    {
        m_upnp->deleteLater();
    }
}

void Tst_upnprootdeviceTest::test_get_device_description()
{
    QVERIFY(m_upnp != Q_NULLPTR);

    QUrl url(QString("http://%1:%2/description/fetch").arg(m_upnp->host().toString()).arg(UPNP_PORT));
    //QUrl url(QString("http://192.168.1.6:8080/MediaRenderer/desc.xml"));
    //QUrl url(QString("http://192.168.1.18:52235/dmr/SamsungMRDesc.xml"));

    QNetworkRequest request(url);
    QNetworkReply *reply = MyNetwork::manager().get(request);

    int timeout = 10;
    while (timeout>0 && !reply->isFinished())
    {
        timeout--;
        QTest::qWait(1000);
    }

    QByteArray data = reply->readAll();
    QVERIFY(data.size() > 0);

//    QXmlSchema schema;
//    QFile fd("/Users/doudou/workspaceQT/Upnp/UpnpLibrary/xml schema/upnp_device.xsd");

//    QCOMPARE(fd.open(QFile::ReadOnly), true);
//    QCOMPARE(schema.load(fd.readAll()), true);
//    fd.close();

//    QXmlSchemaValidator validator(schema);
//    QCOMPARE(validator.validate(data), true);

    reply->deleteLater();}

QTEST_MAIN(Tst_upnprootdeviceTest)

#include "tst_tst_upnprootdevicetest.moc"
