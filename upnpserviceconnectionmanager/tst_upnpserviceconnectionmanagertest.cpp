#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include <QtXmlPatterns>
#include "upnpcontrolpoint.h"
#include "Services/serviceconnectionmanager.h"

class UpnpserviceconnectionmanagerTest : public QObject
{
    Q_OBJECT

public:
    UpnpserviceconnectionmanagerTest();

private Q_SLOTS:
    void actionXmlAnswer(const QString &data);
    void initTestCase();
    void cleanupTestCase();

    void test_get_service_description();
    void test_get_protocolInfo();
    void test_get_currentConnectionIds();
    void test_get_currentConnectionInfo();

private:
    QNetworkAccessManager nam;
    UpnpControlPoint *m_upnp = Q_NULLPTR;
    UpnpRootDevice *m_root = Q_NULLPTR;
    int UPNP_PORT = -1;
    int EVENT_PORT = -1;
    QString m_XmlActionAnswer;
};

UpnpserviceconnectionmanagerTest::UpnpserviceconnectionmanagerTest()
{
}

void UpnpserviceconnectionmanagerTest::initTestCase()
{
    UPNP_PORT = 6100;
    EVENT_PORT = 6000;

    m_upnp = new UpnpControlPoint(EVENT_PORT);
    m_upnp->setNetworkManager(&nam);

    m_root = new UpnpRootDevice(&nam, m_upnp->macAddress(), QString());

    QUrl tmp(QString("http://%1:%2").arg(m_upnp->host().toString()).arg(UPNP_PORT));
    m_root->setUrl(tmp.resolved(QString("/description/fetch")));

    UpnpRootDeviceDescription *deviceDescription = new UpnpRootDeviceDescription();
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

    m_root->setServerName(QString("%1/%2 UPnP/%3 QMS/1.0").arg(QSysInfo::productType()).arg(QSysInfo::productVersion()).arg(m_root->version()));



    ServiceConnectionManager *connection_manager = new ServiceConnectionManager(m_root);

    QStringList format;
    format << "http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM";
    format << "http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_MED";
    format << "http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG";

    format << "http-get:*:audio/mpeg:DLNA.ORG_PN=MP3";
    format << "http-get:*:audio/L16:DLNA.ORG_PN=LPCM";

    format << "http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_24_AC3_ISO;SONY.COM_PN=AVC_TS_HD_24_AC3_ISO";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_24_AC3;SONY.COM_PN=AVC_TS_HD_24_AC3";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_24_AC3_T;SONY.COM_PN=AVC_TS_HD_24_AC3_T";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_PS_PAL";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_PS_NTSC";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_50_L2_T";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_60_L2_T";
    format << "http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_EU_ISO";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU_T";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_50_AC3_T";
    format << "http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_50_L2_ISO;SONY.COM_PN=HD2_50_ISO";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_60_AC3_T";
    format << "http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_60_L2_ISO;SONY.COM_PN=HD2_60_ISO";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_50_L2_T;SONY.COM_PN=HD2_50_T";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_60_L2_T;SONY.COM_PN=HD2_60_T";
    format << "http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_50_AC3_ISO;SONY.COM_PN=AVC_TS_HD_50_AC3_ISO";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3;SONY.COM_PN=AVC_TS_HD_50_AC3";
    format << "http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_60_AC3_ISO;SONY.COM_PN=AVC_TS_HD_60_AC3_ISO";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3;SONY.COM_PN=AVC_TS_HD_60_AC3";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3_T;SONY.COM_PN=AVC_TS_HD_50_AC3_T";
    format << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3_T;SONY.COM_PN=AVC_TS_HD_60_AC3_T";
    format << "http-get:*:video/x-mp2t-mphl-188:*";
    format << "http-get:*:*:*,http-get:*:video/*:*";
    format << "http-get:*:audio/*:*";
    format << "http-get:*:image/*:*";
    connection_manager->updateStateVariable("SourceProtocolInfo", format.join(","));

    m_root->addService(connection_manager);

    m_upnp->addLocalRootDevice(m_root);
    m_root->startServer();
}

void UpnpserviceconnectionmanagerTest::cleanupTestCase()
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

void UpnpserviceconnectionmanagerTest::actionXmlAnswer(const QString &data)
{
    m_XmlActionAnswer = data;
}

void UpnpserviceconnectionmanagerTest::test_get_service_description()
{
    QVERIFY(m_upnp != Q_NULLPTR);

    QUrl url(QString("http://%1:%2/UPnP_AV_ConnectionManager_1.0.xml").arg(m_upnp->host().toString()).arg(UPNP_PORT));

    QNetworkRequest request(url);
    QNetworkReply *reply = nam.get(request);

    int timeout = 10;
    while (timeout>0 && !reply->isFinished())
    {
        timeout--;
        QTest::qWait(1000);
    }

    QByteArray data = reply->readAll();
    QVERIFY(data.size() > 0);

//    QXmlSchema schema;
//    QFile fd("/Users/doudou/workspaceQT/Upnp/UpnpLibrary/xml schema/upnp_service.xsd");

//    QCOMPARE(fd.open(QFile::ReadOnly), true);
//    QCOMPARE(schema.load(fd.readAll()), true);
//    fd.close();

//    QXmlSchemaValidator validator(schema);
//    QCOMPARE(validator.validate(data), true);

    reply->deleteLater();
}

void UpnpserviceconnectionmanagerTest::test_get_protocolInfo()
{
    QString actionName = "GetProtocolInfo";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    ServiceConnectionManager *service = (ServiceConnectionManager*)m_root->getService("urn:upnp-org:serviceId:ConnectionManager");
    connect(service, SIGNAL(actionXmlAnswer(QString)), this, SLOT(actionXmlAnswer(QString)));
    QVERIFY(service != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    service->runAction(actionName);

    int timeout = 10;
    while (timeout>0 && m_XmlActionAnswer.size()==0)
    {
        timeout--;
        QTest::qWait(1000);
    }

    QVERIFY(m_XmlActionAnswer.size() > 0);
}

void UpnpserviceconnectionmanagerTest::test_get_currentConnectionIds()
{
    QString actionName = "GetCurrentConnectionIDs";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    ServiceConnectionManager *service = (ServiceConnectionManager*)m_root->getService("urn:upnp-org:serviceId:ConnectionManager");
    connect(service, SIGNAL(actionXmlAnswer(QString)), this, SLOT(actionXmlAnswer(QString)));
    QVERIFY(service != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    service->runAction(actionName);

    int timeout = 10;
    while (timeout>0 && m_XmlActionAnswer.size()==0)
    {
        timeout--;
        QTest::qWait(1000);
    }

    QVERIFY(m_XmlActionAnswer.size() > 0);
}

void UpnpserviceconnectionmanagerTest::test_get_currentConnectionInfo()
{
    QString actionName = "GetCurrentConnectionInfo";
    QMap<QString, QVariant> arguments;
    arguments["ConnectionID"] = "0";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    ServiceConnectionManager *service = (ServiceConnectionManager*)m_root->getService("urn:upnp-org:serviceId:ConnectionManager");
    connect(service, SIGNAL(actionXmlAnswer(QString)), this, SLOT(actionXmlAnswer(QString)));
    QVERIFY(service != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    service->runAction(actionName, arguments);

    int timeout = 10;
    while (timeout>0 && m_XmlActionAnswer.size()==0)
    {
        timeout--;
        QTest::qWait(1000);
    }

    QVERIFY(m_XmlActionAnswer.size() > 0);
}

QTEST_MAIN(UpnpserviceconnectionmanagerTest)

#include "tst_upnpserviceconnectionmanagertest.moc"
