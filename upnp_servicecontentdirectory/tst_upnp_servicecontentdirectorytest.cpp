#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include <QtXmlPatterns>
#include "upnpcontrolpoint.h"
#include "Services/servicecontentdirectory.h"

class Upnp_servicecontentdirectoryTest : public QObject
{
    Q_OBJECT

public:
    Upnp_servicecontentdirectoryTest();

private Q_SLOTS:
    void actionXmlAnswer(const QString &data);
    void errorRaised(const UpnpError &error);
    void initTestCase();
    void cleanupTestCase();

    void test_invalid_action();
    void test_get_service_description();
    void test_get_searchCapabilities();
    void test_get_sortCapabilities();
    void test_get_systemUpdateId();
    void test_browse();

private:
    void initRootDevice();
    void initContentDirectory();

private:
    QNetworkAccessManager nam;
    UpnpControlPoint *m_upnp = Q_NULLPTR;
    UpnpRootDevice *m_root = Q_NULLPTR;
    ServiceContentDirectory *m_contentDirectory = Q_NULLPTR;
    int UPNP_PORT = -1;
    int EVENT_PORT = -1;
    QString m_XmlActionAnswer;
    UpnpError m_error;
};

Upnp_servicecontentdirectoryTest::Upnp_servicecontentdirectoryTest()
{
}

void Upnp_servicecontentdirectoryTest::initTestCase()
{
    UPNP_PORT = 6100;
    EVENT_PORT = 6000;

    m_upnp = new UpnpControlPoint(EVENT_PORT);
    m_upnp->setNetworkManager(&nam);

    initRootDevice();

    initContentDirectory();

    m_upnp->addLocalRootDevice(m_root);
    m_root->startServer();
}

void Upnp_servicecontentdirectoryTest::cleanupTestCase()
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

void Upnp_servicecontentdirectoryTest::initRootDevice()
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

void Upnp_servicecontentdirectoryTest::initContentDirectory()
{
    if (m_root)
    {
        if (!m_contentDirectory)
        {
            m_contentDirectory = new ServiceContentDirectory(m_root);

            connect(m_contentDirectory, SIGNAL(actionXmlAnswer(QString)), this, SLOT(actionXmlAnswer(QString)));
            connect(m_contentDirectory, SIGNAL(errorOccured(UpnpError)), this, SLOT(errorRaised(UpnpError)));

            m_root->addService(m_contentDirectory);
        }
        else
        {
            qCritical() << "content directory already initialized.";
        }
    }
    else
    {
        qCritical() << "root device not initialized, cannot initialize content directory.";
    }
}

void Upnp_servicecontentdirectoryTest::actionXmlAnswer(const QString &data)
{
    m_XmlActionAnswer = data;
}

void Upnp_servicecontentdirectoryTest::errorRaised(const UpnpError &error)
{
    m_error = error;
}

void Upnp_servicecontentdirectoryTest::test_invalid_action()
{
    QString actionName = "GetProtocol";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    SoapAction action(m_contentDirectory->serviceType(), actionName);
    m_contentDirectory->runAction(action);

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


void Upnp_servicecontentdirectoryTest::test_get_service_description()
{
    QVERIFY(m_upnp != Q_NULLPTR);

    QUrl url(QString("http://%1:%2/UPnP_AV_ContentDirectory_1.0.xml").arg(m_upnp->host().toString()).arg(UPNP_PORT));

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

void Upnp_servicecontentdirectoryTest::test_get_searchCapabilities()
{
    QString actionName = "GetSearchCapabilities";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();
    m_contentDirectory->runAction(actionName);

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
    QCOMPARE(answer.serviceType(), m_contentDirectory->serviceType());
    QCOMPARE(answer.arguments().size(), 1);
    QCOMPARE(answer.arguments().at(0), "SearchCaps");
    QCOMPARE(answer.value("SearchCaps"), "");

    QCOMPARE(m_error.netError(), QNetworkReply::NoError);
    QCOMPARE(m_error.code(), -5);
    QCOMPARE(m_error.description(), "");
    QCOMPARE(m_error.faultCode(), "");
    QCOMPARE(m_error.faultString(), "");
}

void Upnp_servicecontentdirectoryTest::test_get_sortCapabilities()
{
    QString actionName = "GetSortCapabilities";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();
    m_contentDirectory->runAction(actionName);

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
    QCOMPARE(answer.serviceType(), m_contentDirectory->serviceType());
    QCOMPARE(answer.arguments().size(), 1);
    QCOMPARE(answer.arguments().at(0), "SortCaps");
    QCOMPARE(answer.value("SortCaps"), "");

    QCOMPARE(m_error.netError(), QNetworkReply::NoError);
    QCOMPARE(m_error.code(), -5);
    QCOMPARE(m_error.description(), "");
    QCOMPARE(m_error.faultCode(), "");
    QCOMPARE(m_error.faultString(), "");
}

void Upnp_servicecontentdirectoryTest::test_get_systemUpdateId()
{
    QString actionName = "GetSystemUpdateID";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();
    m_contentDirectory->runAction(actionName);

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
    QCOMPARE(answer.serviceType(), m_contentDirectory->serviceType());
    QCOMPARE(answer.arguments().size(), 1);
    QCOMPARE(answer.arguments().at(0), "Id");
    QCOMPARE(answer.value("Id"), "");

    QCOMPARE(m_error.netError(), QNetworkReply::NoError);
    QCOMPARE(m_error.code(), -5);
    QCOMPARE(m_error.description(), "");
    QCOMPARE(m_error.faultCode(), "");
    QCOMPARE(m_error.faultString(), "");
}

void Upnp_servicecontentdirectoryTest::test_browse()
{
    QString actionName = "Browse";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    QVariantMap arguments;
    arguments["ObjectID"] = "0";
    arguments["BrowseFlag"] = "";
    arguments["Filter"] = "";
    arguments["StartingIndex"] = "0";
    arguments["RequestedCount"] = "0";
    arguments["SortCriteria"] = "";

    m_XmlActionAnswer.clear();
    m_error = UpnpError();
    m_contentDirectory->runAction(actionName, arguments);

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
    QCOMPARE(answer.serviceType(), m_contentDirectory->serviceType());
    QCOMPARE(answer.arguments().size(), 1);
    QCOMPARE(answer.arguments().at(0), "Id");
    QCOMPARE(answer.value("Id"), "");

    QCOMPARE(m_error.netError(), QNetworkReply::NoError);
    QCOMPARE(m_error.code(), -5);
    QCOMPARE(m_error.description(), "");
    QCOMPARE(m_error.faultCode(), "");
    QCOMPARE(m_error.faultString(), "");
}

QTEST_MAIN(Upnp_servicecontentdirectoryTest)

#include "tst_upnp_servicecontentdirectorytest.moc"
