#include <QString>
#include <QtTest>
#include "Http/httprange.h"

class HttpRangeTests : public QObject
{
    Q_OBJECT

public:
    HttpRangeTests() = default;

private Q_SLOTS:
    void testCase1_data();
    void testCase1();
};

void HttpRangeTests::testCase1_data()
{
    QTest::addColumn<QString>("p_range");
    QTest::addColumn<QString>("p_size");

    QTest::addColumn<bool>("isNull");
    QTest::addColumn<QString>("unit");
    QTest::addColumn<long>("lowRange");
    QTest::addColumn<long>("highRange");
    QTest::addColumn<long>("startByte");
    QTest::addColumn<long>("endByte");
    QTest::addColumn<long>("length");
    QTest::addColumn<long>("size");

    QTest::newRow("null") << QString() << QString() << true << QString() << (long)0 << (long)0 << (long)-1 << (long)-1 << (long)-1 << (long)-1;
    QTest::newRow("null size=500") << QString() << QString("500") << true << QString() << (long)0 << (long)0 << (long)-1 << (long)-1 << (long)-1 << (long)500;

    QTest::newRow("RANGE: BYTES=10-500") << QString("RANGE: BYTES=10-500") << QString() << false << QString("BYTES") << (long)10 << (long)500 << (long)-1 << (long)-1 << (long)-1 << (long)-1;
    QTest::newRow("RANGE: BYTES=10-500 size=1000") << QString("RANGE: BYTES=10-500") << QString("1000") << false << QString("BYTES") << (long)10 << (long)500 << (long)10 << (long)500 << (long)491 << (long)1000;
    QTest::newRow("RANGE: BYTES=10-500 size=0") << QString("RANGE: BYTES=10-500") << QString("0") << false << QString("BYTES") << (long)10 << (long)500 << (long)-1 << (long)-1 << (long)-1 << (long)0;
    QTest::newRow("RANGE: BYTES=10-500 size=3") << QString("RANGE: BYTES=10-500") << QString("3") << false << QString("BYTES") << (long)10 << (long)500 << (long)-1 << (long)2 << (long)-1 << (long)3;
    QTest::newRow("RANGE: BYTES=10-500 size=9") << QString("RANGE: BYTES=10-500") << QString("9") << false << QString("BYTES") << (long)10 << (long)500 << (long)-1 << (long)8 << (long)-1 << (long)9;
    QTest::newRow("RANGE: BYTES=10-500 size=10") << QString("RANGE: BYTES=10-500") << QString("10") << false << QString("BYTES") << (long)10 << (long)500 << (long)-1 << (long)9 << (long)-1 << (long)10;
    QTest::newRow("RANGE: BYTES=10-500 size=11") << QString("RANGE: BYTES=10-500") << QString("11") << false << QString("BYTES") << (long)10 << (long)500 << (long)10 << (long)10 << (long)1 << (long)11;
    QTest::newRow("RANGE: BYTES=10-500 size=400") << QString("RANGE: BYTES=10-500") << QString("400") << false << QString("BYTES") << (long)10 << (long)500 << (long)10 << (long)399 << (long)390 << (long)400;
    QTest::newRow("RANGE: BYTES=10-500 size=499") << QString("RANGE: BYTES=10-500") << QString("499") << false << QString("BYTES") << (long)10 << (long)500 << (long)10 << (long)498 << (long)489 << (long)499;
    QTest::newRow("RANGE: BYTES=10-500 size=500") << QString("RANGE: BYTES=10-500") << QString("500") << false << QString("BYTES") << (long)10 << (long)500 << (long)10 << (long)499 << (long)490 << (long)500;
    QTest::newRow("RANGE: BYTES=10-500 size=501") << QString("RANGE: BYTES=10-500") << QString("501") << false << QString("BYTES") << (long)10 << (long)500 << (long)10 << (long)500 << (long)491 << (long)501;

    QTest::newRow("RANGE: BYTES=20-") << QString("RANGE: BYTES=20-") << QString() << false << QString("BYTES") << (long)20 << (long)-1 << (long)-1 << (long)-1 << (long)-1 << (long)-1;
    QTest::newRow("RANGE: BYTES=20- size=1000") << QString("RANGE: BYTES=20-") << QString("1000") << false << QString("BYTES") << (long)20 << (long)-1 << (long)20 << (long)999 << (long)980 << (long)1000;
    QTest::newRow("RANGE: BYTES=20- size=3") << QString("RANGE: BYTES=20-") << QString("3") << false << QString("BYTES") << (long)20 << (long)-1 << (long)-1 << (long)2 << (long)-1 << (long)3;
    QTest::newRow("RANGE: BYTES=20- size=10") << QString("RANGE: BYTES=20-") << QString("10") << false << QString("BYTES") << (long)20 << (long)-1 << (long)-1 << (long)9 << (long)-1 << (long)10;
    QTest::newRow("RANGE: BYTES=20- size=19") << QString("RANGE: BYTES=20-") << QString("19") << false << QString("BYTES") << (long)20 << (long)-1 << (long)-1 << (long)18 << (long)-1 << (long)19;
    QTest::newRow("RANGE: BYTES=20- size=20") << QString("RANGE: BYTES=20-") << QString("20") << false << QString("BYTES") << (long)20 << (long)-1 << (long)-1 << (long)19 << (long)-1 << (long)20;
    QTest::newRow("RANGE: BYTES=20- size=21") << QString("RANGE: BYTES=20-") << QString("21") << false << QString("BYTES") << (long)20 << (long)-1 << (long)20 << (long)20 << (long)1 << (long)21;

    QTest::newRow("RANGE: BYTES=-900") << QString("RANGE: BYTES=-900") << QString() << false << QString("BYTES") << (long)-1 << (long)900 << (long)-1 << (long)-1 << (long)-1 << (long)-1;
    QTest::newRow("RANGE: BYTES=-900 size=1000") << QString("RANGE: BYTES=-900") << QString("1000") << false << QString("BYTES") << (long)-1 << (long)900 << (long)100 << (long)999 << (long)900 << (long)1000;
    QTest::newRow("RANGE: BYTES=-900 size=3") << QString("RANGE: BYTES=-900") << QString("3") << false << QString("BYTES") << (long)-1 << (long)900 << (long)0 << (long)2 << (long)3 << (long)3;
    QTest::newRow("RANGE: BYTES=-900 size=10") << QString("RANGE: BYTES=-900") << QString("10") << false << QString("BYTES") << (long)-1 << (long)900 << (long)0 << (long)9 << (long)10 << (long)10;
    QTest::newRow("RANGE: BYTES=-900 size=100") << QString("RANGE: BYTES=-900") << QString("100") << false << QString("BYTES") << (long)-1 << (long)900 << (long)0 << (long)99 << (long)100 << (long)100;

    QTest::newRow("RANGE: BYTES=0-0") << QString("RANGE: BYTES=0-0") << QString() << false << QString("BYTES") << (long)0 << (long)0 << (long)-1 << (long)-1 << (long)-1 << (long)-1;
    QTest::newRow("RANGE: BYTES=0-0 size=1000") << QString("RANGE: BYTES=0-0") << QString("1000") << false << QString("BYTES") << (long)0 << (long)0 << (long)0 << (long)0 << (long)1 << (long)1000;
    QTest::newRow("RANGE: BYTES=0-0 size=3") << QString("RANGE: BYTES=0-0") << QString("3") << false << QString("BYTES") << (long)0 << (long)0 << (long)0 << (long)0 << (long)1 << (long)3;
    QTest::newRow("RANGE: BYTES=0-0 size=10") << QString("RANGE: BYTES=0-0") << QString("10") << false << QString("BYTES") << (long)0 << (long)0 << (long)0 << (long)0 << (long)1 << (long)10;

    QTest::newRow("RANGE: BYTES=110-5000") << QString("RANGE: BYTES=110-5000") << QString() << false << QString("BYTES") << (long)110 << (long)5000 << (long)-1 << (long)-1 << (long)-1 << (long)-1;
    QTest::newRow("RANGE: BYTES=110-5000 size=1000") << QString("RANGE: BYTES=110-5000") << QString("1000") << false << QString("BYTES") << (long)110 << (long)5000 << (long)110 << (long)999 << (long)890 << (long)1000;
    QTest::newRow("RANGE: BYTES=110-5000 size=3") << QString("RANGE: BYTES=110-5000") << QString("3") << false << QString("BYTES") << (long)110 << (long)5000 << (long)-1 << (long)2 << (long)-1 << (long)3;
    QTest::newRow("RANGE: BYTES=110-5000 size=10") << QString("RANGE: BYTES=110-5000") << QString("10") << false << QString("BYTES") << (long)110 << (long)5000 << (long)-1 << (long)9 << (long)-1 << (long)10;

    QTest::newRow("RANGE: BYTES=500-10") << QString("RANGE: BYTES=500-10") << QString() << true << QString("BYTES") << (long)-1 << (long)-1 << (long)-1 << (long)-1 << (long)-1 << (long)-1;
    QTest::newRow("RANGE: BYTES=500-10 size=500") << QString("RANGE: BYTES=500-10") << QString("500") << true << QString("BYTES") << (long)-1 << (long)-1 << (long)-1 << (long)-1 << (long)-1 << (long)500;
}

void HttpRangeTests::testCase1()
{    
    QFETCH(QString, p_range);
    QFETCH(QString, p_size);

    HttpRange *obj;
    if (p_range.isNull())
        obj = new HttpRange();
    else
        obj = new HttpRange(p_range);

    if (!p_size.isNull())
        obj->setSize(p_size.toLong());

    QFETCH(bool, isNull);
    QCOMPARE(obj->isNull(), isNull);

    QFETCH(QString, unit);
    QCOMPARE(obj->getUnit(), unit);

    QFETCH(long, lowRange);
    QCOMPARE(obj->getLowRange(), lowRange);

    QFETCH(long, highRange);
    QCOMPARE(obj->getHighRange(), highRange);

    QFETCH(long, startByte);
    QCOMPARE(obj->getStartByte(), startByte);

    QFETCH(long, endByte);
    QCOMPARE(obj->getEndByte(), endByte);

    QFETCH(long, length);
    QCOMPARE(obj->getLength(), length);

    QFETCH(long, size);
    QCOMPARE(obj->getSize(), size);

    delete obj;
}

QTEST_MAIN(HttpRangeTests)

#include "tst_httprangetests.moc"
