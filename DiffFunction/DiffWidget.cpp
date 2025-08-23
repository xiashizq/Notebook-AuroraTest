#include "diffwidget.h"
#include "myersdiff.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QFile>
#include <QFileDialog>
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
    QFont m_font("Menlo", fontsize);
    return font;
#else
    QFont m_font("DejaVu Sans Mono", fontsize);
    return font;
#endif
}


//DiffWidget::DiffWidget(QWidget* parent): QWidget(parent) {
//    auto* mainLayout = new QVBoxLayout(this);
//    auto* editLayout = new QHBoxLayout();

//    leftEdit = new QTextEdit(this);
//    rightEdit = new QTextEdit(this);
//    leftEdit->setPlaceholderText("左侧代码");
//    rightEdit->setPlaceholderText("右侧代码");
//    QFont font("Consolas", 11);
//    leftEdit->setFont(font);
//    rightEdit->setFont(font);

//    editLayout->addWidget(leftEdit);
//    editLayout->addWidget(rightEdit);
//    mainLayout->addLayout(editLayout);

//    diffBtn = new QPushButton("Myers对比", this);
//    mainLayout->addWidget(diffBtn);

//    resultEdit = new QTextEdit(this);
//    resultEdit->setReadOnly(true);
//    resultEdit->setFont(font);
//    mainLayout->addWidget(resultEdit);

//    connect(diffBtn, &QPushButton::clicked, this, &DiffWidget::doDiff);

//    connect(leftEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &DiffWidget::syncLeftScroll);
//    connect(rightEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &DiffWidget::syncRightScroll);
//}

//void DiffWidget::syncLeftScroll() {
//    if (syncingScroll) return;
//    syncingScroll = true;
//    rightEdit->verticalScrollBar()->setValue(leftEdit->verticalScrollBar()->value());
//    syncingScroll = false;
//}

//void DiffWidget::syncRightScroll() {
//    if (syncingScroll) return;
//    syncingScroll = true;
//    leftEdit->verticalScrollBar()->setValue(rightEdit->verticalScrollBar()->value());
//    syncingScroll = false;
//}

//void DiffWidget::doDiff() {
//    auto leftLines = leftEdit->toPlainText().split('\n');
//    auto rightLines = rightEdit->toPlainText().split('\n');
//    auto diffs = myersDiff(leftLines, rightLines);

//    QString html;
//    html += "<pre style='font-family:Consolas;font-size:11pt'>";
//    for (const auto& d : diffs) {
//        QString line;
//        if (d.type == Same) {
//            line = htmlEscape(d.left.isEmpty() ? d.right : d.left);
//            html += QString("<span>%1</span>\n").arg(line);
//        } else if (d.type == Added) {
//            line = htmlEscape(d.right);
//            html += QString("<span style='background:#c8ffc8'>+ %1</span>\n").arg(line);
//        } else if (d.type == Removed) {
//            line = htmlEscape(d.left);
//            html += QString("<span style='background:#ffc8c8'>- %1</span>\n").arg(line);
//        }
//    }

//    html += "</pre>";
//    resultEdit->setHtml(html);
//}


DiffWidget::DiffWidget(QWidget* parent): QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);

    // 文件选择控件
    auto* fileSelectLayout = new QHBoxLayout();
    leftFileEdit = new QLineEdit(this);
    leftFileEdit->setPlaceholderText("");
    leftFileBtn = new QPushButton("选择文件", this);
    fileSelectLayout->addWidget(leftFileEdit);
    fileSelectLayout->addWidget(leftFileBtn);

    rightFileEdit = new QLineEdit(this);
    rightFileEdit->setPlaceholderText("");
    rightFileBtn = new QPushButton("选择文件", this);
    fileSelectLayout->addWidget(rightFileEdit);
    fileSelectLayout->addWidget(rightFileBtn);

    mainLayout->addLayout(fileSelectLayout);

    // 编辑框布局
    auto* editLayout = new QHBoxLayout();
    leftEdit = new QTextEdit(this);
    rightEdit = new QTextEdit(this);
    leftEdit->setPlaceholderText("请输入旧字符/代码");
    rightEdit->setPlaceholderText("请输入新字符/代码");

    leftEdit->setFont(m_font(12));
    rightEdit->setFont(m_font(12));

    editLayout->addWidget(leftEdit);
    editLayout->addWidget(rightEdit);
    mainLayout->addLayout(editLayout);

    diffBtn = new QPushButton("Myers对比", this);
    mainLayout->addWidget(diffBtn);

    resultEdit = new QTextEdit(this);
    resultEdit->setReadOnly(true);
    resultEdit->setFont(m_font(12));
    mainLayout->addWidget(resultEdit);

    connect(diffBtn, &QPushButton::clicked, this, &DiffWidget::doDiff);
    connect(leftEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &DiffWidget::syncLeftScroll);
    connect(rightEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &DiffWidget::syncRightScroll);

    // 文件选择槽函数
    connect(leftFileBtn, &QPushButton::clicked, this, &DiffWidget::selectLeftFile);
    connect(rightFileBtn, &QPushButton::clicked, this, &DiffWidget::selectRightFile);
}

// 文件选择槽函数实现
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
    auto leftLines = leftEdit->toPlainText().split('\n');
    auto rightLines = rightEdit->toPlainText().split('\n');
    auto diffs = myersDiff(leftLines, rightLines);

    QString html;
    html += "<pre style='font-family:Consolas;font-size:11pt'>";
    for (const auto& d : diffs) {
        QString line;
        if (d.type == Same) {
            line = htmlEscape(d.left.isEmpty() ? d.right : d.left);
            html += QString("<span>%1</span>\n").arg(line);
        } else if (d.type == Added) {
            line = htmlEscape(d.right);
            html += QString("<span style='background:#c8ffc8'>+ %1</span>\n").arg(line);
        } else if (d.type == Removed) {
            line = htmlEscape(d.left);
            html += QString("<span style='background:#ffc8c8'>- %1</span>\n").arg(line);
        }
    }

    html += "</pre>";
    resultEdit->setHtml(html);
}
