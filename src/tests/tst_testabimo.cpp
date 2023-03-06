#include <QDir>
#include <QFile>
#include <QtDebug>
#include <QtGlobal>
#include <QString>
#include <QStringList>
#include <QtTest>

#include "../app/calculation.h"
#include "../app/config.h"
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
    void test_requiredFields();
    void test_dbaseReader();
    void test_xmlReader();
    void test_config_getTWS();
    void test_calc();
    void test_bagrov();

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
        Helpers::abortIfNoSuchFile(filePath, context);
    }

    return filePath;
}

void TestAbimo::test_helpers_containsAll()
{
    QHash<QString, int> hash;
    hash["one"] = 1;
    hash["two"] = 2;

    QCOMPARE(Helpers::containsAll(hash, {"one", "two"}), true);
    QCOMPARE(Helpers::containsAll(hash, {"one", "two", "three"}), false);    
}

void TestAbimo::test_helpers_filesAreIdentical()
{
    QString file_1 = dataFilePath("abimo_2019_mitstrassen.dbf");
    QString file_2 = dataFilePath("abimo_2019_mitstrassenout_3.2_xml-config.dbf");

    bool debug = false;

    QCOMPARE(Helpers::filesAreIdentical(file_1, file_1, debug), true);
    QCOMPARE(Helpers::filesAreIdentical(file_1, file_2, debug), false);
}

void TestAbimo::test_helpers_stringsAreEqual()
{
    QString strings_1[] = {"a", "b", "c", "d", "e", "f"};
    QString strings_2[] = {"a", "b", "d", "e", "f", "g"};

    QCOMPARE(Helpers::stringsAreEqual(strings_1, strings_1, 6), true);
    QCOMPARE(Helpers::stringsAreEqual(strings_1, strings_2, 6), false);
}

void TestAbimo::test_requiredFields()
{
    QStringList strings = DbaseReader::requiredFields();
    QCOMPARE(strings.length(), 25);
}

void TestAbimo::test_dbaseReader()
{
    DbaseReader reader(dataFilePath("abimo_2019_mitstrassen.dbf"));

    bool success = reader.checkAndRead();

    qDebug() << "fullError: " << reader.getFullError();

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

    QVERIFY(qFuzzyCompare(iv.getInfiltrationFactorRoof(), 0.0F));
    QVERIFY(qFuzzyCompare(iv.getInfiltrationFactorSurface1(), 0.1F));
    QVERIFY(qFuzzyCompare(iv.getInfiltrationFactorSurface2(), 0.3F));
    QVERIFY(qFuzzyCompare(iv.getInfiltrationFactorSurface3(), 0.6F));
    QVERIFY(qFuzzyCompare(iv.getInfiltrationFactorSurface4(), 0.9F));
}

void TestAbimo::test_config_getTWS()
{
    // Create configuration object
    Config config;
    QVERIFY(qFuzzyCompare(config.getRootingDepth(Usage::vegetationless_D, 50), 0.2F));
    QVERIFY(qFuzzyCompare(config.getRootingDepth(Usage::agricultural_L, 50), 0.6F));
    QVERIFY(qFuzzyCompare(config.getRootingDepth(Usage::agricultural_L, 51), 0.7F));
    QVERIFY(qFuzzyCompare(config.getRootingDepth(Usage::horticultural_K, 50), 0.7F));
    QVERIFY(qFuzzyCompare(config.getRootingDepth(Usage::forested_W, 50), 1.0F));
    QVERIFY(qFuzzyCompare(config.getRootingDepth(Usage::unknown, 50), 0.2F));
}

void TestAbimo::test_calc()
{
    QString inputFile = dataFilePath("abimo_2019_mitstrassen.dbf");
    QString configFile = dataFilePath("config.xml");
    QString outputFile = dataFilePath("tmp_out.dbf", false);
    QString outFile_noConfig = dataFilePath("abimo_2019_mitstrassenout_3.2.1_default-config.dbf");
    QString outFile_xmlConfig = dataFilePath("abimo_2019_mitstrassenout_3.2.1_xml-config.dbf");

    Calculation::runCalculation(inputFile, "", outputFile, false);

    //QVERIFY(Helpers::filesAreIdentical(outputFile, referenceFile));
    QVERIFY(dbfHeadersAreIdentical(outputFile, outFile_noConfig));
    QVERIFY(dbfStringsAreIdentical(outputFile, outFile_noConfig));

    // Run the simulation with initial values from config file
    Calculation::runCalculation(inputFile, configFile, outputFile);
    QVERIFY(dbfStringsAreIdentical(outputFile, outFile_xmlConfig));
}

void TestAbimo::test_bagrov()
{

}

bool TestAbimo::dbfHeadersAreIdentical(QString file_1, QString file_2)
{
    DbaseReader reader_1(file_1);
    DbaseReader reader_2(file_2);

    reader_1.read();
    reader_2.read();

    if (reader_1.getVersion() != reader_2.getVersion()) {
        qDebug() << "version differs";
        return false;
    }

    QDate date_1 = reader_1.getDate();
    QDate date_2 = reader_2.getDate();

    if (date_1 != date_2) {
        qDebug() << "date differs (" << date_1 << "vs" << date_2 << ")."
                 << "This is expected.";
    }

    if (reader_1.getNumberOfRecords() != reader_2.getNumberOfRecords()) {
        qDebug() << "numberOfRecords differs";
        return false;
    }

    if (reader_1.getLengthOfHeader() != reader_2.getLengthOfHeader()) {
        qDebug() << "lengthOfHeader differs";
        return false;
    }

    if (reader_1.getLengthOfEachRecord() != reader_2.getLengthOfEachRecord()) {
        qDebug() << "lengthOfEachRecord differs";
        return false;
    }

    QString lng_1 = reader_1.getLanguageDriver();
    QString lng_2 = reader_2.getLanguageDriver();

    if (lng_1 != lng_2) {
        qDebug() << "languageDriver differs"
                 << "(" << lng_1 << "vs" << lng_2 << ")";
        //return false;
    }

    return true;
}

bool TestAbimo::dbfStringsAreIdentical(QString file_1, QString file_2)
{
    DbaseReader reader_1(file_1);
    DbaseReader reader_2(file_2);

    reader_1.read();
    reader_2.read();

    int nrows_1 = reader_1.getNumberOfRecords();
    int nrows_2 = reader_2.getNumberOfRecords();

    if (numbersInFilesDiffer(file_1, file_2, nrows_1, nrows_2, "rows")) {
        return false;
    };

    int ncols_1 = reader_1.getCountFields();
    int ncols_2 = reader_2.getCountFields();

    if (numbersInFilesDiffer(file_1, file_2, ncols_1, ncols_2, "columns")) {
        return false;
    };

    return Helpers::stringsAreEqual(
        reader_1.getValues(), reader_2.getValues(), nrows_1 * ncols_1, 5, true
    );
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

QTEST_APPLESS_MAIN(TestAbimo)

#include "tst_testabimo.moc"
