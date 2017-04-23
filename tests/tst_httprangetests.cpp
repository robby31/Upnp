#include <QString>
#include <QtTest>

class HttpRangeTests : public QObject
{
    Q_OBJECT

public:
    HttpRangeTests();

private Q_SLOTS:
    void testCase1_data();
    void testCase1();
};

HttpRangeTests::HttpRangeTests()
{
}

void HttpRangeTests::testCase1_data()
{
    QTest::addColumn<QString>("data");
    QTest::newRow("0") << QString();
}

void HttpRangeTests::testCase1()
{
    QFETCH(QString, data);
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(HttpRangeTests)

#include "tst_httprangetests.moc"
