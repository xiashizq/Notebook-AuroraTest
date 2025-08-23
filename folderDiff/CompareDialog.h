#ifndef COMPAREDIALOG_H
#define COMPAREDIALOG_H

#include <QDialog>

class FolderCompareWidget; // 前向声明

class CompareDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompareDialog(QWidget *parent = nullptr);
    ~CompareDialog();

private:
    FolderCompareWidget *m_folderCompareWidget;
};

#endif // COMPAREDIALOG_H
