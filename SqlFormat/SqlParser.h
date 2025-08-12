#pragma once
#include <QString>
#include <QList>

struct FieldValuePos {
    QString field;
    QString value;
    int fieldStart;
    int fieldEnd;
    int valueStart;
    int valueEnd;
};

class SqlParser {
public:
    static QList<FieldValuePos> parseInsertWithPos(const QString &sql);
};
