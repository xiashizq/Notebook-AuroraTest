#include "comparedialog.h"
#include "FolderCompareWidget.h"
#include <QVBoxLayout>
#include <QPushButton>

CompareDialog::CompareDialog(QWidget *parent)
    : QDialog(parent)
{
    // 设置窗口标题和大小
    this->setWindowTitle(tr("Compare Tool"));
    this->resize(1200, 600);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    // 创建 CompareWidget
    m_folderCompareWidget = new FolderCompareWidget(this);

    // 创建布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_folderCompareWidget);

    // 设置对话框为非模态（默认），也可以用 setModal(true) 变成模态
    // this->setModal(false); // 非模态：可与主窗口交互
}

CompareDialog::~CompareDialog()
{
    // m_compareWidget 会被自动 delete（因为 parent 是 this）
}
