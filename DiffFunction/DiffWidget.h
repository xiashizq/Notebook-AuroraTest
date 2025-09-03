#pragma once
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>

class DiffWidget : public QWidget {
    Q_OBJECT
public:
    explicit DiffWidget(QWidget* parent = nullptr);
    void setFiles(const QString& srcFile, const QString& dstFile);

private slots:
    void doDiff();
    void syncLeftScroll();
    void syncRightScroll();

    void selectLeftFile();
    void selectRightFile();
    QFont m_font(const int &fontsize);

private:
    QTextEdit* leftEdit;
    QTextEdit* rightEdit;
    QTextEdit* resultEdit;
    QPushButton* diffBtn;

    QLineEdit* leftFileEdit;
    QPushButton* leftFileBtn;
    QLineEdit* rightFileEdit;
    QPushButton* rightFileBtn;

    bool syncingScroll = false;
};
