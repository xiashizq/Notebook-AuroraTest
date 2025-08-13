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

    // 添加“打开”菜单项
    QAction *openAction = fileMenu->addAction("打开...");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFileNoPar);

    // 添加“最近打开”菜单
    setupRecentFilesMenu(); // 之前定义的函数

    QAction *saveAct = new QAction(this);
    saveAct->setShortcut(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &MainWindow::onSave);
    this->addAction(saveAct); // 添加到窗口，否则快捷键不生效

    QAction *saveAsAct = new QAction(this);
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::onSaveAs);
    this->addAction(saveAsAct);

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
    editor->setWrapMode(QsciScintilla::WrapCharacter);

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
        // 判断是否是 .autosave 恢复
        if (fi.fileName().endsWith(".autosave", Qt::CaseInsensitive)) {
            baseName = fi.fileName().left(fi.fileName().length() - 9);
            isAutoSaveRecovery = true;
            finalFilePath = ""; // 标记为未命名
        } else {
            baseName = fi.fileName();
            finalFilePath = filePath;
        }
    } else {
        baseName = "未命名" + QString::number(m_untitledCount++);
        finalFilePath = "";
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
    info.filePath = finalFilePath;
    info.baseName = baseName;
    info.autoSavePath = autoSavePathForTab(info);
    info.isModified = false; // 初始未修改

    m_tabs.append(info);

    // ------------------------
    // 5. 连接 modificationChanged（更新 tab 标题）
    // ------------------------
    connect(editor, &QsciScintilla::modificationChanged, this, [this, editor](bool modified) {
        for (int i = 0; i < m_tabs.size(); ++i) {
            if (m_tabs[i].editor == editor) {
                m_tabs[i].isModified = modified;

                // ✅ 正确逻辑：只要修改了，就显示 *
                QString title = m_tabs[i].baseName;
                if (modified) {
                    m_tabWidget->setTabText(i, title + " *");
                } else {
                    m_tabWidget->setTabText(i, title);
                }
                break;
            }
        }
    });

    // ------------------------
    // 6. 加载文件内容（如果指定了路径）
    // ------------------------
    if (filePath.isEmpty()) {
        // 1. 确保 .autosave 所在目录存在
        QFileInfo fi(info.autoSavePath);
        QDir().mkpath(fi.path()); // 创建 temp/ 目录（如果不存在）
        // 2. 创建空的 .autosave 文件
        QFile file(info.autoSavePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(editor->text().toUtf8()); // 当前内容（可能为空）
            file.close();
        } else {
            qWarning() << "无法创建自动保存文件:" << info.autoSavePath;
        }
        // 3. 将 .autosave 路径加入“最近打开文件”
        QStringList lastFiles = getLastOpenFiles();
        lastFiles.removeOne(info.autoSavePath); // 避免重复
        lastFiles.prepend(info.autoSavePath);
        while (lastFiles.size() > 10) {
            lastFiles.removeLast();
        }
        setLastOpenFiles(lastFiles);
        updateRecentFilesMenu(); // 立即更新菜单显示
    }
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            editor->setText(file.readAll());
            editor->setModified(false); // 刚加载，未修改
            file.close();

            if (isAutoSaveRecovery) {
                // 可选提示
                // statusBar()->showMessage("已恢复: " + baseName, 3000);
            }
        } else {
            QMessageBox::warning(this, "打开失败", "无法读取：" + filePath);
            m_tabWidget->removeTab(index);
            m_tabs.removeAt(m_tabs.size() - 1);
            editor->deleteLater();
            return;
        }
    } else {
        editor->setModified(false);
    }
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
    updateRecentFilesMenu();
}

void MainWindow::setupRecentFilesMenu()
{
    m_recentFilesMenu = new QMenu("最近打开", this);
    menuBar()->addMenu(m_recentFilesMenu);
    // 添加分隔线，保持菜单清晰
    QAction *separator = m_recentFilesMenu->addSeparator();
    separator->setObjectName("recentFilesSeparator");
    // 初始更新
    updateRecentFilesMenu();
}

void MainWindow::updateRecentFilesMenu()
{
    // === 1. 获取最近文件列表 ===
    QStringList filePaths = getLastOpenFiles();
    // === 2. 找到分隔线（作为锚点）===
    QAction *separator = nullptr;
    for (QAction *act : m_recentFilesMenu->actions()) {
        if (act->objectName() == "recentFilesSeparator") {
            separator = act;
            break;
        }
    }
    // 如果没有分隔线，说明菜单结构异常，清空重建
    if (!separator) {
        m_recentFilesMenu->clear();
        separator = m_recentFilesMenu->addSeparator();
        separator->setObjectName("recentFilesSeparator");
    }
    // === 3. 删除 separator 之前的所有菜单项（即所有文件项）===
    while (m_recentFilesMenu->actions().first() != separator) {
        QAction *action = m_recentFilesMenu->actions().first();
        m_recentFilesMenu->removeAction(action);
        delete action; // ✅ 必须 delete，否则内存泄漏 + 菜单残留
    }
    // === 4. 删除 separator 之后的所有项（包括旧的“清除”按钮）===
    bool afterSeparator = false;
    QList<QAction *> actionsToErase;
    for (QAction *act : m_recentFilesMenu->actions()) {
        if (act == separator) {
            afterSeparator = true;
            continue;
        }
        if (afterSeparator) {
            actionsToErase.append(act);
        }
    }
    // 反向删除，避免迭代器失效
    for (int i = actionsToErase.size() - 1; i >= 0; i--) {
        m_recentFilesMenu->removeAction(actionsToErase[i]);
        delete actionsToErase[i];
    }
    // === 5. 添加菜单项 ===
    if (filePaths.isEmpty()) {
        QAction *noFiles = m_recentFilesMenu->addAction("无最近文件");
        noFiles->setEnabled(false);
    } else {
        for (const QString &path : filePaths) {
            QFileInfo fi(path);
            QString displayName = fi.fileName();

            if (fi.fileName().endsWith(".autosave", Qt::CaseInsensitive)) {
                displayName = fi.completeBaseName() + " (已恢复)";
            }

            QAction *action = m_recentFilesMenu->addAction(displayName);
            action->setData(path);
            action->setToolTip(path);
            connect(action, &QAction::triggered, this, &MainWindow::openRecentFile);
        }
    }
    // === 6. 添加“清除最近文件”按钮 ===
    QAction *clearAction = m_recentFilesMenu->addAction("清除最近文件");
    connect(clearAction, &QAction::triggered, this, [this]() {
        setLastOpenFiles(QStringList());  // 清空配置
        updateRecentFilesMenu();          // 递归刷新（安全，因为此时列表为空）
    });
}


void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) return;

    QString targetPath = action->data().toString(); // 可能是 .autosave 路径
    QFileInfo targetFi(targetPath);

    // ---------- 1. 检查文件是否存在 ----------
    if (targetPath.isEmpty() || !QFile::exists(targetPath)) {
        QMessageBox::warning(this, "文件不存在", "该文件可能已被删除或移动。\n路径：" + targetPath);

        // 从最近列表中移除
        QStringList files = getLastOpenFiles();
        files.removeOne(targetPath);
        setLastOpenFiles(files);
        updateRecentFilesMenu();
        return;
    }

    // 遍历所有 tab，检查是否已打开
    for (int i = 0; i < m_tabs.size(); ++i) {
        const TabInfo &info = m_tabs[i];

        // 情况1：已命名文件
        if (!info.filePath.isEmpty()) {
            QFileInfo openedFi(info.filePath);
            if (openedFi.canonicalFilePath() == targetFi.canonicalFilePath()) {
                m_tabWidget->setCurrentIndex(i);
                return;
            }
        }

        // 情况2：未命名文件（检查 .autosave 路径）
        if (info.filePath.isEmpty() && !info.autoSavePath.isEmpty()) {
            QFileInfo autoFi(info.autoSavePath);
            if (autoFi.canonicalFilePath() == targetFi.canonicalFilePath()) {
                m_tabWidget->setCurrentIndex(i);
                return;
            }
        }
    }

    // ---------- 3. 都没找到 → 新建 tab 恢复 ----------
    addNewTab(targetPath);
}


void MainWindow::openFileNoPar()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString path = QFileDialog::getOpenFileName(
        this,
        "打开文件",
        dir,
        "所有文件 (*.*)"
        );
    if (!path.isEmpty()) {
        openFile(path); // 转调有参版本
    }
}

void MainWindow::openFile(const QString &path)
{
    QFileInfo fi(path);
    if (!fi.exists()) {
        QMessageBox::warning(this, "文件不存在", "无法找到文件：" + path);
        return;
    }
    // ---------- 1. 检查是否已打开（避免重复） ----------
    for (const TabInfo &info : m_tabs) {
        if (!info.filePath.isEmpty() && QFileInfo(info.filePath).canonicalFilePath() == fi.canonicalFilePath()) {
            // 文件已打开，切换到该 tab
            int index = m_tabWidget->indexOf(info.editor);
            m_tabWidget->setCurrentIndex(index);
            return;
        }
    }
    // ---------- 2. 更新最近文件列表 ----------
    QStringList files = getLastOpenFiles();
    // 从列表中移除（如果已存在）
    files.removeOne(path);
    // 将当前文件放到最前面
    files.prepend(path);
    // 限制最多 10 个
    while (files.size() > 10) {
        files.removeLast();
    }
    setLastOpenFiles(files);
    // ---------- 3. 调用 addNewTab 打开 ----------
    addNewTab(path); // ✅ 自动处理正常文件和 .autosave
    // ---------- 4. 更新最近文件菜单 ----------
    updateRecentFilesMenu();
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

void MainWindow::closeTab(int index) {
    if (!closeTabWithPrompt(index)) {
        return; // 用户取消关闭
    }

    TabInfo &info = m_tabs[index];

    // ✅ 获取 .autosave 路径（用于后续判断）
    QString autoSavePath = info.autoSavePath;

    // 执行关闭
    m_tabWidget->removeTab(index);
    delete info.editor; // 记得释放 editor！
    m_tabs.removeAt(index);

    // ✅ 更新最近打开列表
    QStringList lastFiles = getLastOpenFiles();

    // 只有当是【未命名 + 未修改】的 tab 时，才需要从 lastFiles 中移除 .autosave 路径
    // 因为这种 tab 的 .autosave 是空的、无效的，用户可能只是误点“新建”
//    if (info.filePath.isEmpty() && !info.isModified) {
//        if (lastFiles.contains(autoSavePath)) {
//            lastFiles.removeOne(autoSavePath);
//            setLastOpenFiles(lastFiles); // 立即保存
//            updateRecentFilesMenu();     // 刷新菜单显示
//        }
//        // 可选：删除空的 .autosave 文件
//        QFile::remove(autoSavePath);
//    }

}


void MainWindow::onSaveAs()
{
    int index = m_tabWidget->currentIndex();
    if (index < 0 || index >= m_tabs.size()) return;

    TabInfo &info = m_tabs[index];

    QString dir;
    QString defaultSuffix = "txt";
    if (!info.filePath.isEmpty()) {
        QFileInfo fi(info.filePath);
        dir = fi.path();
        defaultSuffix = fi.suffix();
    } else {
        dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    QString defaultFileName = dir + "/" + info.baseName + "." + defaultSuffix;
    QString fileName = QFileDialog::getSaveFileName(this, "另存为", defaultFileName, "所有文件 (*)");
    if (fileName.isEmpty()) return;

    QFileInfo fi(fileName);
    if (fi.suffix().isEmpty()) {
        fileName += "." + defaultSuffix;
    }

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(info.editor->text().toUtf8());
        file.close();

        // ✅ 1. 更新路径和名称
        info.filePath = fileName;
        info.baseName = fi.fileName();

        // ✅ 2. 保存成功 → 内容未修改
        info.editor->setModified(false); // 这会触发 modificationChanged

        // ✅ 3. 更新最近打开
        QStringList files = getLastOpenFiles();
        files.removeOne(fileName);
        files.prepend(fileName);
        while (files.size() > 10) files.removeLast();
        setLastOpenFiles(files);
        updateRecentFilesMenu();

        // ✅ 4. 删除旧 .autosave
        if (!info.autoSavePath.isEmpty()) {
            QFile::remove(info.autoSavePath);
            info.autoSavePath = autoSavePathForTab(info);
        }
    } else {
        QMessageBox::warning(this, "保存失败", "无法写入：" + fileName);
    }
}

void MainWindow::onSave()
{
    int index = m_tabWidget->currentIndex();
    if (index < 0 || index >= m_tabs.size()) return;

    TabInfo &info = m_tabs[index];

    if (info.filePath.isEmpty()) {
        onSaveAs();
        updateRecentFilesMenu();
    } else {
        // 直接保存到 info.filePath
        QFile file(info.filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(info.editor->text().toUtf8());
            file.close();

            // ✅ 保存成功 → 标记为未修改
            info.editor->setModified(false); // 自动触发 modificationChanged，清除 *
        } else {
            QMessageBox::warning(this, "保存失败", "无法写入：" + info.filePath);
        }
    }
}


