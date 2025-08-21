#pragma once
#include <QStringList>
#include <QString>
#include <QList>

enum DiffType { Same, Added, Removed };

struct DiffLine {
    QString left;
    QString right;
    DiffType type;
};

QList<DiffLine> myersDiff(const QStringList& a, const QStringList& b);
