/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#include <QAction>
#include <QApplication>
#include <QCommandLineParser>
#include <QEventLoop>
#include <QFont>
#include <QFile>
#include <QFileDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressDialog>
#include <QString>
#include <QTextStream>
#include <QWidget>

#include "calculation.h"
#include "constants.h"
#include "dbaseReader.h"
#include "helpers.h"
#include "initvalues.h"
#include "mainwindow.h"

MainWindow::MainWindow(QApplication* app, QCommandLineParser* arguments):
    QMainWindow(),
    programName(tr(PROGRAM_NAME)),
    userStop(false),
    calc(0),
    app(app),
    arguments(arguments),
    folder("/")
{
    // Define action: Compute File
    openAct = new QAction(tr("&Compute File"), this);
    openAct->setShortcut(tr("Ctrl+C"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(computeFile()));

    // Define action: About
    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setShortcut(tr("Ctrl+A"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    // Add actions to the menu bar
    menuBar()->addAction(openAct);
    menuBar()->addAction(aboutAct);

    // Set window title and size
    setWindowTitle(programName);
    resize(350, 150);

    widget = new QWidget();
    setCentralWidget(widget);

    textfield = new QLabel("Willkommen...", widget);
    textfield->setMargin(4);
    textfield->setFont(QFont("Arial", 8, QFont::Bold));

    progress = new QProgressDialog( "Lese Datei.", "Abbrechen", 0, 50, this, 0);
    progress->setWindowTitle(programName);
    progress->setModal(true);
    progress->setMinimumDuration (0);
    connect(progress, SIGNAL(canceled()), this, SLOT(userCancel()));
}

MainWindow::~MainWindow()
{
    delete textfield;
    delete openAct;
    delete aboutAct;
    delete progress;
    delete widget;
}

void MainWindow::processEvent(int i, QString string)
{
    progress->setValue(i);
    progress->setLabelText(string);
    app->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void MainWindow::userCancel()
{
    userStop = true;
    setText("Berechnungen abgebrochen.");

    if (calc != 0) {
        calc->stopProcessing();
    }

    app->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void MainWindow::about()
{
    QMessageBox::about(
        this,
        tr("About ") + programName,
        tr("Claus & Meiko Rachimow\nCopyright 2009")
    );
}

void MainWindow::setText(QString info)
{
    textfield->setText(info);
    textfield->adjustSize();
}

void MainWindow::critical(QString string)
{
    progress->close();
    QMessageBox::critical(this, programName, string);
}

void MainWindow::warning(QString string)
{
    QMessageBox::warning(this, programName, string);
}

void MainWindow::computeFile()
{
    QString inputFileName = Helpers::positionalArgOrNULL(arguments, 0);
    QString outputFileName = Helpers::positionalArgOrNULL(arguments, 1);
    QString configFileName = arguments->value("config");
    QString protokollFileName;

    userStop = false;

    // Select input file
    inputFileName = QFileDialog::getOpenFileName(
        this,
        "Daten einlesen von...",
        folder,
        Helpers::patternDbfFile()
    );

    // Return if no file was selected
    if (inputFileName.isNull()) {
        return;
    }

    // Select configuration file
    configFileName = QString("config.xml");

    // Open a DBASE File
    DbaseReader dbReader(inputFileName);

    setText("Lese Quelldatei...");
    repaint();

    if (! dbReader.checkAndRead()) {
        critical(dbReader.getFullError());
        return;
    }

    // Update default initial values with values given in config.xml
    InitValues initValues;
    QString errorMessage = InitValues::updateFromConfig(initValues, configFileName);

    if (! errorMessage.isEmpty()) {
        warning(errorMessage);
    }

    setText("Quelldatei eingelesen, waehlen Sie eine Zieldatei...");

    // Select output file
    outputFileName = QFileDialog::getSaveFileName(
        this,
        "Ergebnisse schreiben nach...",
        Helpers::defaultOutputFileName(inputFileName),
        Helpers::patternDbfFile()
    );

    // Return if no output file was selected
    if (outputFileName == NULL) {
        setText("Willkommen...");
        return;
    }

    setText("Bitte Warten...");
    processEvent(0, "Lese Datei.");

    // Protokoll
    protokollFileName = Helpers::defaultLogFileName(outputFileName);

    QFile protokollFile(protokollFileName);

    if (! protokollFile.open(QFile::WriteOnly)) {
        critical(
            "Konnte Datei: " + Helpers::singleQuote(protokollFileName) +
            " nicht oeffnen.\n" + protokollFile.error()
        );
        return;
    }

    QTextStream protokollStream(&protokollFile);

    // Start the Calculation
    protokollStream << "Start der Berechnung " + Helpers::nowString() + "\r\n";

    // Create calculator object
    calc = new Calculation(dbReader, initValues, protokollStream);

    connect(
        calc,
        SIGNAL(processSignal(int, QString)),
        this,
        SLOT(processEvent(int, QString))
    );

    // Do the calculation
    bool success = calc->calculate(outputFileName);

    // Report about success or failure
    if (success) {

        if (userStop) {
            reportCancelled(protokollStream);
        }
        else {
            reportSuccess(calc, protokollStream, outputFileName, protokollFileName);
        }
    }
    else {
        critical(calc->getError());
    }

    delete calc;
    calc = 0;
    protokollFile.close();
}

void MainWindow::reportSuccess(
    Calculation* calc,
    QTextStream &protokollStream,
    QString outFile,
    QString protokollFileName
)
{
    QString protCount;
    QString nutzungIstNull;
    QString keineFlaechenAngegeben;
    QString readRecCount;
    QString writeRecCount;

    Counters counters = calc->getCounters();

    protCount.setNum(counters.recordsProtocol);
    nutzungIstNull.setNum(counters.noUsageGiven);
    keineFlaechenAngegeben.setNum(counters.noAreaGiven);
    readRecCount.setNum(counters.recordsRead);
    writeRecCount.setNum(counters.recordsWritten);

    setText(
        "Berechnungen mit " + protCount + " Fehlern beendet.\n" +
        "Eingelesene Records: " + readRecCount +"\n" +
        "Geschriebene Records: " + writeRecCount +"\n" +
        "Ergebnisse in Datei: '" + outFile + "' geschrieben.\n" +
        "Protokoll in Datei: '" + protokollFileName + "' geschrieben."
    );

    protokollStream << "\r\nBei der Berechnung traten " << protCount <<
        " Fehler auf.\r\n";

    if (counters.noAreaGiven != 0) {
        protokollStream << "\r\nBei " + keineFlaechenAngegeben +
            " Flaechen deren Wert 0 war wurde 100 eingesetzt.\r\n";
    }

    if (counters.noUsageGiven != 0) {
        protokollStream << "\r\nBei " + nutzungIstNull +
            " Records war die Nutzung 0, diese wurden ignoriert.\r\n";
    }

    if (counters.irrigationForcedToZero != 0) {
        protokollStream << "\r\nBei " << counters.irrigationForcedToZero <<
            " Records wurde BER==0 erzwungen.\r\n";
    }

    protokollStream << "\r\nEingelesene Records: " + readRecCount + "\r\n";
    protokollStream << "\r\nGeschriebene Records: " + writeRecCount + "\r\n";
    protokollStream << "\r\nEnde der Berechnung " + Helpers::nowString() + "\r\n";
}

void MainWindow::reportCancelled(QTextStream & /*protokollStream*/)
{
    setText("Die Berechnungen wurden durch den Benutzer abgebrochen.");

    /*
    protokollStream << "\r\nNutzer-Unterbrechung der Berechnungen " +
        nowString() + "\r\n";
    */
}
