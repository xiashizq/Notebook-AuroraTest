#include "SqlParser.h"
#include <QRegularExpression>

QList<FieldValuePos> SqlParser::parseInsertWithPos(const QString &sql) {
    QList<FieldValuePos> result;
    QRegularExpression re(R"(INSERT\s+(?:INTO\s+)?(?:\w+\.)?\w+\s*\(([^)]+)\)\s*VALUES\s*\(([^)]+)\))", QRegularExpression::CaseInsensitiveOption);
    auto m = re.match(sql);
    if (!m.hasMatch()) return result;
    QString fieldsStr = m.captured(1);
    QString valuesStr = m.captured(2);

    // 字段和值列表
    QStringList fields = fieldsStr.split(",", Qt::SkipEmptyParts);
    QStringList values;
    // 处理带引号和逗号的值
    QString valbuf;
    bool inQuote = false;
    for (int i = 0; i < valuesStr.length(); ++i) {
        QChar c = valuesStr[i];
        if (c == '\'' && (i == 0 || valuesStr[i-1] != '\\')) inQuote = !inQuote;
        if (c == ',' && !inQuote) {
            values << valbuf.trimmed();
            valbuf.clear();
        } else {
            valbuf += c;
        }
    }
    if (!valbuf.isEmpty()) values << valbuf.trimmed();

    int minCount = qMin(fields.size(), values.size());

    // 记录每个字段/值的区间
    int fieldsPos = sql.indexOf(fieldsStr);
    int valuesPos = sql.indexOf(valuesStr);
    int fOffset = 0, vOffset = 0;

    for (int i = 0; i < minCount; ++i) {
        QString field = fields[i].trimmed();
        QString value = values[i].trimmed();
        // 原始带引号的值
        QString rawValue = value;
        if (value.startsWith("'") && value.endsWith("'") && value.length() > 1)
            value = value.mid(1, value.length()-2);

        // 定位字段
        int fieldStart = fieldsStr.indexOf(field, fOffset);
        fieldStart = (fieldStart == -1 ? fOffset : fieldStart);
        int fieldEnd = fieldStart + field.length();
        fOffset = fieldEnd;

        // 定位值
        int valueStart = valuesStr.indexOf(rawValue, vOffset);
        valueStart = (valueStart == -1 ? vOffset : valueStart);
        int valueEnd = valueStart + rawValue.length();
        vOffset = valueEnd;

        FieldValuePos fvp;
        fvp.field = field;
        fvp.value = value;
        fvp.fieldStart = fieldsPos + fieldStart;
        fvp.fieldEnd = fieldsPos + fieldEnd;
        fvp.valueStart = valuesPos + valueStart;
        fvp.valueEnd = valuesPos + valueEnd;
        result << fvp;
    }
    return result;
}
