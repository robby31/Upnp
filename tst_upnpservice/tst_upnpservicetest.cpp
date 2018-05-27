#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include "upnpservice.h"
#include "upnpservicedescription.h"

class UpnpServiceTest : public QObject
{
    Q_OBJECT

public:
    UpnpServiceTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testUpnpServiceFromDescription();
};

UpnpServiceTest::UpnpServiceTest()
{
}

void UpnpServiceTest::initTestCase()
{
}

void UpnpServiceTest::cleanupTestCase()
{
}

void UpnpServiceTest::testUpnpServiceFromDescription()
{
    UpnpServiceDescription *serviceDescription = new UpnpServiceDescription();
    serviceDescription->setServiceAttribute("serviceType", "urn:schemas-upnp-org:service:ContentDirectory:1");
    serviceDescription->setServiceAttribute("serviceId", "urn:upnp-org:serviceId:ContentDirectory");
    serviceDescription->setServiceAttribute("SCPDURL", "/UPnP_AV_ContentDirectory_1.0.xml");
    serviceDescription->setServiceAttribute("controlURL", "/upnp/control/content_directory");
    serviceDescription->setServiceAttribute("eventSubURL", "/upnp/event/content_directory");

    QDomElement action = serviceDescription->addAction("GetSearchCapabilities");
    serviceDescription->addArgument(action, "SearchCaps", UpnpServiceDescription::OUT, "SearchCapabilities");
    QCOMPARE(serviceDescription->actionsName().size(), 1);

    QDomElement stateVariable = serviceDescription->addStateVariable("SearchCapabilities", true, false, "string", "true");
    serviceDescription->addAllowedValueRange(stateVariable, "0", "10", "1");
    stateVariable = serviceDescription->addStateVariable("TestWithList", true, false, "string", "UNKNOWN");
    QStringList l_values;
    l_values << "OK" << "KO" << "UNKNOWN";
    serviceDescription->addAllowedValueList(stateVariable, l_values);
    QCOMPARE(serviceDescription->stateVariablesName().size(), 2);

    qDebug() << serviceDescription->stringInfo();
    qDebug() << serviceDescription->stringDescription();

    QCOMPARE(serviceDescription->xmlDescription().tagName(), QString("scpd"));
    QCOMPARE(serviceDescription->version(), QString("1.0"));

    UpnpService service(Q_NULLPTR, serviceDescription->xmlInfo());
    QCOMPARE(service.serviceType(), QString("urn:schemas-upnp-org:service:ContentDirectory:1"));
    QCOMPARE(service.id(), QString("urn:upnp-org:serviceId:ContentDirectory"));
    QCOMPARE(service.serviceId(), QString("urn:upnp-org:serviceId:ContentDirectory"));
    QCOMPARE(service.scpdUrl(), QString("/UPnP_AV_ContentDirectory_1.0.xml"));
    QCOMPARE(service.controlUrl(), QString("/upnp/control/content_directory"));
    QCOMPARE(service.eventSubUrl(), QString("/upnp/event/content_directory"));

    QCOMPARE(service.available(), false);
    QCOMPARE(service.status(), UpnpObject::Null);

    service.setDescription(serviceDescription);
    service.setStatus(UpnpObject::Ready);

    QCOMPARE(service.status(), UpnpObject::Ready);
    QCOMPARE(service.description()->xmlDescription().childNodes().size(), 3);

    QCOMPARE(service.actionsModel().size(), 1);
    QCOMPARE(service.stateVariablesModel()->rowCount(), 2);
}

QTEST_MAIN(UpnpServiceTest)

#include "tst_upnpservicetest.moc"
