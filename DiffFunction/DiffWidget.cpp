#include "diffwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QFile>
#include <QFileDialog>
#include <QtConcurrent>
#include <QSplitter>
#include <QGroupBox>
#pragma execution_character_set("utf-8")


QString htmlEscape(const QString& str) {
    QString out = str;
    out.replace('&', "&amp;");
    out.replace('<', "&lt;");
    out.replace('>', "&gt;");
    out.replace('"', "&quot;");
    out.replace('\'', "&#39;");
    return out;
}

QFont DiffWidget::m_font(const int &fontsize){
#ifdef Q_OS_WIN
    QFont font("Consolas", fontsize);
    return font;
#elif defined(Q_OS_MAC)
    QFont font("Menlo", fontsize);  // 修复：变量名是 font
    return font;
#else
    QFont font("DejaVu Sans Mono", fontsize);
    return font;
#endif
}

DiffWidget::DiffWidget(QWidget* parent): QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);

    // 文件选择分组
    auto* fileGroup = new QGroupBox("文件选择", this);
    auto* fileSelectLayout = new QHBoxLayout(fileGroup);
    leftFileEdit = new QLineEdit(this);
    leftFileBtn = new QPushButton("选择文件", this);
    rightFileEdit = new QLineEdit(this);
    rightFileBtn = new QPushButton("选择文件", this);
    fileSelectLayout->addWidget(leftFileEdit);
    fileSelectLayout->addWidget(leftFileBtn);
    fileSelectLayout->addWidget(rightFileEdit);
    fileSelectLayout->addWidget(rightFileBtn);

    mainLayout->addWidget(fileGroup);

    // 编辑器区（用 QSplitter）
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    auto* leftBox = new QGroupBox("旧版本", splitter);
    auto* rightBox = new QGroupBox("新版本", splitter);

    auto* leftLayout = new QVBoxLayout(leftBox);
    leftEdit = new QTextEdit(this);
    leftEdit->setFont(m_font(11));
    leftEdit->setPlaceholderText("可以选择文档对比，也可以直接输入文本");
    leftLayout->addWidget(leftEdit);

    auto* rightLayout = new QVBoxLayout(rightBox);
    rightEdit = new QTextEdit(this);
    rightEdit->setFont(m_font(11));
    rightEdit->setPlaceholderText("可以选择文档对比，也可以直接输入文本");
    rightLayout->addWidget(rightEdit);

    splitter->addWidget(leftBox);
    splitter->addWidget(rightBox);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter);

    // 操作按钮（工具栏风格）
    auto* toolLayout = new QHBoxLayout();
    diffBtn = new QPushButton("对比", this);
    toolLayout->addStretch();
    toolLayout->addWidget(diffBtn);
    mainLayout->addLayout(toolLayout);

    // 结果显示（用 Tab 分页）
    auto* tab = new QTabWidget(this);
    resultEdit = new QTextEdit(this);
    resultEdit->setReadOnly(true);
    resultEdit->setFont(m_font(11));
    tab->addTab(resultEdit, "文本结果");
    mainLayout->addWidget(tab);

    // 信号槽
    connect(diffBtn, &QPushButton::clicked, this, &DiffWidget::doDiff);
    connect(leftEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &DiffWidget::syncLeftScroll);
    connect(rightEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &DiffWidget::syncRightScroll);
    connect(leftFileBtn, &QPushButton::clicked, this, &DiffWidget::selectLeftFile);
    connect(rightFileBtn, &QPushButton::clicked, this, &DiffWidget::selectRightFile);
    connect(&watcher, &QFutureWatcher<QString>::finished, this, &DiffWidget::onDiffFinished);

    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);
}


void DiffWidget::setFiles(const QString& srcFile, const QString& dstFile){
    leftEdit->hide();
    rightEdit->hide();
    diffBtn->hide();
    leftFileEdit->setDisabled(true);
    rightFileEdit->setDisabled(true);
    leftFileBtn->setDisabled(true);
    rightFileBtn->setDisabled(true);
    if (!srcFile.isEmpty()) {
        leftFileEdit->setText(srcFile);
        QFile file(srcFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            leftEdit->setPlainText(QString::fromUtf8(file.readAll()));
        }
    }

    if (!dstFile.isEmpty()) {
        rightFileEdit->setText(dstFile);
        QFile file(dstFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            rightEdit->setPlainText(QString::fromUtf8(file.readAll()));
        }
    }
    doDiff();
}

void DiffWidget::selectLeftFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "选择左侧文件");
    if (!filePath.isEmpty()) {
        leftFileEdit->setText(filePath);
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            leftEdit->setPlainText(QString::fromUtf8(file.readAll()));
        }
    }
}

void DiffWidget::selectRightFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "选择右侧文件");
    if (!filePath.isEmpty()) {
        rightFileEdit->setText(filePath);
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            rightEdit->setPlainText(QString::fromUtf8(file.readAll()));
        }
    }
}

void DiffWidget::syncLeftScroll() {
    if (syncingScroll) return;
    syncingScroll = true;
    rightEdit->verticalScrollBar()->setValue(leftEdit->verticalScrollBar()->value());
    syncingScroll = false;
}

void DiffWidget::syncRightScroll() {
    if (syncingScroll) return;
    syncingScroll = true;
    leftEdit->verticalScrollBar()->setValue(rightEdit->verticalScrollBar()->value());
    syncingScroll = false;
}

void DiffWidget::doDiff() {
    QString leftText = leftEdit->toPlainText();
    QString rightText = rightEdit->toPlainText();

    // 捕获值，传入线程
    QFuture<QString> future = QtConcurrent::run([=]() -> QString {
        QStringList leftLines = leftText.split('\n');
        QStringList rightLines = rightText.split('\n');
        QList<Diff::Line> diffs = myersDiff(leftLines, rightLines);
        return generateDiffHtml(diffs);  // 调用成员函数
    });

    watcher.setFuture(future);
}

void DiffWidget::onDiffFinished() {
    QString html = watcher.future().result();
    resultEdit->setHtml(html);
}

//实现 generateDiffHtml（参数是 QList<DiffLine>）
QString DiffWidget::generateDiffHtml(const QList<Diff::Line>& diffs) {
    QString html;
    html += "<pre style='font-family:Consolas;font-size:11pt'>";
    for (const auto& d : diffs) {
        if (d.type == Diff::Type::Same) {
            QString line = d.left.isEmpty() ? d.right : d.left;
            html += QString("<span>%1</span>\n").arg(htmlEscape(line));
        } else if (d.type == Diff::Type::Added) {
            html += QString("<span style='background:#c8ffc8'>+ %1</span>\n")
                        .arg(htmlEscape(d.right));
        } else if (d.type == Diff::Type::Removed) {
            html += QString("<span style='background:#ffc8c8'>- %1</span>\n")
                        .arg(htmlEscape(d.left));
        }
    }
    html += "</pre>";
    return html;
}
