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
    Upnp_servicecontentdirectoryTest() = default;

private Q_SLOTS:
    void actionFinished();
    void errorRaised(const UpnpError &error);

    void init();
    void cleanup();

    void test_invalid_action();
    void test_get_service_description();

    void test_get_searchCapabilities();
    void test_get_searchCapabilities_InvalidArgs();

    void test_get_sortCapabilities();
    void test_get_sortCapabilities_InvalidArgs();

    void test_get_systemUpdateId();
    void test_get_systemUpdateId_InvalidArgs();

    void test_browse_Root_MetaData();
    void test_browse_Root_DirectChildren();
    void test_browse_InvalidArgs();
    void test_browse_Invalid_objectID();
    void test_browse_Invalid_browseFlag();
    void test_browse_Invalid_filter();
    void test_browse_Invalid_startingIndex();
    void test_browse_Invalid_requestedCount();

private:
    void initRootDevice();
    void initContentDirectory();

private:
    UpnpControlPoint *m_upnp = Q_NULLPTR;
    UpnpRootDevice *m_root = Q_NULLPTR;
    ServiceContentDirectory *m_contentDirectory = Q_NULLPTR;
    quint16 UPNP_PORT = 0;
    quint16 EVENT_PORT = 0;
    QString m_XmlActionAnswer;
    UpnpError m_error;
};

void Upnp_servicecontentdirectoryTest::init()
{
    UPNP_PORT = 6100;
    EVENT_PORT = 6000;

    m_upnp = new UpnpControlPoint(EVENT_PORT);

    initRootDevice();

    initContentDirectory();

    m_upnp->addLocalRootDevice(m_root);
    m_root->startServer();
}

void Upnp_servicecontentdirectoryTest::cleanup()
{
    m_root = Q_NULLPTR;
    m_contentDirectory = Q_NULLPTR;
    delete m_upnp;

    DebugInfo::display_alive_objects();
    QCOMPARE(DebugInfo::count_alive_objects(), 0);
}

void Upnp_servicecontentdirectoryTest::initRootDevice()
{
    if (m_root == Q_NULLPTR)
    {
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
            m_contentDirectory = new ServiceContentDirectory(Q_NULLPTR, m_root);

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

void Upnp_servicecontentdirectoryTest::actionFinished()
{
    auto reply = qobject_cast<UpnpActionReply *>(sender());
    if (reply)
        m_XmlActionAnswer = reply->data();
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
    UpnpActionReply *reply = m_contentDirectory->runAction(action);
    QVERIFY(reply != Q_NULLPTR);
    connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
    connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

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

    UpnpActionReply *reply = m_contentDirectory->runAction(actionName);
    QVERIFY(reply != Q_NULLPTR);
    connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
    connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

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

void Upnp_servicecontentdirectoryTest::test_get_searchCapabilities_InvalidArgs()
{
    QString actionName = "GetSearchCapabilities";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    SoapAction action(m_contentDirectory->serviceType(), actionName);
    action.addArgument("args", "invalid");
    UpnpActionReply *reply = m_contentDirectory->runAction(action);
    QVERIFY(reply != Q_NULLPTR);
    connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
    connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

    int timeout = 10;
    while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
    {
        timeout--;
        QTest::qWait(1000);
    }

    QVERIFY(m_XmlActionAnswer.size() == 0);

    QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
    QCOMPARE(m_error.code(), 402);
    QCOMPARE(m_error.description(), "Invalid Args");
    QCOMPARE(m_error.faultCode(), "s:Client");
    QCOMPARE(m_error.faultString(), "UPnPError");
}

void Upnp_servicecontentdirectoryTest::test_get_sortCapabilities()
{
    QString actionName = "GetSortCapabilities";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    UpnpActionReply *reply = m_contentDirectory->runAction(actionName);
    QVERIFY(reply != Q_NULLPTR);
    connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
    connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

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

void Upnp_servicecontentdirectoryTest::test_get_sortCapabilities_InvalidArgs()
{
    QString actionName = "GetSortCapabilities";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    SoapAction action(m_contentDirectory->serviceType(), actionName);
    action.addArgument("args", "invalid");
    UpnpActionReply *reply = m_contentDirectory->runAction(action);
    QVERIFY(reply != Q_NULLPTR);
    connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
    connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

    int timeout = 10;
    while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
    {
        timeout--;
        QTest::qWait(1000);
    }

    QVERIFY(m_XmlActionAnswer.size() == 0);

    QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
    QCOMPARE(m_error.code(), 402);
    QCOMPARE(m_error.description(), "Invalid Args");
    QCOMPARE(m_error.faultCode(), "s:Client");
    QCOMPARE(m_error.faultString(), "UPnPError");
}

void Upnp_servicecontentdirectoryTest::test_get_systemUpdateId()
{
    QString actionName = "GetSystemUpdateID";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    UpnpActionReply *reply = m_contentDirectory->runAction(actionName);
    QVERIFY(reply != Q_NULLPTR);
    connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
    connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

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

void Upnp_servicecontentdirectoryTest::test_get_systemUpdateId_InvalidArgs()
{
    QString actionName = "GetSystemUpdateID";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    SoapAction action(m_contentDirectory->serviceType(), actionName);
    action.addArgument("args", "invalid");
    UpnpActionReply *reply = m_contentDirectory->runAction(action);
    QVERIFY(reply != Q_NULLPTR);
    connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
    connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

    int timeout = 10;
    while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
    {
        timeout--;
        QTest::qWait(1000);
    }

    QVERIFY(m_XmlActionAnswer.size() == 0);

    QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
    QCOMPARE(m_error.code(), 402);
    QCOMPARE(m_error.description(), "Invalid Args");
    QCOMPARE(m_error.faultCode(), "s:Client");
    QCOMPARE(m_error.faultString(), "UPnPError");
}


void Upnp_servicecontentdirectoryTest::test_browse_Root_MetaData()
{
    // BrowseMetadata of root object

    QString actionName = "Browse";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    QVariantMap arguments;
    arguments["ObjectID"] = "0";
    arguments["BrowseFlag"] = "BrowseMetadata";
    arguments["Filter"] = "*";
    arguments["StartingIndex"] = "0";
    arguments["RequestedCount"] = "0";
    arguments["SortCriteria"] = "";

    UpnpActionReply *reply = m_contentDirectory->runAction(actionName, arguments);
    QVERIFY(reply != Q_NULLPTR);
    connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
    connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

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
    QCOMPARE(answer.arguments().size(), 4);
    QCOMPARE(answer.arguments().at(0), "Result");

    qWarning() << answer.value("Result");

//    QXmlSchema schema;
//    QFile fd("/Users/doudou/workspaceQT/Upnp/UpnpLibrary/xml schema/didl-Lite.xsd");

//    QCOMPARE(fd.open(QFile::ReadOnly), true);
//    QCOMPARE(schema.load(fd.readAll()), true);
//    fd.close();

//    QXmlSchemaValidator validator(schema);
//    QCOMPARE(validator.validate(answer.value("Result").toUtf8()), true);

    QDomDocument result;
    QCOMPARE(result.setContent(answer.value("Result"), true), true);
    QCOMPARE(result.isNull(), false);

    QDomElement rootResult = result.firstChildElement();
    QCOMPARE(rootResult.isNull(), false);
    QCOMPARE(rootResult.childNodes().size(), 1);
    QCOMPARE(rootResult.toElement().tagName(), "DIDL-Lite");

    QDomElement container = rootResult.firstChildElement();
    QCOMPARE(container.tagName(), "container");

    QCOMPARE(container.attributes().size(), 4);
    QCOMPARE(container.attribute("id"), "0");
    QCOMPARE(container.attribute("childCount"), "6");
    QCOMPARE(container.attribute("parentID"), "-1");
    QCOMPARE(container.attribute("restricted"), "true");

    QCOMPARE(container.childNodes().size(), 2);
    QCOMPARE(container.childNodes().at(0).toElement().tagName(), "title");
    QCOMPARE(container.childNodes().at(0).toElement().firstChild().toText().data(), "root");
    QCOMPARE(container.childNodes().at(1).toElement().tagName(), "class");
    QCOMPARE(container.childNodes().at(1).toElement().firstChild().toText().data(), "object.container.storageFolder");

    QCOMPARE(answer.arguments().at(1), "NumberReturned");
    QCOMPARE(answer.value("NumberReturned"), "1");
    QCOMPARE(answer.arguments().at(2), "TotalMatches");
    QCOMPARE(answer.value("TotalMatches"), "1");
    QCOMPARE(answer.arguments().at(3), "UpdateID");
    QCOMPARE(answer.value("UpdateID"), "1");


    QCOMPARE(m_error.netError(), QNetworkReply::NoError);
    QCOMPARE(m_error.code(), -5);
    QCOMPARE(m_error.description(), "");
    QCOMPARE(m_error.faultCode(), "");
    QCOMPARE(m_error.faultString(), "");
}

void Upnp_servicecontentdirectoryTest::test_browse_Root_DirectChildren()
{
    // Browse direct children of root object

    QString actionName = "Browse";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    QVariantMap arguments;
    arguments["ObjectID"] = "0";
    arguments["BrowseFlag"] = "BrowseDirectChildren";
    arguments["Filter"] = "*";
    arguments["StartingIndex"] = "0";
    arguments["RequestedCount"] = "0";
    arguments["SortCriteria"] = "";

    UpnpActionReply *reply = m_contentDirectory->runAction(actionName, arguments);
    QVERIFY(reply != Q_NULLPTR);
    connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
    connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

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
    QCOMPARE(answer.arguments().size(), 4);
    QCOMPARE(answer.arguments().at(0), "Result");

    qWarning() << answer.value("Result");

//    QXmlSchema schema;
//    QFile fd("/Users/doudou/workspaceQT/Upnp/UpnpLibrary/xml schema/didl-Lite.xsd");

//    QCOMPARE(fd.open(QFile::ReadOnly), true);
//    QCOMPARE(schema.load(fd.readAll()), true);
//    fd.close();

//    QXmlSchemaValidator validator(schema);
//    QCOMPARE(validator.validate(answer.value("Result").toUtf8()), true);

    QDomDocument result;
    QCOMPARE(result.setContent(answer.value("Result"), true), true);
    QCOMPARE(result.isNull(), false);

    QDomElement rootResult = result.firstChildElement();
    QCOMPARE(rootResult.isNull(), false);
    QCOMPARE(rootResult.childNodes().size(), 6);
    QCOMPARE(rootResult.toElement().tagName(), "DIDL-Lite");

    QDomElement container = rootResult.firstChildElement();
    QCOMPARE(container.tagName(), "container");

    QCOMPARE(container.attributes().size(), 4);
    QCOMPARE(container.attribute("id"), "0$1");
    QCOMPARE(container.attribute("childCount"), "-1");
    QCOMPARE(container.attribute("parentID"), "0");
    QCOMPARE(container.attribute("restricted"), "true");

    QCOMPARE(container.childNodes().size(), 2);
    QCOMPARE(container.childNodes().at(0).toElement().tagName(), "title");
    QCOMPARE(container.childNodes().at(0).toElement().firstChild().toText().data(), "Recently Played");
    QCOMPARE(container.childNodes().at(1).toElement().tagName(), "class");
    QCOMPARE(container.childNodes().at(1).toElement().firstChild().toText().data(), "object.container.storageFolder");

    container = container.nextSiblingElement();
    QCOMPARE(container.tagName(), "container");

    QCOMPARE(container.attributes().size(), 4);
    QCOMPARE(container.attribute("id"), "0$2");
    QCOMPARE(container.attribute("childCount"), "-1");
    QCOMPARE(container.attribute("parentID"), "0");
    QCOMPARE(container.attribute("restricted"), "true");

    QCOMPARE(container.childNodes().size(), 2);
    QCOMPARE(container.childNodes().at(0).toElement().tagName(), "title");
    QCOMPARE(container.childNodes().at(0).toElement().firstChild().toText().data(), "Resume");
    QCOMPARE(container.childNodes().at(1).toElement().tagName(), "class");
    QCOMPARE(container.childNodes().at(1).toElement().firstChild().toText().data(), "object.container.storageFolder");

    container = container.nextSiblingElement();
    QCOMPARE(container.tagName(), "container");

    QCOMPARE(container.attributes().size(), 4);
    QCOMPARE(container.attribute("id"), "0$3");
    QCOMPARE(container.attribute("childCount"), "-1");
    QCOMPARE(container.attribute("parentID"), "0");
    QCOMPARE(container.attribute("restricted"), "true");

    QCOMPARE(container.childNodes().size(), 2);
    QCOMPARE(container.childNodes().at(0).toElement().tagName(), "title");
    QCOMPARE(container.childNodes().at(0).toElement().firstChild().toText().data(), "Last Added");
    QCOMPARE(container.childNodes().at(1).toElement().tagName(), "class");
    QCOMPARE(container.childNodes().at(1).toElement().firstChild().toText().data(), "object.container.storageFolder");

    container = container.nextSiblingElement();
    QCOMPARE(container.tagName(), "container");

    QCOMPARE(container.attributes().size(), 4);
    QCOMPARE(container.attribute("id"), "0$4");
    QCOMPARE(container.attribute("childCount"), "-1");
    QCOMPARE(container.attribute("parentID"), "0");
    QCOMPARE(container.attribute("restricted"), "true");

    QCOMPARE(container.childNodes().size(), 2);
    QCOMPARE(container.childNodes().at(0).toElement().tagName(), "title");
    QCOMPARE(container.childNodes().at(0).toElement().firstChild().toText().data(), "Favorites");
    QCOMPARE(container.childNodes().at(1).toElement().tagName(), "class");
    QCOMPARE(container.childNodes().at(1).toElement().firstChild().toText().data(), "object.container.storageFolder");

    container = container.nextSiblingElement();
    QCOMPARE(container.tagName(), "container");

    QCOMPARE(container.attributes().size(), 4);
    QCOMPARE(container.attribute("id"), "0$5");
    QCOMPARE(container.attribute("childCount"), "0");
    QCOMPARE(container.attribute("parentID"), "0");
    QCOMPARE(container.attribute("restricted"), "true");

    QCOMPARE(container.childNodes().size(), 2);
    QCOMPARE(container.childNodes().at(0).toElement().tagName(), "title");
    QCOMPARE(container.childNodes().at(0).toElement().firstChild().toText().data(), "Playlists");
    QCOMPARE(container.childNodes().at(1).toElement().tagName(), "class");
    QCOMPARE(container.childNodes().at(1).toElement().firstChild().toText().data(), "object.container.storageFolder");

    container = container.nextSiblingElement();
    QCOMPARE(container.tagName(), "container");

    QCOMPARE(container.attributes().size(), 4);
    QCOMPARE(container.attribute("id"), "0$6");
    QCOMPARE(container.attribute("childCount"), "0");
    QCOMPARE(container.attribute("parentID"), "0");
    QCOMPARE(container.attribute("restricted"), "true");

    QCOMPARE(container.childNodes().size(), 2);
    QCOMPARE(container.childNodes().at(0).toElement().tagName(), "title");
    QCOMPARE(container.childNodes().at(0).toElement().firstChild().toText().data(), "root");
    QCOMPARE(container.childNodes().at(1).toElement().tagName(), "class");
    QCOMPARE(container.childNodes().at(1).toElement().firstChild().toText().data(), "object.container.storageFolder");

    container = container.nextSiblingElement();
    QCOMPARE(container.isNull(), true);

    QCOMPARE(answer.arguments().at(1), "NumberReturned");
    QCOMPARE(answer.value("NumberReturned"), "6");
    QCOMPARE(answer.arguments().at(2), "TotalMatches");
    QCOMPARE(answer.value("TotalMatches"), "6");
    QCOMPARE(answer.arguments().at(3), "UpdateID");
    QCOMPARE(answer.value("UpdateID"), "1");

    QCOMPARE(m_error.netError(), QNetworkReply::NoError);
    QCOMPARE(m_error.code(), -5);
    QCOMPARE(m_error.description(), "");
    QCOMPARE(m_error.faultCode(), "");
    QCOMPARE(m_error.faultString(), "");
}

void Upnp_servicecontentdirectoryTest::test_browse_InvalidArgs()
{
    QString actionName = "Browse";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    {
        SoapAction action(m_contentDirectory->serviceType(), actionName);
        action.addArgument("ObjectId", "0");
        action.addArgument("BrowseFlag", "BrowseMetadata");
        action.addArgument("Filter", "*");
        action.addArgument("StartingIndex", "0");
        action.addArgument("RequestedCount", "0");
        action.addArgument("SortCriteria", "");

        UpnpActionReply *reply = m_contentDirectory->runAction(action);
        QVERIFY(reply != Q_NULLPTR);
        connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
        connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

        int timeout = 10;
        while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
        {
            timeout--;
            QTest::qWait(1000);
        }

        QVERIFY(m_XmlActionAnswer.size() == 0);

        QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
        QCOMPARE(m_error.code(), 402);
        QCOMPARE(m_error.description(), "Invalid Args");
        QCOMPARE(m_error.faultCode(), "s:Client");
        QCOMPARE(m_error.faultString(), "UPnPError");
    }

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    {
        SoapAction action(m_contentDirectory->serviceType(), actionName);
        action.addArgument("ObjectID", "0");
        action.addArgument("BrowseFlag", "BrowseMetadata");
        action.addArgument("Filter", "*");
        action.addArgument("StartingIndex", "0");
        action.addArgument("RequestedCount", "0");

        UpnpActionReply *reply = m_contentDirectory->runAction(action);
        QVERIFY(reply != Q_NULLPTR);
        connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
        connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

        int timeout = 10;
        while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
        {
            timeout--;
            QTest::qWait(1000);
        }

        QVERIFY(m_XmlActionAnswer.size() == 0);

        QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
        QCOMPARE(m_error.code(), 402);
        QCOMPARE(m_error.description(), "Invalid Args");
        QCOMPARE(m_error.faultCode(), "s:Client");
        QCOMPARE(m_error.faultString(), "UPnPError");
    }

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    {
        SoapAction action(m_contentDirectory->serviceType(), actionName);
        action.addArgument("ObjectID", "0");
        action.addArgument("BrowseFlag", "BrowseMetadata");
        action.addArgument("Filter", "*");
        action.addArgument("StartingIndex", "0");
        action.addArgument("RequestedCount", "0");
        action.addArgument("SortCriteria", "");
        action.addArgument("Args", "Invalid");

        UpnpActionReply *reply = m_contentDirectory->runAction(action);
        QVERIFY(reply != Q_NULLPTR);
        connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
        connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

        int timeout = 10;
        while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
        {
            timeout--;
            QTest::qWait(1000);
        }

        QVERIFY(m_XmlActionAnswer.size() == 0);

        QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
        QCOMPARE(m_error.code(), 402);
        QCOMPARE(m_error.description(), "Invalid Args");
        QCOMPARE(m_error.faultCode(), "s:Client");
        QCOMPARE(m_error.faultString(), "UPnPError");
    }
}

void Upnp_servicecontentdirectoryTest::test_browse_Invalid_objectID()
{
    QString actionName = "Browse";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    QVariantMap arguments;
    arguments["ObjectID"] = "";
    arguments["BrowseFlag"] = "BrowseMetadata";
    arguments["Filter"] = "*";
    arguments["StartingIndex"] = "0";
    arguments["RequestedCount"] = "0";
    arguments["SortCriteria"] = "";

    UpnpActionReply *reply = m_contentDirectory->runAction(actionName, arguments);
    QVERIFY(reply != Q_NULLPTR);
    connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
    connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

    int timeout = 10;
    while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
    {
        timeout--;
        QTest::qWait(1000);
    }

    QVERIFY(m_XmlActionAnswer.size() == 0);

    QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
    QCOMPARE(m_error.code(), 701);
    QCOMPARE(m_error.description(), "No such object");
    QCOMPARE(m_error.faultCode(), "s:Client");
    QCOMPARE(m_error.faultString(), "UPnPError");

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    arguments.clear();
    arguments["ObjectID"] = "39";
    arguments["BrowseFlag"] = "BrowseMetadata";
    arguments["Filter"] = "*";
    arguments["StartingIndex"] = "0";
    arguments["RequestedCount"] = "0";
    arguments["SortCriteria"] = "";

    reply = m_contentDirectory->runAction(actionName, arguments);
    QVERIFY(reply != Q_NULLPTR);
    connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
    connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

    timeout = 10;
    while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
    {
        timeout--;
        QTest::qWait(1000);
    }

    QVERIFY(m_XmlActionAnswer.size() == 0);

    QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
    QCOMPARE(m_error.code(), 701);
    QCOMPARE(m_error.description(), "No such object");
    QCOMPARE(m_error.faultCode(), "s:Client");
    QCOMPARE(m_error.faultString(), "UPnPError");
}

void Upnp_servicecontentdirectoryTest::test_browse_Invalid_browseFlag()
{
    // BrowseMetadata of root object

    QString actionName = "Browse";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    {
        QVariantMap arguments;
        arguments["ObjectID"] = "0";
        arguments["BrowseFlag"] = "BrowseMetaData";
        arguments["Filter"] = "*";
        arguments["StartingIndex"] = "0";
        arguments["RequestedCount"] = "0";
        arguments["SortCriteria"] = "";

        UpnpActionReply *reply = m_contentDirectory->runAction(actionName, arguments);
        QVERIFY(reply != Q_NULLPTR);
        connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
        connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

        int timeout = 10;
        while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
        {
            timeout--;
            QTest::qWait(1000);
        }

        QVERIFY(m_XmlActionAnswer.size() == 0);

        QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
        QCOMPARE(m_error.code(), 402);
        QCOMPARE(m_error.description(), "Invalid Args");
        QCOMPARE(m_error.faultCode(), "s:Client");
        QCOMPARE(m_error.faultString(), "UPnPError");
    }

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    {
        QVariantMap arguments;
        arguments["ObjectID"] = "0";
        arguments["BrowseFlag"] = "";
        arguments["Filter"] = "*";
        arguments["StartingIndex"] = "0";
        arguments["RequestedCount"] = "0";
        arguments["SortCriteria"] = "";

        UpnpActionReply *reply = m_contentDirectory->runAction(actionName, arguments);
        QVERIFY(reply != Q_NULLPTR);
        connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
        connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

        int timeout = 10;
        while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
        {
            timeout--;
            QTest::qWait(1000);
        }

        QVERIFY(m_XmlActionAnswer.size() == 0);

        QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
        QCOMPARE(m_error.code(), 402);
        QCOMPARE(m_error.description(), "Invalid Args");
        QCOMPARE(m_error.faultCode(), "s:Client");
        QCOMPARE(m_error.faultString(), "UPnPError");
    }
}

void Upnp_servicecontentdirectoryTest::test_browse_Invalid_filter()
{
    // BrowseMetadata of root object

    QString actionName = "Browse";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    {
        QVariantMap arguments;
        arguments["ObjectID"] = "0";
        arguments["BrowseFlag"] = "BrowseMetadata";
        arguments["Filter"] = "";
        arguments["StartingIndex"] = "0";
        arguments["RequestedCount"] = "0";
        arguments["SortCriteria"] = "";

        UpnpActionReply *reply = m_contentDirectory->runAction(actionName, arguments);
        QVERIFY(reply != Q_NULLPTR);
        connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
        connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

        int timeout = 10;
        while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
        {
            timeout--;
            QTest::qWait(1000);
        }

        QVERIFY(m_XmlActionAnswer.size() == 0);

        QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
        QCOMPARE(m_error.code(), 402);
        QCOMPARE(m_error.description(), "Invalid Args");
        QCOMPARE(m_error.faultCode(), "s:Client");
        QCOMPARE(m_error.faultString(), "UPnPError");
    }
}

void Upnp_servicecontentdirectoryTest::test_browse_Invalid_startingIndex()
{
    // BrowseMetadata of root object

    QString actionName = "Browse";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    {
        QVariantMap arguments;
        arguments["ObjectID"] = "0";
        arguments["BrowseFlag"] = "BrowseMetadata";
        arguments["Filter"] = "*";
        arguments["StartingIndex"] = "-1";
        arguments["RequestedCount"] = "0";
        arguments["SortCriteria"] = "";

        UpnpActionReply *reply = m_contentDirectory->runAction(actionName, arguments);
        QVERIFY(reply != Q_NULLPTR);
        connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
        connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

        int timeout = 10;
        while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
        {
            timeout--;
            QTest::qWait(1000);
        }

        QVERIFY(m_XmlActionAnswer.size() == 0);

        QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
        QCOMPARE(m_error.code(), 402);
        QCOMPARE(m_error.description(), "Invalid Args");
        QCOMPARE(m_error.faultCode(), "s:Client");
        QCOMPARE(m_error.faultString(), "UPnPError");
    }

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    {
        QVariantMap arguments;
        arguments["ObjectID"] = "0";
        arguments["BrowseFlag"] = "BrowseMetadata";
        arguments["Filter"] = "*";
        arguments["StartingIndex"] = "1";
        arguments["RequestedCount"] = "0";
        arguments["SortCriteria"] = "";

        UpnpActionReply *reply = m_contentDirectory->runAction(actionName, arguments);
        QVERIFY(reply != Q_NULLPTR);
        connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
        connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

        int timeout = 10;
        while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
        {
            timeout--;
            QTest::qWait(1000);
        }

        QVERIFY(m_XmlActionAnswer.size() == 0);

        QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
        QCOMPARE(m_error.code(), 402);
        QCOMPARE(m_error.description(), "Invalid Args");
        QCOMPARE(m_error.faultCode(), "s:Client");
        QCOMPARE(m_error.faultString(), "UPnPError");
    }

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    {
        QVariantMap arguments;
        arguments["ObjectID"] = "0";
        arguments["BrowseFlag"] = "BrowseDirectChildren";
        arguments["Filter"] = "*";
        arguments["StartingIndex"] = "-1";
        arguments["RequestedCount"] = "0";
        arguments["SortCriteria"] = "";

        UpnpActionReply *reply = m_contentDirectory->runAction(actionName, arguments);
        QVERIFY(reply != Q_NULLPTR);
        connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
        connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

        int timeout = 10;
        while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
        {
            timeout--;
            QTest::qWait(1000);
        }

        QVERIFY(m_XmlActionAnswer.size() == 0);

        QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
        QCOMPARE(m_error.code(), 402);
        QCOMPARE(m_error.description(), "Invalid Args");
        QCOMPARE(m_error.faultCode(), "s:Client");
        QCOMPARE(m_error.faultString(), "UPnPError");
    }
}

void Upnp_servicecontentdirectoryTest::test_browse_Invalid_requestedCount()
{
    // BrowseMetadata of root object

    QString actionName = "Browse";

    QVERIFY(m_upnp != Q_NULLPTR);
    QVERIFY(m_root != Q_NULLPTR);

    QVERIFY(m_contentDirectory != Q_NULLPTR);

    m_XmlActionAnswer.clear();
    m_error = UpnpError();

    {
        QVariantMap arguments;
        arguments["ObjectID"] = "0";
        arguments["BrowseFlag"] = "BrowseMetadata";
        arguments["Filter"] = "*";
        arguments["StartingIndex"] = "0";
        arguments["RequestedCount"] = "-1";
        arguments["SortCriteria"] = "";

        UpnpActionReply *reply = m_contentDirectory->runAction(actionName, arguments);
        QVERIFY(reply != Q_NULLPTR);
        connect(reply, &UpnpActionReply::errorOccured, this, &Upnp_servicecontentdirectoryTest::errorRaised);
        connect(reply, &UpnpActionReply::finished, this, &Upnp_servicecontentdirectoryTest::actionFinished);

        int timeout = 10;
        while (timeout>0 && m_XmlActionAnswer.size()==0 && m_error.netError()==QNetworkReply::NoError)
        {
            timeout--;
            QTest::qWait(1000);
        }

        QVERIFY(m_XmlActionAnswer.size() == 0);

        QCOMPARE(m_error.netError(), QNetworkReply::InternalServerError);
        QCOMPARE(m_error.code(), 402);
        QCOMPARE(m_error.description(), "Invalid Args");
        QCOMPARE(m_error.faultCode(), "s:Client");
        QCOMPARE(m_error.faultString(), "UPnPError");
    }
}

QTEST_MAIN(Upnp_servicecontentdirectoryTest)

#include "tst_upnp_servicecontentdirectorytest.moc"
