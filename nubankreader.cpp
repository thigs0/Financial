#include "nubankreader.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

static QString extractTo(const QString &text) {
    QString t = text.trimmed().toLower();

    if (t.contains("débito") || t.contains("pix") || t.contains("celular")) {
        QStringList parts = t.split("-", Qt::SkipEmptyParts);
        return parts.size() > 1 ? parts[1].trimmed() : "";
    } else if (t.contains("fatura")) {
        return "nubank";
    } else {
        return "";
    }
}

QVector<NubankEntry> read_nubank_csv(const QString &path) {
    QVector<NubankEntry> entries;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Empty file" << path;
        return entries;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) break;

        QStringList cols = line.split(',');
        for (int i = 0; i < cols.size(); ++i) {
            cols[i] = cols[i].trimmed();
            if (cols[i].startsWith('"') && cols[i].endsWith('"')) {
                cols[i] = cols[i].mid(1, cols[i].length() - 2);
            }
            
        }
        if (cols.size() < 4) {
            qWarning() << "Linha inválida (menos de 4 colunas):" << line;
            continue;
        }

        QString dateStr = cols[0].trimmed();
        QString valueStr = cols[1].trimmed();
        QString idStr = cols[2].trimmed();
        QString rawDescription = cols[3].trimmed();

        QDate date = QDate::fromString(dateStr, "dd/MM/yyyy");
        if (!date.isValid()) {
            qWarning() << "Data inválida:" << dateStr << "na linha:" << line;
            continue;
        }

        bool ok = false;
        double value = valueStr.toDouble(&ok);
        if (!ok) {
            qWarning() << "Valor inválido:" << valueStr << "na linha:" << line;
            continue;
        }

        QString type_transfer = rawDescription.section("-", 0, 0).trimmed();
        QString description = rawDescription.section("-", 1).trimmed();
        QString to = extractTo(rawDescription);

        entries.append({
            date,
            value,
            idStr,
            type_transfer,
            description,
            to
        });
    }
    return entries;
}
