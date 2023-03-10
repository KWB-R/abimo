#include <vector>

#include <QCommandLineParser>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QStringList>

#include "helpers.h"

QString helpers::nowString()
{
    QDateTime now = QDateTime::currentDateTime();

    return
        "am: " + now.toString("dd.MM.yyyy") +
        " um: " + now.toString("hh:mm:ss");
}

QString helpers::positionalArgOrNULL(QCommandLineParser* arguments, int index)
{
    const QStringList posArgs = arguments->positionalArguments();
    return (posArgs.length() > index) ? posArgs.at(index) : NULL;
}

QString helpers::removeFileExtension(QString fileName)
{
    QFileInfo fileInfo(fileName);

    return fileInfo.absolutePath() + "/" + fileInfo.baseName();
}

QString helpers::singleQuote(QString string)
{
    return "'" + string + "'";
}

QString helpers::patternDbfFile()
{
    return QString("dBase (*.dbf)");
}

QString helpers::patternXmlFile()
{
    return QString("Extensible Markup Language (*.xml)");
}

QString helpers::defaultOutputFileName(QString inputFileName)
{
    return helpers::removeFileExtension(inputFileName)  + "_out.dbf";
}

QString helpers::defaultLogFileName(QString outputFileName)
{
    return helpers::removeFileExtension(outputFileName)  + ".log";
}

// Return true if all keys are contained in the hash, else false
bool helpers::containsAll(QHash<QString, int> hash, QStringList keys)
{
    for (int i = 0; i < keys.length(); i++) {
        if (! hash.contains(keys.at(i))) {
            return false;
        }
    }

    return true;
}

void helpers::openFileOrAbort(QFile& file, QIODevice::OpenModeFlag mode)
{
    if (!file.open(mode)) {
        qDebug() << "Cannot open file: " << file.fileName() << ": " << file.errorString();
        abort();
    }
}

bool helpers::filesAreIdentical(QString fileName_1, QString fileName_2, bool debug, int maxDiffs)
{
    if (debug) {
        qDebug() << "Comparing two files: ";
        qDebug() << "File 1: " << fileName_1;
        qDebug() << "File 2: " << fileName_2;
    }

    QFile file_1(fileName_1);
    QFile file_2(fileName_2);

    openFileOrAbort(file_1);
    openFileOrAbort(file_2);

    bool result = true;

    if (file_1.size() != file_2.size()) {
        result = false;
    }

    if (result) {

        QByteArray blob_1 = file_1.readAll();
        QByteArray blob_2 = file_2.readAll();

        int i = 0;
        int n_diffs = 0;

        while(i < blob_1.length() && n_diffs < maxDiffs) {
            if (blob_1[i] != blob_2[i]) {
                n_diffs++;
                qDebug() << QString("%1. byte difference at index %2").arg(
                    QString::number(n_diffs),
                    QString::number(i)
                );
            }
            i++;
        }

        result = (n_diffs == 0);
    }

    if (debug) {
        qDebug() << QString("The files %1 identical.").arg(result ? "are": "are not");
    }

    return result;
}

void helpers::abortIfNoSuchFile(QString filePath, QString context)
{
    if (!QFile::exists(filePath)) {
        qDebug() << "File does not exist: " << filePath;
        qDebug() << "Current directory: " << QDir::currentPath();

        if (!context.isEmpty()) {
            qDebug() << context;
        }

        abort();
    }
}

bool helpers::stringsAreEqual(
        QVector<QString>& strings_1,
        QVector<QString>& strings_2, int maxDiffs, bool debug)
{
    int n_diffs = 0;
    int i = 0;
    int n = helpers::min(strings_1.size(), strings_2.size());

    while (i < n && n_diffs < maxDiffs) {
        if (strings_1.at(i) != strings_2.at(i)) {
            n_diffs++;
            if (debug) {
                qDebug() << QString(
                    "%1. string mismatch at index %2:").arg(
                    QString::number(n_diffs),
                    QString::number(i)
                );
                qDebug() << QString("  string 1: '%1'").arg(strings_1.at(i));
                qDebug() << QString("  string 2: '%1'").arg(strings_2.at(i));
            }
        }
        i++;
    }

    return (n_diffs == 0);
}

int helpers::stringToInt(QString string, QString context, bool debug)
{
    int result = string.toInt();

    if (debug) {
        qDebug() << context << result << QString("(= %1.toInt())").arg(singleQuote(string));
    }

    return result;
}

float helpers::stringToFloat(QString string, QString context, bool debug)
{
    float result = string.toFloat();

    if (debug) {
        qDebug() << context << result << QString("(= %1.toFloat())").arg(singleQuote(string));
    }

    return result;
}

//
// Find the index of a value in a sorted array
//
int helpers::index(float xi, const std::vector<float> &x, float epsilon)
{
    int n = x.size();

    for (int i = 0; i < n; i++) {
        if (xi <= x.at(i) + epsilon) {
            return i;
        }
    }

    return n - 1;
}

float helpers::interpolate(
    float xi, const std::vector<float> &x, const std::vector<float> &y
)
{
    int i;
    int n = static_cast<int>(x.size());

    assert(n == static_cast<int>(y.size()));

    if (xi <= x.at(0)) {
        return y.at(0);
    }

    if (xi >= x.at(n - 1)) {
        return y.at(n - 1);
    }

    for (i = 1; i < n; i++) {
        if (xi <= x.at(i)) {
            return (y.at(i - 1) + y.at(i)) / 2.0;
        }
    }

    return 0.0;
}
