/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QtDebug>

#include "main.h"
#include "bagrov.h"
#include "calculation.h"
#include "constants.h"
#include "dbaseReader.h"
#include "helpers.h"
#include "initvalues.h"
#include "mainwindow.h"

bool parseForBatch(int &argc, char** /*argv*/)
{
    //return false;
    return argc > 1;

    /*
    bool isBatch = false;

    for (int i = 0; i < argc; i++) {

        qDebug() << "Argument arg[" << i << "]: " << argv[i];

        if (! qstrcmp(argv[i], "--batch"))
            isBatch = true;
    }

    return isBatch;
    */
}

void defineParser(QCommandLineParser* parser)
{
    parser->setApplicationDescription("Abimo-Programm");
    parser->addHelpOption();
    parser->addVersionOption();

    parser->addPositionalArgument(
        "source",
        QCoreApplication::translate("main", "Input dbf-file.")
    );

    parser->addPositionalArgument(
        "destination",
        QCoreApplication::translate("main", "Destination dbf-file (optional)."),
        "[destination]"
    );

    // Option -d --debug: debug mode
    QCommandLineOption debugOption(
        QStringList() << "d" << "debug",
        QCoreApplication::translate("main", "Show debug information.")
    );

    // Option -c --config <config-file>
    QCommandLineOption configOption(
        QStringList() << "c" << "config",
        QCoreApplication::translate("main", "Override initial values with values in 'config.xml'"),
        QCoreApplication::translate("main", "config-file")
    );

    // Option --write-bagrov-table
    QCommandLineOption bagrovOption(
        QStringList() << "b" << "write-bagrov-table",
        QCoreApplication::translate("main", "Output table of Bagrov calculations")
    );

    parser->addOption(debugOption);
    parser->addOption(configOption);
    parser->addOption(bagrovOption);
}

void debugInputs(
    QString inputFileName,
    QString outputFileName,
    QString configFileName,
    QString logFileName,
    bool debug
)
{
    qDebug() << "Running in batch mode...";
    qDebug() << "inputFileName =" << inputFileName;
    qDebug() << "outputFileName =" << outputFileName;
    qDebug() << "configFile =" << configFileName;
    qDebug() << "logFileName =" << logFileName;
    qDebug() << "debug =" << debug;
}

// Options/arguments for example call on the command line
// --config ..\config.xml ..\abimo_2012ges.dbf ..\abimo-result.dbf

int main_batch(int argc, char *argv[])
{    
    QCoreApplication app(argc, argv);
    app.setApplicationVersion(VERSION_STRING);

    QCommandLineParser parser;
    defineParser(&parser);

    // Process the actual command line arguments given by the user
    parser.process(app);

    const QStringList positionalArgs = parser.positionalArguments();
    QString inputFileName = Helpers::positionalArgOrNULL(&parser, 0);

    QString outputFileName = Helpers::positionalArgOrNULL(&parser, 1);

    // If no output file name was given, create a default output file name
    if (outputFileName == NULL) {
        outputFileName = Helpers::defaultOutputFileName(inputFileName);
    }

    QString configFileName= parser.value("config");

    QString logFileName = Helpers::defaultLogFileName(outputFileName);
    bool debug = parser.isSet("debug");

    // Handle --write-bagrov-table
    if (parser.isSet("write-bagrov-table")) {
        writeBagrovTable();
        return 0;
    }

    debugInputs(inputFileName, outputFileName, configFileName, logFileName, debug);

    DbaseReader dbReader(inputFileName);

    if (! dbReader.checkAndRead()) {
        qDebug() << dbReader.getFullError();
        return 2;
    }

    // Update default initial values with values given in config.xml
    InitValues initValues;
    QString errorMessage = InitValues::updateFromConfig(initValues, configFileName);

    if (errorMessage.length() > 0) {
        qDebug() << "Error: " << errorMessage;
    }

    QFile logFile(logFileName);

    if (! logFile.open(QFile::WriteOnly)) {
        qDebug() << "Konnte Datei: '" << logFileName <<
            "' nicht oeffnen.\n" << logFile.error();
        return 1;
    }

    QTextStream logStream(&logFile);

    logStream << "Start der Berechnung " + Helpers::nowString() + "\r\n";

    // Create calculator object
    Calculation calculator(dbReader, initValues, logStream);

    qDebug() << "Start the calculation";
    calculator.calculate(outputFileName);
    qDebug() << "End of calculation (Results are in " << outputFileName << ").";

    return -1;
}

int main_gui(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationVersion(VERSION_STRING);

    QCommandLineParser parser;
    defineParser(&parser);

    // Process the actual command line arguments given by the user
    parser.process(app);

    MainWindow mainWin(&app, &parser);
    mainWin.show();

    return app.exec();
}

int main(int argc, char *argv[])
{
    //qDebug() << QString("1.2").toFloat();
    //qDebug() << QString("1,2").toFloat();

    //return 0;

    qDebug() << "Number of command line arguments: " << argc;

    if (parseForBatch(argc, argv)) {

        // Start batch version...
        return main_batch(argc, argv);
    }

    // Start GUI version...
    return main_gui(argc, argv);
}

QTextStream& qStdOut()
{
    static QTextStream ts( stdout );
    return ts;
}

#if defined(_MSC_VER)
    #define MY_LIB_API __declspec(dllexport) // Microsoft-only, if used as signature, breaks build on other platforms.
#elif defined(__GNUC__)
    #define MY_LIB_API __attribute__((visibility("default"))) // GCC
#else
    #define MY_LIB_API // Most compilers export all the symbols by default. We hope for the best here.
    #pragma warning Unknown dynamic link import/export semantics.
#endif

extern "C" MY_LIB_API int dllmain(
  const char* infile,
  const char* configfile,
  const char* outfile
)
{
    qStdOut() <<
          "The function 'dllmain' was called with:\n" <<
          "  infile: '" << infile << "'\n" <<
          "  configfile: '" << configfile << "'\n" <<
          "  outfile: '" << outfile << "'\n";

    //const char *argv[] = { "input.dbf", "output.dbf" };
    const char* argv[5];

    argv[0] = "program_name";
    argv[1] = "--config";
    argv[2] = configfile;
    argv[3] = infile;
    argv[4] = outfile;

    return main_batch(5, (char**) argv);
}

void writeBagrovTable(float bag_min, float bag_max, float bag_step,
                      float x_min, float x_max, float x_step)
{
    qStdOut() << "# Writing a table of bagrov values to stdout...\n";
    qStdOut() << "bag,x,y\n";

    Bagrov bagrov;

    float y = 0.0;
    float bag = bag_min;
    QString str;

    while(bag <= bag_max) {

        float x = x_min;

        while (x <= x_max) {
            y = bagrov.nbagro(bag, x);
            qStdOut() << str.sprintf("%0.1f", bag) << ","
                      << str.sprintf("%0.2f", x) << ","
                      << y << "\n";
            x += x_step;
        }

        bag += bag_step;
    }
}
