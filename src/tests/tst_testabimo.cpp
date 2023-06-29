#include <QDir>
#include <QFile>
#include <QtDebug>
#include <QtGlobal>
#include <QString>
#include <QStringList>
#include <QtTest>
#include <QtTest/QtTest>

#include "../app/abimoReader.h"
#include "../app/calculation.h"
#include "../app/usageConfiguration.h"
#include "../app/dbaseField.h"
#include "../app/dbaseReader.h"
#include "../app/helpers.h"

class TestAbimo : public QObject
{
    Q_OBJECT

public:
    TestAbimo();
    ~TestAbimo();

private slots:
    void test_helpers_containsAll();
    void test_helpers_filesAreIdentical();
    void test_helpers_stringsAreEqual();
    void test_helpers_formatFloat();
    void test_helpers_formatNumericString();
    void test_requiredFields();
    void test_dbaseReader();
    void test_xmlReader();
    void test_config_getTWS();
    void test_calc();
    void test_bagrov();
    void compare_dbfHeaders(DbaseFileHeader& h1, DbaseFileHeader& h2);

    QString testDataDir();
    QString dataFilePath(QString fileName, bool mustExist = true);
    bool dbfHeadersAreIdentical(QString file_1, QString file_2);
    bool dbfStringsAreIdentical(QString file_1, QString file_2);
    bool numbersInFilesDiffer(QString file_1, QString file_2, int n_1, int n_2, QString subject);
};

TestAbimo::TestAbimo()
{
}

TestAbimo::~TestAbimo()
{
}

QString TestAbimo::testDataDir()
{
    return QString("../../../abimo/data");
}

QString TestAbimo::dataFilePath(QString fileName, bool mustExist)
{
    QString filePath = QString(testDataDir() + "/" + fileName);

    if (mustExist) {
        QString context = "Data directory: " + testDataDir();
        helpers::abortIfNoSuchFile(filePath, context);
    }

    return filePath;
}

void TestAbimo::test_helpers_containsAll()
{
    QHash<QString, int> hash;
    hash["one"] = 1;
    hash["two"] = 2;

    QCOMPARE(helpers::containsAll(hash, {"one", "two"}), true);
    QCOMPARE(helpers::containsAll(hash, {"one", "two", "three"}), false);    
}

void TestAbimo::test_helpers_filesAreIdentical()
{
    QString file_1 = dataFilePath("abimo_2019_mitstrassen.dbf");
    QString file_2 = dataFilePath("abimo_2019_mitstrassenout_3.2_xml-config.dbf");

    bool debug = false;

    QCOMPARE(helpers::filesAreIdentical(file_1, file_1, debug), true);
    QCOMPARE(helpers::filesAreIdentical(file_1, file_2, debug), false);
}

void TestAbimo::test_helpers_stringsAreEqual()
{
    QVector<QString> strings_1 = {"a", "b", "c", "d", "e", "f"};
    QVector<QString> strings_2 = {"a", "b", "d", "e", "f", "g"};

    QCOMPARE(helpers::stringsAreEqual(strings_1, strings_1), true);
    QCOMPARE(helpers::stringsAreEqual(strings_1, strings_2), false);
}

void test_stringManipulation(QString& s) {
    s = QString("a");
}

void TestAbimo::test_helpers_formatFloat()
{
    QCOMPARE(helpers::formatFloat(123.0, 4, 0), QString("0123"));
    QCOMPARE(helpers::formatFloat(123.456, 5, 1), QString("123.5"));
    QCOMPARE(helpers::formatFloat(123.456, 6, 1), QString("0123.5"));
    QCOMPARE(helpers::formatFloat(123.456, 6, 2), QString("123.46"));
    QCOMPARE(helpers::formatFloat(-123.4, 5, 0), QString("-0123"));
    QCOMPARE(helpers::formatFloat(-123.456, 7, 1), QString("-0123.5"));

    QCOMPARE(helpers::rightJustifiedNumber("123.4", 5), QString("123.4"));
    QCOMPARE(helpers::rightJustifiedNumber("123.4", 6), QString("0123.4"));
    QCOMPARE(helpers::rightJustifiedNumber("123.45", 7), QString("0123.45"));
    QCOMPARE(helpers::rightJustifiedNumber("-123.4", 6), QString("-123.4"));
    QCOMPARE(helpers::rightJustifiedNumber("-123.4", 7), QString("-0123.4"));
}

void TestAbimo::test_helpers_formatNumericString()
{
    // Result string
    QString s;

    s = QString("abc");
    test_stringManipulation(s);
    QCOMPARE(s, QString("a"));
}

void TestAbimo::test_requiredFields()
{
    QStringList strings = AbimoReader::requiredFields();
    QCOMPARE(strings.length(), 25);
}

void TestAbimo::test_dbaseReader()
{
    AbimoReader reader(dataFilePath("abimo_2019_mitstrassen.dbf"));

    bool success = reader.checkAndRead();

    qDebug() << "fullError: " << reader.getError().textLong;

    QCOMPARE(success, true);
    QCOMPARE(reader.isAbimoFile(), true);
}

void TestAbimo::test_xmlReader()
{
    QString configFile = dataFilePath("config.xml");
    InitValues iv;

    QString errorMessage = InitValues::updateFromConfig(iv, configFile);
    QVERIFY(errorMessage.isEmpty());

    // Is the following part of config.xml read correctly?
    /*<section name="Infiltrationsfaktoren">
        <item key="Dachflaechen"   value="0.00" />
        <item key="Belaglsklasse1" value="0.10" />
        <item key="Belaglsklasse2" value="0.30" />
        <item key="Belaglsklasse3" value="0.60" />
        <item key="Belaglsklasse4" value="0.90" />*/

    QVERIFY(qFuzzyCompare(iv.getInfiltrationFactor(0), 0.0F));
    QVERIFY(qFuzzyCompare(iv.getInfiltrationFactor(1), 0.1F));
    QVERIFY(qFuzzyCompare(iv.getInfiltrationFactor(2), 0.3F));
    QVERIFY(qFuzzyCompare(iv.getInfiltrationFactor(3), 0.6F));
    QVERIFY(qFuzzyCompare(iv.getInfiltrationFactor(4), 0.9F));
}

void TestAbimo::test_config_getTWS()
{
    // Create configuration object
    UsageConfiguration config;
    QVERIFY(qFuzzyCompare(config.getRootingDepth(Usage::vegetationless_D, 50), 0.2F));
    QVERIFY(qFuzzyCompare(config.getRootingDepth(Usage::agricultural_L, 50), 0.6F));
    QVERIFY(qFuzzyCompare(config.getRootingDepth(Usage::agricultural_L, 51), 0.7F));
    QVERIFY(qFuzzyCompare(config.getRootingDepth(Usage::horticultural_K, 50), 0.7F));
    QVERIFY(qFuzzyCompare(config.getRootingDepth(Usage::forested_W, 50), 1.0F));
    QVERIFY(qFuzzyCompare(config.getRootingDepth(Usage::unknown, 50), 0.2F));
}

void TestAbimo::test_calc()
{
    QFile inputFile(dataFilePath("abimo_2019_mitstrassen.dbf"));
    QFile configFile(dataFilePath("config.xml"));
    QFile tmpOut_noConfig(dataFilePath("tmp_out_no-config.dbf", false));
    QFile tmpOut_config(dataFilePath("tmp_out_config.dbf", false));
    QFile refOut_noConfig(dataFilePath("abimo_2019_mitstrassenout_3.2.1_default-config.dbf"));
    QFile refOut_config(dataFilePath("abimo_2019_mitstrassenout_3.2.1_xml-config.dbf"));

    // Run the simulation without config file
    Calculation::runCalculation(inputFile.fileName(), "", tmpOut_noConfig.fileName(), false);
    QVERIFY(tmpOut_noConfig.size() == refOut_noConfig.size());
    QVERIFY(dbfHeadersAreIdentical(tmpOut_noConfig.fileName(), refOut_noConfig.fileName()));
    QVERIFY(dbfStringsAreIdentical(tmpOut_noConfig.fileName(), refOut_noConfig.fileName()));

    // Files are not identical due to differing dates in the header
    //QVERIFY(helpers::filesAreIdentical(outputFile, referenceFile));

    // Run the simulation with initial values from config file
    Calculation::runCalculation(inputFile.fileName(), configFile.fileName(), tmpOut_config.fileName());
    QVERIFY(tmpOut_config.size() == refOut_config.size());
    QVERIFY(dbfHeadersAreIdentical(tmpOut_config.fileName(), refOut_config.fileName()));
    QVERIFY(dbfStringsAreIdentical(tmpOut_config.fileName(), refOut_config.fileName()));

    // file created using the GUI
    //QString file_tmp = dataFilePath("abimo_2019_mitstrassen_out2.dbf", true);
    //QVERIFY(dbfStringsAreIdentical(file_tmp, outFile_noConfig));
}

void TestAbimo::test_bagrov()
{

}

void TestAbimo::compare_dbfHeaders(DbaseFileHeader& h1, DbaseFileHeader& h2)
{
    QCOMPARE(h1.headerLength, h2.headerLength);
    QCOMPARE(h1.languageDriver, h2.languageDriver);
    QCOMPARE(h1.numberOfRecords, h2.numberOfRecords);
    QCOMPARE(h1.recordLength, h2.recordLength);
    QCOMPARE(h1.version, h2.version);
}

bool TestAbimo::dbfHeadersAreIdentical(QString file_1, QString file_2)
{
    DbaseReader reader_1(file_1);
    DbaseReader reader_2(file_2);

    reader_1.read();
    reader_2.read();

    DbaseFileHeader h1 = reader_1.getHeader();
    DbaseFileHeader h2 = reader_2.getHeader();

    compare_dbfHeaders(h1, h2);

    QDate date_1 = h1.date;
    QDate date_2 = h2.date;

    if (date_1 != date_2) {
        qDebug() << "date differs (" << date_1 << "vs" << date_2 << ")."
                 << "This is expected.";
    }

    int length1 = reader_1.calculateRecordLength();
    int length2 = reader_2.calculateRecordLength();

    if (length1 != length2) {
        qDebug() << "record length (recalculated) differs";
        qDebug() << "length1:" << length1 << "\b, length2:" << length2;
        return false;
    }

    return true;
}

bool TestAbimo::dbfStringsAreIdentical(QString file_1, QString file_2)
{
    DbaseReader reader_1(file_1);
    DbaseReader reader_2(file_2);

    reader_1.read();
    reader_2.read();

    int nrows_1 = reader_1.getHeader().numberOfRecords;
    int nrows_2 = reader_2.getHeader().numberOfRecords;

    if (numbersInFilesDiffer(file_1, file_2, nrows_1, nrows_2, "rows")) {
        return false;
    };

    int ncols_1 = reader_1.getNumberOfFields();
    int ncols_2 = reader_2.getNumberOfFields();

    if (numbersInFilesDiffer(file_1, file_2, ncols_1, ncols_2, "columns")) {
        return false;
    };

    QVector<QString> values_1 = reader_1.getStringValues();
    QVector<QString> values_2 = reader_2.getStringValues();

    return helpers::stringsAreEqual(values_1, values_2, 10, true);
}

bool TestAbimo::numbersInFilesDiffer(QString file_1, QString file_2, int n_1, int n_2, QString subject)
{
    if (n_1 != n_2) {
        qDebug() << QString("Number of %1 (file_1: %2, file_2: %3) differs.").arg(
            subject, file_1, file_2, QString::number(n_1), QString::number(n_2)
        );
        return true;
    };

    return false;
}

//QTEST_APPLESS_MAIN(TestAbimo)
int main(int argc, char *argv[])
{
    TestAbimo tc;

    //QTEST_SET_MAIN_SOURCE_PATH
    printf("__FILE__: %s\n", __FILE__);
    QTest::setMainSourcePath(__FILE__);

    return QTest::qExec(&tc, argc, argv);
}

#include "tst_testabimo.moc"
