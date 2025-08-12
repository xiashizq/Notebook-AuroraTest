#ifndef SQLPARSERWINDOW_H
#define SQLPARSERWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
class CodeEditor;
class SqlHighlighter;
class SqlLexer;

class SqlParserWindow : public QWidget {
    Q_OBJECT

public:
    explicit SqlParserWindow(QWidget *parent = nullptr);
private slots:
    void onCursorChanged();

private:
    CodeEditor *textEdit;
    SqlHighlighter *highlighter;
};

#endif // SQLPARSERWINDOW_H
