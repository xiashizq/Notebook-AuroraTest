#pragma once
#include <QStringList>
#include <QString>
#include <QList>
#include <QVector>

namespace Diff {

enum class Type {
    Same,
    Added,
    Removed
};

struct Line {
    QString left;
    QString right;
    Type type;

    Line() = default;
    Line(const QString& l, const QString& r, Type t)
        : left(l), right(r), type(t) {}
};

} // namespace Diff

// 声明函数
QList<Diff::Line> myersDiff(const QStringList& a, const QStringList& b);
