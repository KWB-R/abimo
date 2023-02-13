#ifndef MAIN_H
#define MAIN_H

//#include <QString>
//void calculate(QString inputFile, QString configFile, QString outputFile);

void writeBagrovTable(
    float bag_min = 0.1F,
    float bag_max = 8.0F,
    float bag_step = 0.1F,
    float x_min = 0.0F,
    float x_max = 3.0F,
    float x_step = 0.05F
);

#endif // MAIN_H
