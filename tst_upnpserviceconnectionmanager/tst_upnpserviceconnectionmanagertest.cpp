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
    void errorRaised(const UpnpError &error);
    void initTestCase();
    void cleanupTestCase();

    void test_invalid_action();
    void test_get_service_description();
    void test_get_protocolInfo();
    void test_get_currentConnectionIds();
    void test_get_currentConnectionInfo();

private:
    void initRootDevice();
    void initConnectionManager();
    QStringList format();

private:
    QNetworkAccessManager nam;
    UpnpControlPoint *m_upnp = Q_NULLPTR;
    UpnpRootDevice *m_root = Q_NULLPTR;
    ServiceConnectionManager *m_connectionManager = Q_NULLPTR;
    int UPNP_PORT = -1;
    int EVENT_PORT = -1;
    QString m_XmlActionAnswer;
    UpnpError m_error;
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

    initRootDevice();

    initConnectionManager();

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

void UpnpserviceconnectionmanagerTest::initRootDevice()
{
    if (m_root == Q_NULLPTR)
    {
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
    }
    else
    {
        qCritical() << "root device already initialized.";
    }
}

QStringList UpnpserviceconnectionmanagerTest::format()
{
    QStringList res;
    res << "http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM";
    res << "http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_MED";
    res << "http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG";

    res << "http-get:*:audio/mpeg:DLNA.ORG_PN=MP3";
    res << "http-get:*:audio/L16:DLNA.ORG_PN=LPCM";

    res << "http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_24_AC3_ISO;SONY.COM_PN=AVC_TS_HD_24_AC3_ISO";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_24_AC3;SONY.COM_PN=AVC_TS_HD_24_AC3";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_24_AC3_T;SONY.COM_PN=AVC_TS_HD_24_AC3_T";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_PS_PAL";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_PS_NTSC";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_50_L2_T";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_60_L2_T";
    res << "http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_EU_ISO";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU_T";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_50_AC3_T";
    res << "http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_50_L2_ISO;SONY.COM_PN=HD2_50_ISO";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_60_AC3_T";
    res << "http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_60_L2_ISO;SONY.COM_PN=HD2_60_ISO";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_50_L2_T;SONY.COM_PN=HD2_50_T";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_60_L2_T;SONY.COM_PN=HD2_60_T";
    res << "http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_50_AC3_ISO;SONY.COM_PN=AVC_TS_HD_50_AC3_ISO";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3;SONY.COM_PN=AVC_TS_HD_50_AC3";
    res << "http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_60_AC3_ISO;SONY.COM_PN=AVC_TS_HD_60_AC3_ISO";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3;SONY.COM_PN=AVC_TS_HD_60_AC3";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3_T;SONY.COM_PN=AVC_TS_HD_50_AC3_T";
    res << "http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3_T;SONY.COM_PN=AVC_TS_HD_60_AC3_T";
    res << "http-get:*:video/x-mp2t-mphl-188:*";
    res << "http-get:*:*:*,http-get:*:video/*:*";
    res << "http-get:*:audio/*:*";
    res << "http-get:*:image/*:*";
    return res;
}

void UpnpserviceconnectionmanagerTest::initConnectionManager()
{
    if (m_root)
    {
        if (!m_connectionManager)
        {
            m_connectionManager = new ServiceConnectionManager(m_root);

            connect(m_connectionManager, SIGNAL(actionXmlAnswer(QString)), this, SLOT(actionXmlAnswer(QString)));
            connect(m_connectionManager, SIGNAL(errorOccured(UpnpError)), this, SLOT(errorRaised(UpnpError)));

            m_connectionManager->updateStateVariable("SourceProtocolInfo", format().join(","));

            m_root->addService(m_connectionManager);
        }
        else
        {
            qCritical() << "connection manager already initialized.";
        }
    }
    else
    {
        qCritical() << "root device not initialized, cannot initialize connection manager.";
    }
}

void UpnpserviceconnectionmanagerTest::actionXmlAnswer(const QString &data)
{
    m_XmlActionAnswer = data;
}

void UpnpserviceconnectionmanagerTest::errorRaised(const UpnpError &error)
{
    m_error = error;
}

void UpnpserviceconnectionmanagerTest::test_invalid_action()
{
    QString actionName = "GetProtocol";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_connectionManager != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    SoapAction action(m_connectionManager->serviceType(), actionName);
    m_connectionManager->runAction(action);

    int timeout = 10;
    while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
    {
        timeout--;
        QTest::qWait(1000);
    }

    QVERIFY(m_XmlActionAnswer.size() == 0);

    QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
    QCOMPARE(m_error.code(), 401);
    QCOMPARE(m_error.description(), "Invalid Action");
    QCOMPARE(m_error.faultCode(), "s:Client");
    QCOMPARE(m_error.faultString(), "UPnPError");
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

    QVERIFY(m_connectionManager != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();
    m_connectionManager->runAction(actionName);

    int timeout = 10;
    while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
    {
        timeout--;
        QTest::qWait(1000);
    }

    QVERIFY(m_XmlActionAnswer.size() > 0);

    SoapActionResponse answer(m_XmlActionAnswer.toUtf8());
    QCOMPARE(answer.isValid(), true);
    QCOMPARE(answer.actionName(), actionName);
    QCOMPARE(answer.serviceType(), m_connectionManager->serviceType());
    QCOMPARE(answer.arguments().size(), 2);
    QCOMPARE(answer.arguments().at(0), "Source");
    QCOMPARE(answer.value("Source"), format().join(","));
    QCOMPARE(answer.arguments().at(1), "Sink");
    QCOMPARE(answer.value("Sink"), "");

    QCOMPARE(m_error.netError(), QNetworkReply::NoError);
    QCOMPARE(m_error.code(), -5);
    QCOMPARE(m_error.description(), "");
    QCOMPARE(m_error.faultCode(), "");
    QCOMPARE(m_error.faultString(), "");
}

void UpnpserviceconnectionmanagerTest::test_get_currentConnectionIds()
{
    QString actionName = "GetCurrentConnectionIDs";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_connectionManager != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();
    m_connectionManager->runAction(actionName);

    int timeout = 10;
    while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
    {
        timeout--;
        QTest::qWait(1000);
    }

    QVERIFY(m_XmlActionAnswer.size() > 0);

    SoapActionResponse answer(m_XmlActionAnswer.toUtf8());
    QCOMPARE(answer.isValid(), true);
    QCOMPARE(answer.actionName(), actionName);
    QCOMPARE(answer.serviceType(), m_connectionManager->serviceType());
    QCOMPARE(answer.arguments().size(), 1);
    QCOMPARE(answer.arguments().at(0), "ConnectionIDs");
    QCOMPARE(answer.value("ConnectionIDs"), "0");

    QCOMPARE(m_error.netError(), QNetworkReply::NoError);
    QCOMPARE(m_error.code(), -5);
    QCOMPARE(m_error.description(), "");
    QCOMPARE(m_error.faultCode(), "");
    QCOMPARE(m_error.faultString(), "");
}

void UpnpserviceconnectionmanagerTest::test_get_currentConnectionInfo()
{
    QString actionName = "GetCurrentConnectionInfo";
    QMap<QString, QVariant> arguments;
    arguments["ConnectionID"] = "0";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_connectionManager != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();
    m_connectionManager->runAction(actionName, arguments);

    int timeout = 10;
    while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
    {
        timeout--;
        QTest::qWait(1000);
    }

    QVERIFY(m_XmlActionAnswer.size() > 0);

    SoapActionResponse answer(m_XmlActionAnswer.toUtf8());
    QCOMPARE(answer.isValid(), true);
    QCOMPARE(answer.actionName(), actionName );
    QCOMPARE(answer.serviceType(), m_connectionManager->serviceType());
    QCOMPARE(answer.arguments().size(), 7);
    QCOMPARE(answer.arguments().at(0), "RcsID");
    QCOMPARE(answer.value("RcsID"), "0");
    QCOMPARE(answer.arguments().at(1), "AVTransportID");
    QCOMPARE(answer.value("AVTransportID"), "0");
    QCOMPARE(answer.arguments().at(2), "ProtocolInfo");
    QCOMPARE(answer.value("ProtocolInfo"), "");
    QCOMPARE(answer.arguments().at(3), "PeerConnectionManager");
    QCOMPARE(answer.value("PeerConnectionManager"), "");
    QCOMPARE(answer.arguments().at(4), "PeerConnectionID");
    QCOMPARE(answer.value("PeerConnectionID"), "-1");
    QCOMPARE(answer.arguments().at(5), "Direction");
    QCOMPARE(answer.value("Direction"), "Input");
    QCOMPARE(answer.arguments().at(6), "Status");
    QCOMPARE(answer.value("Status"), "OK");

    QCOMPARE(m_error.netError(), QNetworkReply::NoError);
    QCOMPARE(m_error.code(), -5);
    QCOMPARE(m_error.description(), "");
    QCOMPARE(m_error.faultCode(), "");
    QCOMPARE(m_error.faultString(), "");
}

QTEST_MAIN(UpnpserviceconnectionmanagerTest)

#include "tst_upnpserviceconnectionmanagertest.moc"
