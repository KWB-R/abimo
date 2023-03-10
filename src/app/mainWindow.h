/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QApplication>
#include <QCommandLineParser>
#include <QLabel>
#include <QMainWindow>
#include <QObject>
#include <QProgressDialog>
#include <QString>
#include <QTextStream>
#include <QWidget>

#include "calculation.h"
#include "initValues.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QApplication*, QCommandLineParser* = NULL);
    ~MainWindow();
    static QString updateInitialValues(InitValues &, QString);

private slots:
    void processEvent(int, QString);
    void about();
    void computeFile();
    void userCancel();

private:
    const QString programName;
    void setText(QString);
    void critical(QString);
    void warning(QString);
    void reportSuccess(Calculation*, QTextStream&, QString, QString);
    void reportCancelled(QTextStream&);
    void reportLine(QTextStream&, QString);
    void reportNumberCasesIfAny(QTextStream&, int, QString);
    QAction *openAct;
    QAction *aboutAct;
    QLabel *textfield;
    QProgressDialog * progress;
    bool userStop;
    Calculation* calc;
    QApplication* app;
    QCommandLineParser* arguments;
    QString folder;
    QWidget *widget;
};

#endif
