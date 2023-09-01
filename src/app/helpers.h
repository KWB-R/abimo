#ifndef HELPERS_H
#define HELPERS_H

#include <cmath> // for round
#include <vector>

#include <QCommandLineParser>
#include <QFile>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

#include "structs.h"

namespace helpers
{
    QString nowString();
    QString positionalArgOrNULL(QCommandLineParser*, int);
    QString singleQuote(QString);
    QString patternDbfFile();
    QString patternXmlFile();
    QString defaultOutputFileName(QString inputFileName);
    QString defaultLogFileName(QString outputFileName);
    bool containsAll(QHash<QString, int> hash, QStringList keys);
    void openFileOrAbort(QFile& file, QIODevice::OpenModeFlag mode = QIODevice::ReadOnly);
    bool filesAreIdentical(QString file_1, QString file_2, bool debug = true, int maxDiffs = 5);
    void abortIfNoSuchFile(QString filePath, QString context = "");
    bool stringsAreEqual(QVector<QString>& strings_1,
                         QVector<QString>& strings_2,
                         int maxDiffs = 5,
                         bool debug = false);
    int stringToInt(QString string, QString context, bool debug = false);
    float stringToFloat(QString string, QString context, bool debug = false);

    QString formatFloat(float value, int length, int digits);
    QString rightJustifiedNumber(QString value, int width, QChar fill = QChar('0'));

    float roundFloat(float value, int digits);

    inline float asFloat(int value) {
        return static_cast<float>(value);
    };

    int index(float xi, const std::vector<float> &x, float epsilon = 0.0001F);
    float interpolate(float xi, const std::vector<float> &x, const std::vector<float> &y);
    QString removeFileExtension(QString);

    inline float min(float a, float b) {
        return (a < b) ? a : b;
    }

    inline float max(float a, float b) {
        return (a > b) ? a : b;
    }

    inline int roundToInteger(float x) {
        return (int) round(x);
    }

    inline float vectorSum(QVector<float> x) {
        float sum = 0.0;
        for (int i = 0; i < static_cast<int>(x.size()); i++) {
            sum += x[i];
        }
        return sum;
    }

    QVector<int> rangesStringToIntegerSequence(QString s);
    IntegerRange splitRangeString(QString s);

};

#endif // HELPERS_H
