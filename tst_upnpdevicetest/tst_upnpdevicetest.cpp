#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include "upnprootdevice.h"
#include "upnprootdevicedescription.h"
#include <QFile>

class UpnpDeviceTest : public QObject
{
    Q_OBJECT

public:
    UpnpDeviceTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testUpnpRootDeviceFromDescription();
    void testUpnpRootDeviceFromXml();
    void testUpnpDeviceFromDescription();
    void testUpnpDeviceFromXml();

private:
    QNetworkAccessManager nam;
};

UpnpDeviceTest::UpnpDeviceTest()
{
}

void UpnpDeviceTest::initTestCase()
{
}

void UpnpDeviceTest::cleanupTestCase()
{
}

void UpnpDeviceTest::testUpnpRootDeviceFromDescription()
{
    UpnpRootDeviceDescription *deviceDescription = new UpnpRootDeviceDescription();
    QCOMPARE(deviceDescription->deviceAttribute("deviceType"), "");
    QCOMPARE(deviceDescription->deviceAttribute("friendlyName"), "");
    QCOMPARE(deviceDescription->deviceAttribute("manufacturer"), "");
    QCOMPARE(deviceDescription->deviceAttribute("modelDescription"), "");
    QCOMPARE(deviceDescription->deviceAttribute("modelName"), "");
    QCOMPARE(deviceDescription->deviceAttribute("modelNumber"), "");
    QCOMPARE(deviceDescription->deviceAttribute("UDN"), "");

    deviceDescription->setConfigId("1");

    deviceDescription->setVersion(2, 1);
    QCOMPARE(deviceDescription->version(), QString("2.1"));

    deviceDescription->setDeviceAttribute("deviceType", "urn:schemas-upnp-org:device:MediaServer:1");
    deviceDescription->setDeviceAttribute("friendlyName", "QT Media Server");
    deviceDescription->setDeviceAttribute("manufacturer", "G HIMBERT");
    deviceDescription->setDeviceAttribute("modelDescription", "UPnP/AV 1.0 Compliant Media Server");
    deviceDescription->setDeviceAttribute("modelName", "QMS");
    deviceDescription->setDeviceAttribute("modelNumber", "01");
    deviceDescription->setDeviceAttribute("UDN", "uuid");
    deviceDescription->setDeviceAttribute("presentationURL", "http://[host]:[port]/console/index.html");

    deviceDescription->addIcon("image/png", 32, 32, 24, "/images/icon-32.png");
    deviceDescription->addIcon("image/png", 16, 16, 24, "/images/icon-16.png");

    qDebug() << deviceDescription->stringDescription();

    QCOMPARE(deviceDescription->deviceAttribute("deviceType"), "urn:schemas-upnp-org:device:MediaServer:1");
    QCOMPARE(deviceDescription->deviceAttribute("friendlyName"), "QT Media Server");
    QCOMPARE(deviceDescription->deviceAttribute("manufacturer"), "G HIMBERT");
    QCOMPARE(deviceDescription->deviceAttribute("modelDescription"), "UPnP/AV 1.0 Compliant Media Server");
    QCOMPARE(deviceDescription->deviceAttribute("modelName"), "QMS");
    QCOMPARE(deviceDescription->deviceAttribute("modelNumber"), "01");
    QCOMPARE(deviceDescription->deviceAttribute("UDN"), "uuid");

    QCOMPARE(deviceDescription->xmlDescription().tagName(), QString("root"));
    QCOMPARE(deviceDescription->xmlDescription().childNodes().size(), 2);

    QCOMPARE(deviceDescription->xmlDescription().childNodes().at(0).toElement().tagName(), "specVersion");
    QCOMPARE(deviceDescription->xmlDescription().childNodes().at(0).childNodes().size(), 2);
    QCOMPARE(deviceDescription->xmlDescription().childNodes().at(0).childNodes().at(0).toElement().tagName(), "major");
    QCOMPARE(deviceDescription->xmlDescription().childNodes().at(0).childNodes().at(1).toElement().tagName(), "minor");

    QCOMPARE(deviceDescription->xmlDescription().childNodes().at(1).toElement().tagName(), "device");
    QCOMPARE(deviceDescription->xmlDescription().childNodes().at(1).childNodes().size(), 15);

    UpnpRootDevice device(&nam, "uuid-1-2", Q_NULLPTR);
    device.setDescription(deviceDescription);
    QCOMPARE(device.version(), QString("2.1"));
    QCOMPARE(device.deviceType(), QString("urn:schemas-upnp-org:device:MediaServer:1"));
    QCOMPARE(device.friendlyName(), QString("QT Media Server"));
    QCOMPARE(device.uuid(), QString("uuid-1-2"));
    QCOMPARE(device.id(), QString("uuid-1-2"));
    QCOMPARE(device.data(UpnpRootDevice::PresentationUrlRole), QString("http://[host]:[port]/console/index.html"));

    QCOMPARE(device.available(), false);
    QCOMPARE(device.status(), UpnpObject::Null);

    UpnpService *service = device.getService("urn:upnp-org:serviceId:ContentDirectory");
    QVERIFY(service != Q_NULLPTR);
}

void UpnpDeviceTest::testUpnpRootDeviceFromXml()
{
    QFile xml_file(":/PMS.xml");
    QCOMPARE(xml_file.open(QIODevice::ReadOnly), true);
    UpnpRootDeviceDescription *description = new UpnpRootDeviceDescription();
    description->setContent(xml_file.readAll());

    QCOMPARE(description->xmlDescription().tagName(), QString("root"));

    QCOMPARE(description->version(), QString("1.0"));
    QCOMPARE(description->deviceAttribute("UDN"), QString("[uuid]"));
    QCOMPARE(description->iconUrl(), QString("/images/icon-512.png"));

    QCOMPARE(description->xmlDescription().childNodes().at(1).toElement().tagName(), "device");
    QCOMPARE(description->xmlDescription().childNodes().at(1).childNodes().size(), 14);
}

void UpnpDeviceTest::testUpnpDeviceFromDescription()
{
    UpnpDeviceDescription *deviceDescription = new UpnpDeviceDescription();
    QCOMPARE(deviceDescription->deviceAttribute("deviceType"), "");
    QCOMPARE(deviceDescription->deviceAttribute("friendlyName"), "");
    QCOMPARE(deviceDescription->deviceAttribute("manufacturer"), "");
    QCOMPARE(deviceDescription->deviceAttribute("modelDescription"), "");
    QCOMPARE(deviceDescription->deviceAttribute("modelName"), "");
    QCOMPARE(deviceDescription->deviceAttribute("modelNumber"), "");
    QCOMPARE(deviceDescription->deviceAttribute("UDN"), "");

    deviceDescription->setDeviceAttribute("deviceType", "urn:schemas-upnp-org:device:MediaServer:1");
    deviceDescription->setDeviceAttribute("friendlyName", "QT Media Server");
    deviceDescription->setDeviceAttribute("manufacturer", "G HIMBERT");
    deviceDescription->setDeviceAttribute("modelDescription", "UPnP/AV 1.0 Compliant Media Server");
    deviceDescription->setDeviceAttribute("modelName", "QMS");
    deviceDescription->setDeviceAttribute("modelNumber", "01");
    deviceDescription->setDeviceAttribute("UDN", "uuid");
    deviceDescription->setDeviceAttribute("presentationURL", "http://[host]:[port]/console/index.html");

    deviceDescription->addIcon("image/png", 32, 32, 24, "/images/icon-32.png");
    deviceDescription->addIcon("image/png", 16, 16, 24, "/images/icon-16.png");

    qDebug() << deviceDescription->stringDescription();

    QCOMPARE(deviceDescription->deviceAttribute("deviceType"), "urn:schemas-upnp-org:device:MediaServer:1");
    QCOMPARE(deviceDescription->deviceAttribute("friendlyName"), "QT Media Server");
    QCOMPARE(deviceDescription->deviceAttribute("manufacturer"), "G HIMBERT");
    QCOMPARE(deviceDescription->deviceAttribute("modelDescription"), "UPnP/AV 1.0 Compliant Media Server");
    QCOMPARE(deviceDescription->deviceAttribute("modelName"), "QMS");
    QCOMPARE(deviceDescription->deviceAttribute("modelNumber"), "01");
    QCOMPARE(deviceDescription->deviceAttribute("UDN"), "uuid");

    QCOMPARE(deviceDescription->xmlDescription().tagName(), QString("device"));
    QCOMPARE(deviceDescription->xmlDescription().childNodes().size(), 15);

    UpnpDevice device("uuid-1-4", Q_NULLPTR);
    device.setDescription(deviceDescription);
    QCOMPARE(device.deviceType(), QString("urn:schemas-upnp-org:device:MediaServer:1"));
    QCOMPARE(device.friendlyName(), QString("QT Media Server"));
    QCOMPARE(device.uuid(), QString("uuid-1-4"));
    QCOMPARE(device.id(), QString("uuid-1-4"));
    QCOMPARE(device.data(UpnpDevice::PresentationUrlRole), QString("http://[host]:[port]/console/index.html"));


    QCOMPARE(device.valueFromDescription("deviceType"), "urn:schemas-upnp-org:device:MediaServer:1");
    QCOMPARE(device.available(), false);
    QCOMPARE(device.status(), UpnpObject::Null);
}

void UpnpDeviceTest::testUpnpDeviceFromXml()
{
    QFile xml_file(":/PMS2.xml");
    QCOMPARE(xml_file.open(QIODevice::ReadOnly), true);
    UpnpDeviceDescription *description = new UpnpDeviceDescription();
    description->setContent(xml_file.readAll());

    QCOMPARE(description->xmlDescription().tagName(), QString("device"));
    QCOMPARE(description->xmlDescription().childNodes().size(), 14);

    QCOMPARE(description->deviceAttribute("UDN"), QString("uuid:dc53b523-0f0a-396d-b3dc-249276216f3b"));
    QCOMPARE(description->iconUrl(), QString(""));
}

QTEST_MAIN(UpnpDeviceTest)

#include "tst_upnpdevicetest.moc"
