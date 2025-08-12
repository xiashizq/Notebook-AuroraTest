#include "SqlHighlighter.h"
#include <QTextCharFormat>

SqlHighlighter::SqlHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent), hasRegion_(false) {}

void SqlHighlighter::setHighlightRegion(int fieldStart, int fieldEnd, int valueStart, int valueEnd, QColor color) {
    fieldStart_ = fieldStart;
    fieldEnd_ = fieldEnd;
    valueStart_ = valueStart;
    valueEnd_ = valueEnd;
    color_ = color;
    hasRegion_ = true;
    rehighlight();
}

void SqlHighlighter::clearHighlight() {
    hasRegion_ = false;
    rehighlight();
}
//文字变色
//void SqlHighlighter::highlightBlock(const QString &text) {
//    if (!hasRegion_) return;
//    int blockStart = currentBlock().position();
//    int blockEnd = blockStart + text.length();
//    QTextCharFormat fmt;
//    fmt.setForeground(color_);
//    // 高亮字段
//    if (fieldStart_ < blockEnd && fieldEnd_ > blockStart) {
//        int start = qMax(0, fieldStart_ - blockStart);
//        int len = qMin(fieldEnd_, blockEnd) - qMax(fieldStart_, blockStart);
//        if (len > 0) setFormat(start, len, fmt);
//    }
//    // 高亮值
//    if (valueStart_ < blockEnd && valueEnd_ > blockStart) {
//        int start = qMax(0, valueStart_ - blockStart);
//        int len = qMin(valueEnd_, blockEnd) - qMax(valueStart_, blockStart);
//        if (len > 0) setFormat(start, len, fmt);
//    }
//}

//背景变色
void SqlHighlighter::highlightBlock(const QString &text) {
    if (!hasRegion_) return;
    int blockStart = currentBlock().position();
    int blockEnd = blockStart + text.length();
    QTextCharFormat fmt;
    fmt.setBackground(color_);  // 这一行由 setForeground 改为 setBackground
    // 高亮字段
    if (fieldStart_ < blockEnd && fieldEnd_ > blockStart) {
        int start = qMax(0, fieldStart_ - blockStart);
        int len = qMin(fieldEnd_, blockEnd) - qMax(fieldStart_, blockStart);
        if (len > 0) setFormat(start, len, fmt);
    }
    // 高亮值
    if (valueStart_ < blockEnd && valueEnd_ > blockStart) {
        int start = qMax(0, valueStart_ - blockStart);
        int len = qMin(valueEnd_, blockEnd) - qMax(valueStart_, blockStart);
        if (len > 0) setFormat(start, len, fmt);
    }
}
