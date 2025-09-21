#ifndef NUBANKREADER_H
#define NUBANKREADER_H

#include <QString>
#include <QDate>
#include <QVector>

struct NubankEntry {
    QDate date;
    double value;
    QString id;
    QString type_transfer;
    QString description;
    QString to;
};

QVector<NubankEntry> read_nubank_csv(const QString &path);

#endif // NUBANKREADER_H
