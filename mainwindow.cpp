#include "MainWindow.h"

#include <Qsci/qsciscintilla.h>
#include "SqlParserWindow/SqlParserWindow.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QTextCursor>
#include <QColor>
#include <QIcon>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFont>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSettings>
#include <QCloseEvent>
#include <QRegularExpression>

#pragma execution_character_set("utf-8")
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{

    this->setWindowTitle("Notebook-AuroraTest");
    this->setWindowIcon(QIcon(":/iocn1.png"));
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setInterval(10 * 1000); // 每 60 秒自动保存一次
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSaveAll);
    m_autoSaveTimer->start();

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    layout->addWidget(m_tabWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    central->setLayout(layout);
    setCentralWidget(central);
    resize(900, 500);
    // 菜单栏
    QMenuBar *menuBar = this->menuBar();
    QMenu *fileMenu = menuBar->addMenu("文件");
    QAction *newWindowAction = new QAction("新建窗口", this);
    QAction *newTabAction = new QAction("新建", this);
    fileMenu->addAction(newWindowAction);
    fileMenu->addAction(newTabAction);
    QMenu *beautifyMenu = menuBar->addMenu("美化");
    connect(newWindowAction, &QAction::triggered, this, &MainWindow::openNewWindow);
    connect(newTabAction, &QAction::triggered, this, &MainWindow::addNewTabMneu);
    restoreLastSession();
}

void MainWindow::openNewWindow() {
    SqlParserWindow* sqlParserWindow = new SqlParserWindow();
    sqlParserWindow->show();
}


// 无参版本：新建空白 tab
void MainWindow::addNewTabMneu()
{
    addNewTab(QString()); // 调用带参版本，传空字符串
}


void MainWindow::addNewTab(const QString &filePath)
{
    auto *editor = new QsciScintilla(m_tabWidget);

    // ------------------------
    // 1. 设置编辑器样式（可提取为 setEditorStyle(editor)）
    // ------------------------
    QFont font;
#if defined(Q_OS_WIN)
    font = QFontDatabase().families().contains("Consolas") ? QFont("Consolas") : QFont();
#elif defined(Q_OS_LINUX)
    if (QFontDatabase().families().contains("Consolas"))
        font = QFont("Consolas");
    else if (QFontDatabase().families().contains("WenQuanYi Micro Hei"))
        font = QFont("WenQuanYi Micro Hei");
    else
        font = QFont();
#elif defined(Q_OS_MACOS)
    font = QFontDatabase().families().contains("PingFang SC") ? QFont("PingFang SC") : QFont();
#else
    font = QFont();
#endif
    font.setPointSize(12);
    editor->setFont(font);
    editor->setMarginsFont(QFont("Consolas", 10));
    editor->setMarginLineNumbers(0, true);
    editor->setMarginsForegroundColor(QColor("#666666"));
    editor->setMarginsBackgroundColor(QColor("#eeeeee"));
    editor->setWrapMode(QsciScintilla::WrapWord);

    // 更新行号宽度
    updateLineNumberWidth(editor);
    connect(editor, &QsciScintilla::linesChanged, this, [=]() {
        updateLineNumberWidth(editor);
    });

    // ------------------------
    // 2. 解析路径，判断是否为自动保存文件
    // ------------------------
    QString baseName;
    QString finalFilePath = filePath; // 实际用于加载的路径
    bool isAutoSaveRecovery = false;

    if (!filePath.isEmpty()) {
        QFileInfo fi(filePath);

        // 判断是否是自动保存文件（以 .autosave 结尾）
        if (fi.fileName().endsWith(".autosave", Qt::CaseInsensitive)) {
            // 是自动保存文件 → 去掉 .autosave 后缀作为显示名
            baseName = fi.fileName().left(fi.fileName().length() - 9); // 去掉 ".autosave"
            isAutoSaveRecovery = true;

            // ❗ 关键：恢复后，filePath 应为空，表示“未保存”
            finalFilePath = ""; // 不再指向 .autosave 文件
        } else {
            // 正常文件
            baseName = fi.fileName();
        }
    } else {
        // 新建空白 tab
        baseName = "未命名" + QString::number(m_untitledCount);
        m_untitledCount++;
    }

    // ------------------------
    // 3. 添加 tab
    // ------------------------
    int index = m_tabWidget->addTab(editor, baseName);
    m_tabWidget->setCurrentIndex(index);

    // ------------------------
    // 4. 初始化 TabInfo
    // ------------------------
    TabInfo info;
    info.editor = editor;
    info.filePath = finalFilePath;     // 可能为空（恢复场景）
    info.baseName = baseName;
    info.isModified = false;

    m_tabs.append(info);

    // ------------------------
    // 5. 连接 modificationChanged（更新 tab 标题）
    // ------------------------
    connect(editor, &QsciScintilla::modificationChanged, this, [this, editor](bool modified) {
        for (int i = 0; i < m_tabs.size(); ++i) {
            if (m_tabs[i].editor == editor) {
                m_tabs[i].isModified = modified;
                QString title = m_tabs[i].displayName();
                m_tabWidget->setTabText(i, modified ? title + " *" : title);
                break;
            }
        }
    });

    // ------------------------
    // 6. 加载文件内容（如果指定了路径）
    // ------------------------
    if (!filePath.isEmpty()) {
        QFile file(filePath); // 注意：用原始 filePath 读内容
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            editor->setText(file.readAll());
            editor->setModified(false); // 内容已加载，标记为未修改
            file.close();

            // 如果是自动保存恢复，可选提示用户
            if (isAutoSaveRecovery) {
                // 可选：显示小提示
                // m_statusBar->showMessage("已恢复: " + baseName, 3000);
            }
        } else {
            QMessageBox::warning(this, "打开失败", "无法读取文件：" + filePath);
            // 加载失败：关闭 tab
            m_tabWidget->removeTab(index);
            m_tabs.removeAt(m_tabs.size() - 1);
            editor->deleteLater();
            return;
        }
    } else {
        editor->setModified(false);
    }
}

void MainWindow::closeTab(int index) {
    if (!closeTabWithPrompt(index)) {
        return; // 取消
    }
    m_tabWidget->removeTab(index);
    m_tabs.removeAt(index);
}

bool MainWindow::saveFile(int index) {
    if (index < 0 || index >= m_tabs.size()) return false;
    TabInfo &info = m_tabs[index];
    QsciScintilla *editor = info.editor;
    QString filePath = info.filePath;
    if (filePath.isEmpty()) {
        // 首次保存，弹出文件对话框
        filePath = QFileDialog::getSaveFileName(this, "保存文件", info.displayName(), "SQL Files (*.sql);;Text Files (*.txt);;All Files (*)");
        if (filePath.isEmpty()) return false; // 用户取消
        info.filePath = filePath;
    }
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(editor->text().toUtf8());
        file.close();
        editor->setModified(false); // 标记为已保存
        m_tabWidget->setTabText(index, info.displayName()); // 去掉 *
        return true;
    } else {
        QMessageBox::warning(this, "保存失败", "无法保存文件：" + filePath);
        return false;
    }
}

void MainWindow::autoSaveAll()
{
    for (int i = 0; i < m_tabs.size(); ++i) {
        TabInfo &info = m_tabs[i];
        QsciScintilla *editor = info.editor;
        // 只有修改了才保存
        if (!editor->isModified()) continue;
        QString savePath;
        // 如果已有文件路径，直接保存到原路径
        if (!info.filePath.isEmpty()) {
            savePath = info.filePath;
        } else {
            // 未命名文件：保存到自动保存目录
            savePath = autoSavePathForTab(info);
        }
        QFileInfo fi(savePath);
        if (!fi.dir().exists()) {
            fi.dir().mkpath("."); // 创建目录
        }
        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(editor->text().toUtf8());
            file.close();
            // 可选：如果是从“未命名”自动保存，可以更新 info.filePath？
            // 不建议！因为用户没主动保存，我们只是“备份”
            // 所以不要改 info.filePath，只作为临时备份
            editor->setModified(false); // 标记为已保存（视觉上去掉 *）
            qDebug() << "Auto-saved:" << savePath;
        }
    }
}

QString MainWindow::autoSavePathForTab(const TabInfo &info)
{
    QString dir = autoSaveDir();
    QDir().mkpath(dir); // 确保目录存在
    QString baseName;
    if (!info.filePath.isEmpty()) {
        // 有路径：使用原文件名（含扩展名）
        baseName = QFileInfo(info.filePath).fileName();
    } else {
        // 未命名文件：使用 baseName（如“未命名1”）
        baseName = info.baseName;
    }
    // ✅ 只加 .autosave，不加 .sql
    return dir + "/" + baseName + ".autosave";
}

QString MainWindow::autoSaveDir()
{
    // 使用临时目录：C:/Users/.../AppData/Local/Temp/YourApp/auto_save
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/Notebook-AuroraTest/auto_save";
}

void MainWindow::updateLineNumberWidth(QsciScintilla *textEdit) {
    int lines = textEdit->lines();
    int digits = QString::number(lines).length();
    QFont font("Consolas", 10);
    QFontMetrics metrics(font);
    int marginWidth = metrics.horizontalAdvance(QString(digits, '9')) + 10;
    textEdit->setMarginsFont(font);
    textEdit->setMarginWidth(0, marginWidth);
}

int MainWindow::extractNumberFromUntitled(const QString &filename)
{
    // 匹配 "未命名123" 这种格式
    QRegularExpression re(R"(未命名(\d+))");
    QRegularExpressionMatch match = re.match(filename);
    if (match.hasMatch()) {
        return match.captured(1).toInt();
    }
    return 0;
}


QStringList MainWindow::getLastOpenFiles()
{
    QSettings settings("YourCompany", "Notebook-AuroraTest");
    return settings.value("session/lastFiles").toStringList();
}

void MainWindow::setLastOpenFiles(const QStringList &files)
{
    QSettings settings("YourCompany", "Notebook-AuroraTest");
    settings.setValue("session/lastFiles", files);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    autoSaveAll();
    // 询问是否保存修改的文件
    for (int i = 0; i < m_tabs.size(); ++i) {
        const TabInfo &info = m_tabs[i];
        QsciScintilla *editor = info.editor;
        // 只对“有文件路径”的 tab 询问是否保存
        if (!info.filePath.isEmpty() && editor->isModified()) {
            if (!closeTabWithPrompt(i)) {
                event->ignore();
                return;
            }
        }
    }
    // ✅ 保存当前所有 tab 的路径（包括 auto-save 路径）
    QStringList files;
    for (const TabInfo &info : m_tabs) {
        if (!info.filePath.isEmpty()) {
            files << info.filePath;
        } else {
            // 未命名文件：保存其 auto-save 路径，下次恢复
            files << autoSavePathForTab(info);
        }
    }
    setLastOpenFiles(files);
    event->accept();
}

void MainWindow::restoreLastSession()
{
    QStringList files = getLastOpenFiles(); // 复用现有 API
    int maxNum = 0;
    // 从 lastFiles 中找所有 "未命名X.autosave"，取最大 X
    for (const QString &path : files) {
        QFileInfo fi(path);
        QString baseName = fi.completeBaseName(); // 去掉 .autosave

        if (baseName.startsWith("未命名")) {
            int num = extractNumberFromUntitled(baseName);
            if (num > maxNum) {
                maxNum = num;
            }
        }
    }
    // ✅ 设置下一个未命名文件的编号
    m_untitledCount = maxNum + 1;

    // 开始恢复 tab
    if (files.isEmpty()) {
        addNewTabMneu();
    } else {
        for (const QString &path : files) {
            addNewTab(path);
        }
    }
}


bool MainWindow::closeTabWithPrompt(int index)
{
    if (index < 0 || index >= m_tabs.size()) return true;
    TabInfo &info = m_tabs[index];
    QsciScintilla *editor = info.editor;
    if (editor->isModified()) {
        QString title = info.displayName().isEmpty() ? "未命名" : info.displayName();
                                                                         QMessageBox::StandardButton ret = QMessageBox::question(
                                                                         this,
                                                                         "保存更改",
                                                                         QString("文件“%1”已修改，是否保存？").arg(title),
                                                                         QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
                                                                         );

        if (ret == QMessageBox::Save) {
            if (!saveFile(index)) {
                return false; // 保存失败或取消，不关闭
            }
        } else if (ret == QMessageBox::Cancel) {
            return false; // 取消关闭
        }
        // Discard：直接关闭
    }
    return true; // 可以关闭
}


