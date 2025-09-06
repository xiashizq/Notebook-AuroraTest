#pragma once
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QFutureWatcher>

#include "myersdiff.h"


class DiffWidget : public QWidget {
    Q_OBJECT
public:
    explicit DiffWidget(QWidget* parent = nullptr);
    void setFiles(const QString& srcFile, const QString& dstFile);

private slots:
    void syncLeftScroll();
    void syncRightScroll();
    void selectLeftFile();
    void selectRightFile();
    QFont m_font(const int &fontsize);
    void onDiffFinished();   // 后台任务完成时调用

private:
    void doDiff();

    // 声明 generateDiffHtml 函数（静态或成员）
    QString generateDiffHtml(const QList<Diff::Line>& diffs);  // ⬅️ 参数必须是 QList！

    QTextEdit* leftEdit;
    QTextEdit* rightEdit;
    QTextEdit* resultEdit;
    QPushButton* diffBtn;

    QLineEdit* leftFileEdit;
    QPushButton* leftFileBtn;
    QLineEdit* rightFileEdit;
    QPushButton* rightFileBtn;

    QFutureWatcher<QString> watcher;

    bool syncingScroll = false;
};
