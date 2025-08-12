#pragma once
#include <QSyntaxHighlighter>
#include <QColor>

class SqlHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    SqlHighlighter(QTextDocument *parent = nullptr);

    void setHighlightRegion(int fieldStart, int fieldEnd, int valueStart, int valueEnd, QColor color);
    void clearHighlight();

protected:
    void highlightBlock(const QString &text) override;

private:
    int fieldStart_, fieldEnd_, valueStart_, valueEnd_;
    QColor color_;
    bool hasRegion_;
};
