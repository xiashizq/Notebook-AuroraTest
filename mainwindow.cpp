#include "MainWindow.h"

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qsciapis.h>
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
#include <QTabBar>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDomDocument>
#include <QToolBar>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>

#pragma execution_character_set("utf-8")




MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{

    this->setWindowTitle("Notebook-AuroraTest");
    this->setWindowIcon(QIcon(":/iocn1.png"));
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setInterval(10 * 1000); // 每 10 秒自动保存一次
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSaveAll);
    m_autoSaveTimer->start();

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setStyleSheet(
        "QTabBar::tab { background: #f3f3f3; color: #666666; }"
        "QTabBar::tab:selected { background: #b3d8fd; color: #222222; }"
        );

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    connect(m_tabWidget->tabBar(), &QTabBar::tabMoved, this, &MainWindow::onTabMoved);

    layout->addWidget(m_tabWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    central->setLayout(layout);
    setCentralWidget(central);
    resize(1200, 700);
    // 菜单栏
    QMenuBar *menuBar = this->menuBar();
    QMenu *fileMenu = menuBar->addMenu("文件");

    // 添加“打开”菜单项
    QAction *openAction = fileMenu->addAction("打开文件");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFileNoPar);

    QAction *newWindowAction = new QAction("新建SqlParser窗口", this);
    fileMenu->addAction(newWindowAction);

    QMenu *beautifyMenu = menuBar->addMenu("美化");
    QAction *jsonBTYAction = new QAction("JSON美化", this);
    QAction *xmlBTYAction = new QAction("XML美化", this);
    beautifyMenu->addAction(jsonBTYAction);
    beautifyMenu->addAction(xmlBTYAction);
    connect(jsonBTYAction, &QAction::triggered, this, &MainWindow::formatJson);
    connect(xmlBTYAction, &QAction::triggered, this, &MainWindow::formatXml);
    connect(newWindowAction, &QAction::triggered, this, &MainWindow::openNewWindow);

    QMenu *codeHightLight = menuBar->addMenu("代码高亮");
    QAction *pythonHlAction = new QAction("Python", this);
    QAction *jsonHlAction = new QAction("JSON", this);
    QAction *cppHlAction = new QAction("C++", this);
    QAction *javaHlAction = new QAction("Java", this);
    QAction *javascriptHlAction = new QAction("JavaScript", this);
    QAction *xmlHlAction = new QAction("XML", this);
    QAction *clearHlAction = new QAction("清除高亮", this);
    codeHightLight->addAction(pythonHlAction);
    codeHightLight->addAction(jsonHlAction);
    codeHightLight->addAction(cppHlAction);
    codeHightLight->addAction(javaHlAction);
    codeHightLight->addAction(javascriptHlAction);
    codeHightLight->addAction(xmlHlAction);
    codeHightLight->addAction(clearHlAction);

    connect(pythonHlAction, &QAction::triggered, this, [this]() {
        codeHightLightFunction("Python");
    });
    connect(jsonHlAction, &QAction::triggered, this, [this]() {
        codeHightLightFunction("JSON");
    });
    connect(cppHlAction, &QAction::triggered, this, [this]() {
        codeHightLightFunction("CPP");
    });
    connect(javaHlAction, &QAction::triggered, this, [this]() {
        codeHightLightFunction("Java");
    });
    connect(javascriptHlAction, &QAction::triggered, this, [this]() {
        codeHightLightFunction("JavaScript");
    });
    connect(xmlHlAction, &QAction::triggered, this, [this]() {
        codeHightLightFunction("XML");
    });
    connect(clearHlAction, &QAction::triggered, this, [this]() {
        codeHightLightFunction("");
    });


    QMenu *diffMenu = menuBar->addMenu("对比工具");
    QAction *diffHJAction = new QAction("行级对比", this);
    diffMenu->addAction(diffHJAction);
    connect(diffHJAction, &QAction::triggered, this, &MainWindow::openDiffWidget);



    // 创建工具栏
    QToolBar *mainToolBar = new QToolBar("主工具栏", this);

    mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly); // 图标在上，文字在下
    mainToolBar->setIconSize(QSize(20, 20)); // 设置图标大小

    // 新增
    QAction *addAction = new QAction(QIcon(":/imageSrc/add.png"), "新增", this);
    connect(addAction, &QAction::triggered, this, &MainWindow::addNewTabMneu);
    mainToolBar->addAction(addAction);

    // 创建保存动作（如果你还没有 QAction，可以复用已绑定的）
    QAction *saveAction = new QAction(QIcon(":/imageSrc/save.png"), "保存", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSave);
    mainToolBar->addAction(saveAction);

    // 新增
    QAction *saveAsAction = new QAction(QIcon(":/imageSrc/saveAs.png"), "另存为", this);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::onSaveAs);
    mainToolBar->addAction(saveAsAction);



    // 可以添加更多图标，比如“自动保存”开关
//    QAction *autoSaveAction = new QAction(QIcon(":/auto_save.png"), "自动保存", this);
//    autoSaveAction->setCheckable(true);
//    autoSaveAction->setChecked(true);
//    connect(autoSaveAction, &QAction::toggled, [this](bool checked) {
//        if (checked)
//            m_autoSaveTimer->start();
//        else
//            m_autoSaveTimer->stop();
//    });
//    mainToolBar->addAction(autoSaveAction);

    // 将工具栏添加到主窗口（会自动显示在菜单栏下方）

    // 设置浅白色背景 + 更紧凑的样式
    mainToolBar->setStyleSheet(
        "QToolBar {"
        "   background-color: #f8f8f8;"
        "   border: 1px solid #dcdcdc;"
        "   spacing: 6px;"
        "   padding: 2px;"
        "   min-height: 30px;"
        "}"
        "QToolButton {"
        "   padding: 2px;"
        "   margin: 0px;"
        "   border: none;"
        "   border-radius: 4px;"
        "}"
        "QToolButton:hover {"
        "   background: #e0e0e0;"
        "}"
        "QToolButton:pressed {"
        "   background: #d0d0d0;"
        "}"
    );

    addToolBar(mainToolBar);







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

    // Ctrl+F 打开搜索
    QAction *findAction = new QAction(this);
    findAction->setShortcut(QKeySequence::Find);
    connect(findAction, &QAction::triggered, this, &MainWindow::openFindReplaceDialog);
    this->addAction(findAction); // 添加到窗口

    restoreSession();
}

void MainWindow::openNewWindow() {
    SqlParserWindow* sqlParserWindow = new SqlParserWindow();
    sqlParserWindow->show();
}

QFont MainWindow::m_font(const int &fontsize){
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

QsciScintilla* MainWindow::getCurrentEditor()
{
    int index = m_tabWidget->currentIndex();
    if (index < 0) return nullptr;
    return qobject_cast<QsciScintilla*>(m_tabWidget->widget(index));
}

void MainWindow::setLexerForLanguage(QsciScintilla *editor, const QString &language)
{
//    QSettings settings("AuroraTestTech", "Notebook-AuroraTest");
//    QFont font = settings.value("editor/font", QFont("Consolas", 12)).value<QFont>();
    delete editor->lexer();
    QsciLexer *lexer = nullptr;
    // 设置所有样式的字体
    if (language == "Python") {
        lexer = new QsciLexerPython(editor);
    } else if (language == "CPP") {
        lexer = new QsciLexerCPP(editor);
    } else if (language == "Java") {
        lexer = new QsciLexerJava(editor);
    } else if (language == "JSON" || language == "JavaScript") {
        QsciLexerJavaScript *jsLexer = new QsciLexerJavaScript(editor);
        lexer = jsLexer;
    } else if (language == "XML") {
        lexer = new QsciLexerXML(editor);
    } else if (language == "None" || language.isEmpty()) {
        // 无高亮模式
        editor->setLexer(nullptr);
        editor->setColor(Qt::black);
        editor->setPaper(Qt::white);
        editor->setFont(m_font(12));
        editor->SendScintilla(QsciScintilla::SCI_STYLECLEARALL);
        return;
    } else {
        editor->setLexer(nullptr);
        return;
    }
    lexer->setFont(m_font(12));
    editor->setLexer(lexer);
}

bool MainWindow::formatJson()
{
    QsciScintilla *editor = getCurrentEditor();
    QString text = editor->text();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, "JSON 格式错误",
                             QString("解析错误: %1\n位置: %2")
                             .arg(error.errorString()).arg(error.offset));
        return false;
    }
    // 美化输出（带缩进）
    QByteArray formatted = doc.toJson(QJsonDocument::Indented);
    editor->setText(QString::fromUtf8(formatted));
    return true;
}

bool MainWindow::formatXml()
{
    QsciScintilla *editor = getCurrentEditor();
    QString text = editor->text();
    QDomDocument doc;
    QString errorStr;
    int errorLine, errorCol;
    if (!doc.setContent(text, &errorStr, &errorLine, &errorCol)) {
        QMessageBox::warning(this, "XML 格式错误",
                             QString("解析错误: %1\n行: %2, 列: %3")
                             .arg(errorStr).arg(errorLine).arg(errorCol));
        return false;
    }
    // 美化输出（缩进 4 个空格）
    QString formatted = doc.toString(4);  // 4 spaces
    editor->setText(formatted);
    return true;
}

void MainWindow::codeHightLightFunction(const QString& language)
{
    QsciScintilla *editor = getCurrentEditor();
    if (!editor) return;
    // 获取当前 tab 的索引
    int currentIndex = m_tabWidget->currentIndex();
    if (currentIndex < 0 || currentIndex >= m_tabs.size()) return;
    TabInfo &info = m_tabs[currentIndex];
    setLexerForLanguage(editor,language);
    // 更新 TabInfo 中的语言记录
    info.language = language;
    // 可选：更新 tab 标题提示（比如加个图标或后缀）
    // m_tabWidget->setTabText(currentIndex, info.displayName() + " [*]");
}


void MainWindow::onTabMoved(int from, int to)
{
    if (from >= 0 && from < m_tabs.size() && to >= 0 && to < m_tabs.size()) {
        // 调整m_tabs列表顺序
        m_tabs.move(from, to);
    }
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
    editor->setMarginsFont(m_font(10));
    editor->setMarginLineNumbers(0, true);
    editor->setMarginsForegroundColor(QColor("#666666"));
    editor->setMarginsBackgroundColor(QColor("#eeeeee"));
    editor->setWrapMode(QsciScintilla::WrapCharacter);

    editor->setCaretLineVisible(true); //是否高亮显示光标所在行
    editor->setCaretLineBackgroundColor(QColor("#eeeeee"));//光标所在行背景颜色

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
            finalFilePath = ""; // 标记为新文件
        } else {
            baseName = fi.fileName();
            finalFilePath = filePath;
        }
    } else {
        baseName = "新文件" + QString::number(m_untitledCount++);
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

    if (isAutoSaveRecovery) {
        // 是从 .autosave 恢复的 → 用户从未手动保存过
        info.neverSavedByUser = true;
    } else if (filePath.isEmpty()) {
        // 是用户点击“新建” → 也从未保存过
        info.neverSavedByUser = true;
    } else {
        // 是打开已有文件 → 用户“已经保存过”（至少保存过一次）
        info.neverSavedByUser = false;
    }

    m_tabs.append(info);

    // ------------------------
    // 5. 连接 modificationChanged（更新 tab 标题）
    // ------------------------
    connect(editor, &QsciScintilla::modificationChanged, this, [this, editor](bool modified) {
        for (int i = 0; i < m_tabs.size(); ++i) {
            if (m_tabs[i].editor == editor) {
                m_tabs[i].isModified = modified;
                QString title = m_tabs[i].baseName;
                //只要用户从未手动保存过，就显示 *
                if (m_tabs[i].neverSavedByUser) {
                    m_tabWidget->setTabText(i, title); //如果是自动创建的文件，自动保存也先移除星号
                } else {
                    // 否则：只在修改时显示 *
                    if (modified) {
                        m_tabWidget->setTabText(i, title + " *");
                    } else {
                        m_tabWidget->setTabText(i, title);
                    }
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
        filePath = QFileDialog::getSaveFileName(this, "保存文件", info.displayName(), "Text Files (*.txt);;All Files (*)");
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
            // 新文件文件：保存到自动保存目录
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
            // 可选：如果是从“新文件”自动保存，可以更新 info.filePath？
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
        // 新文件文件：使用 baseName（如“新文件1”）
        baseName = info.baseName;
    }
    //只加 .autosave
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
    // 匹配 "新文件123" 这种格式
    QRegularExpression re(R"(新文件(\d+))");
    QRegularExpressionMatch match = re.match(filename);
    if (match.hasMatch()) {
        return match.captured(1).toInt();
    }
    return 0;
}




QStringList MainWindow::getLastOpenFiles()
{
    QSettings settings("AuroraTestTech", "Notebook-AuroraTest");
    return settings.value("session/lastFiles").toStringList();
}

void MainWindow::setLastOpenFiles(const QStringList &files)
{
    QSettings settings("AuroraTestTech", "Notebook-AuroraTest");
    settings.setValue("session/lastFiles", files);
}

//void MainWindow::saveSession()
//{
//    // 保存当前打开的标签路径，用于下次启动时恢复
//    QStringList openTabs;
//    for (const TabInfo &info : m_tabs) {
//        if (!info.filePath.isEmpty()) {
//            openTabs << info.filePath;
//        } else {
//            // 新文件：保存其自动保存路径
//            openTabs << autoSavePathForTab(info);
//        }
//        openTabs << info.language;
//    }
//    QSettings settings("AuroraTestTech", "Notebook-AuroraTest");
//    settings.setValue("session/openTabs", openTabs);
//    settings.setValue("session/currentTabIndex", m_tabWidget->currentIndex());
//}

void MainWindow::saveSession()
{
    QVariantList sessionTabs;

    for (const TabInfo &info : m_tabs) {
        QVariantMap tabData;
        if (!info.filePath.isEmpty()) {
            tabData["path"] = info.filePath;
            tabData["type"] = "file";
        } else {
            tabData["path"] = info.autoSavePath;
            tabData["type"] = "autosave";
        }
        //  保存 language
        tabData["language"] = info.language;
        // 可选：保存其他状态
        // tabData["isModified"] = info.isModified;
        // tabData["baseName"] = info.baseName;
        sessionTabs << tabData;
    }
    QSettings settings("AuroraTestTech", "Notebook-AuroraTest");
    settings.setValue("session/tabs", sessionTabs);
    settings.setValue("session/currentTabIndex", m_tabWidget->currentIndex());
}

void MainWindow::setEditorLexer(QsciScintilla *editor, const QString &language)
{
    // 删除旧的 lexer，避免内存泄漏
    setLexerForLanguage(editor,language);
}

void MainWindow::restoreSession()
{
    QSettings settings("AuroraTestTech", "Notebook-AuroraTest");
    QVariantList sessionTabs = settings.value("session/tabs").toList();
    QStringList lastFiles = getLastOpenFiles();
    int maxNum = 0;
    for (const QString &path : lastFiles) {
        QFileInfo fi(path);
        QString baseName = fi.completeBaseName();
        if (baseName.startsWith("新文件")) {
            int num = extractNumberFromUntitled(baseName);
            if (num > maxNum) {
                maxNum = num;
            }
        }
    }
    m_untitledCount = maxNum + 1;
    if (sessionTabs.isEmpty()) {
        addNewTabMneu();
    } else {
        for (const QVariant &tabVar : sessionTabs) {
            QVariantMap tabData = tabVar.toMap();
            QString path = tabData["path"].toString();
            QString language = tabData["language"].toString();  //  读取保存的语言

            // 打开文件
            addNewTab(path);
            // 获取刚添加的编辑器
            QsciScintilla *editor = getCurrentEditor(); // 或 getLastEditor()
            if (editor && !language.isEmpty()) {
                // 设置词法分析器
                setEditorLexer(editor, language);  // 使用之前定义的函数
                //  更新当前 TabInfo 的 language
                int currentIndex = m_tabWidget->currentIndex();
                if (currentIndex < m_tabs.size()) {
                    m_tabs[currentIndex].language = language;
                }
            }
        }
        int currentTabIndex = settings.value("session/currentTabIndex", -1).toInt();
        if (currentTabIndex >= 0 && currentTabIndex < m_tabWidget->count()) {
            m_tabWidget->setCurrentIndex(currentTabIndex);
        }
    }
    updateRecentFilesMenu();
}

//void MainWindow::restoreSession()
//{
//    QSettings settings("AuroraTestTech", "Notebook-AuroraTest");
//    QStringList openTabs = settings.value("session/openTabs").toStringList();
//    QStringList lastFiles = getLastOpenFiles();
//    int maxNum = 0;
//    // 从最近文件中找出“新文件X.autosave”的最大编号
//    for (const QString &path : lastFiles) {
//        QFileInfo fi(path);
//        QString baseName = fi.completeBaseName(); // 去掉 .autosave
//        if (baseName.startsWith("新文件")) {
//            int num = extractNumberFromUntitled(baseName);
//            if (num > maxNum) {
//                maxNum = num;
//            }
//        }
//    }
//    m_untitledCount = maxNum + 1;
//    // 恢复标签页
//    if (openTabs.isEmpty()) {
//        addNewTabMneu();
//    } else {
//        for (const QString &path : openTabs) {
//            addNewTab(path);
//        }
//        // 恢复上次选中的标签页
//        int currentTabIndex = settings.value("session/currentTabIndex", -1).toInt();
//        if (currentTabIndex >= 0 && currentTabIndex < m_tabWidget->count()) {
//            m_tabWidget->setCurrentIndex(currentTabIndex);
//        }
//    }
//    updateRecentFilesMenu();
//}



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
    saveSession();
    event->accept();
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
        delete action; //必须 delete，否则内存泄漏 + 菜单残留
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
                displayName = fi.completeBaseName();
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
        // 情况2：新文件文件（检查 .autosave 路径）
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
    addNewTab(path); //自动处理正常文件和 .autosave
    // ---------- 4. 更新最近文件菜单 ----------
    updateRecentFilesMenu();
}


bool MainWindow::closeTabWithPrompt(int index)
{
    if (index < 0 || index >= m_tabs.size()) return true;
    TabInfo &info = m_tabs[index];
    QsciScintilla *editor = info.editor;
    if (editor->isModified()) {
        QString title = info.displayName().isEmpty() ? "新文件" : info.displayName();
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
    auto *editor = info.editor;
    // 获取 .autosave 路径（用于后续判断）
    QString autoSavePath = info.autoSavePath;
    // 执行关闭
    m_tabWidget->removeTab(index);
    m_tabs.removeAt(index);
    editor->deleteLater();
    // 只有当是【新文件 + 未修改】的 tab 时，才需要从 lastFiles 中移除 .autosave 路径
    // 因为这种 tab 的 .autosave 是空的、无效的，用户可能只是误点“新建”
    if (info.filePath.isEmpty() && !info.isModified) {
        QStringList lastFiles = getLastOpenFiles();
        if (lastFiles.contains(autoSavePath)) {
            lastFiles.removeOne(autoSavePath);
            setLastOpenFiles(lastFiles); // 立即保存
            updateRecentFilesMenu();     // 刷新菜单显示
        }
        // 可选：删除空的 .autosave 文件
        QFile::remove(autoSavePath);
    }
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
        //更新路径和名称
        info.filePath = fileName;
        info.baseName = fi.fileName();
        //保存成功 → 内容未修改
        info.editor->setModified(false); // 这会触发 modificationChanged
        //更新最近打开
        QStringList files = getLastOpenFiles();
        files.removeOne(fileName);
        files.prepend(fileName);
        while (files.size() > 10) files.removeLast();
        setLastOpenFiles(files);
        updateRecentFilesMenu();
        //删除旧 .autosave
        if (!info.autoSavePath.isEmpty()) {
            QFile::remove(info.autoSavePath);
            info.autoSavePath = autoSavePathForTab(info);
        }
        info.neverSavedByUser = false;
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
            //保存成功 → 标记为未修改
            info.editor->setModified(false); // 自动触发 modificationChanged，清除 *
            info.neverSavedByUser = false;
        } else {
            QMessageBox::warning(this, "保存失败", "无法写入：" + info.filePath);
        }
    }
}


void MainWindow::openFindReplaceDialog()
{
    QsciScintilla *editor = getCurrentEditor();
    if (!editor) return;
    // 如果已打开，就激活
    if (m_findReplaceDialog) {
        m_findReplaceDialog->raise();
        m_findReplaceDialog->activateWindow();
        return;
    }
    m_findReplaceDialog = new QDialog(this);
    m_findReplaceDialog->setWindowTitle("查找和替换");
    m_findReplaceDialog->resize(320, 160);
    m_findReplaceDialog->setAttribute(Qt::WA_DeleteOnClose);
    m_findReplaceDialog->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    connect(m_findReplaceDialog, &QDialog::finished, this, [this]() {
        m_findReplaceDialog = nullptr;
        m_findStarted = false; // 关闭时重置状态
    });
    // === UI 控件 ===
    auto *findEdit = new QLineEdit(m_findReplaceDialog);
    auto *replaceEdit = new QLineEdit(m_findReplaceDialog);
    auto *caseCheck = new QCheckBox("区分大小写", m_findReplaceDialog);
    auto *wrapCheck = new QCheckBox("循环查找", m_findReplaceDialog);
    wrapCheck->setChecked(true);
    auto *regexCheck = new QCheckBox("正则表达式", m_findReplaceDialog);
    regexCheck->setToolTip("启用后，\\n \\t ^ $ . * + 等作为正则语法");
    auto *findButton = new QPushButton("查找下一个", m_findReplaceDialog);
    auto *replaceButton = new QPushButton("替换", m_findReplaceDialog);
    auto *replaceAllButton = new QPushButton("全部替换", m_findReplaceDialog);
    auto *closeButton = new QPushButton("关闭", m_findReplaceDialog);
    // 布局
    auto *topLayout = new QGridLayout;
    topLayout->addWidget(new QLabel("查找："), 0, 0);
    topLayout->addWidget(findEdit, 0, 1);
    topLayout->addWidget(new QLabel("替换为："), 1, 0);
    topLayout->addWidget(replaceEdit, 1, 1);
    topLayout->addWidget(caseCheck, 2, 0);
    topLayout->addWidget(wrapCheck, 2, 1);
    topLayout->addWidget(regexCheck, 3, 0, 1, 2);
    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(findButton);
    buttonLayout->addWidget(replaceButton);
    buttonLayout->addWidget(replaceAllButton);
    buttonLayout->addWidget(closeButton);
    auto *mainLayout = new QVBoxLayout(m_findReplaceDialog);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(buttonLayout);
    // === 功能实现（同上）===
    QObject::connect(findButton, &QPushButton::clicked, [=]() {
        QsciScintilla *currentEditor = getCurrentEditor();
        if (!currentEditor) {
            QMessageBox::warning(m_findReplaceDialog, "错误", "当前没有打开的编辑器。");
            return;
        }
        m_findStarted = false; // 重置“替换”状态，确保从当前光标开始
        QString findText = findEdit->text();
        if (findText.isEmpty()) return;
        bool useRegex = regexCheck->isChecked();
        bool cs = caseCheck->isChecked();
        // 正则模式：用原始字符串；普通模式：解析 \n \t
        QString textToFind = useRegex ? findText : unescapeString(findText);
        bool found = editor->findFirst(
            textToFind,           // 查找内容
            false,                // 向前查找
            cs,                   // 区分大小写
            useRegex,             // 是否正则
            true,                 // wholeWord: 可加 checkbox 控制
            true,                 // 允许空搜索
            -1, -1,               // 从当前光标开始
            wrapCheck->isChecked() // 循环查找
        );
        if (!found) {
            QMessageBox::information(m_findReplaceDialog, "查找", "未找到 \"" + findText + "\"");
        }
    });
    QObject::connect(replaceButton, &QPushButton::clicked, [=]() {
        QsciScintilla *currentEditor = getCurrentEditor();
        if (!currentEditor) {
            QMessageBox::warning(m_findReplaceDialog, "错误", "当前没有打开的编辑器。");
            return;
        }
        QString findText = findEdit->text();
        if (findText.isEmpty()) return;
        bool useRegex = regexCheck->isChecked();
        bool cs = caseCheck->isChecked();
        QString textToFind = useRegex ? findText : unescapeString(findText);
        // 第一次点击：启动查找
        if (!m_findStarted) {
            m_findStarted = true;
            bool found = currentEditor->findFirst(
                textToFind, false, cs, useRegex, true, true, -1, -1, wrapCheck->isChecked()
            );
            if (!found) {
                QMessageBox::information(m_findReplaceDialog, "替换", "未找到 \"" + findText + "\"");
                m_findStarted = false;
            }
            return;
        }
        // 已有匹配，执行替换
        if (currentEditor->hasSelectedText()) {
            QString selected = currentEditor->selectedText();
            bool matches = useRegex ? true : // 正则模式下 assume 匹配
                          (cs ? selected == textToFind : selected.toLower() == textToFind.toLower());
            if (matches) {
                QString replaceText = unescapeString(replaceEdit->text());
                currentEditor->replaceSelectedText(replaceText);
            }
        }
        // 查找下一个
        bool found = currentEditor->findNext();
        if (!found) {
            QMessageBox::information(m_findReplaceDialog, "替换", "已到末尾，未找到更多匹配项。");
            m_findStarted = false;
        }
    });

    QObject::connect(replaceAllButton, &QPushButton::clicked, [=]() {
        QsciScintilla *currentEditor = getCurrentEditor();
        if (!currentEditor) return;
        QString findText = findEdit->text();
        if (findText.isEmpty()) return;
        QString replaceTextStr = replaceEdit->text();
        bool useRegex = regexCheck->isChecked();
        bool cs = caseCheck->isChecked();
        QString text = currentEditor->text();
        int count = 0;
        QString result;
        if (useRegex) {
            // 正则模式
            QString literalPattern = unescapeString(findText);
            QRegularExpression::PatternOptions options = cs ?
                        QRegularExpression::NoPatternOption :
                        QRegularExpression::CaseInsensitiveOption;
            QRegularExpression regex(literalPattern, options);
            if (!regex.isValid()) {
                QMessageBox::warning(m_findReplaceDialog, "正则错误", "正则表达式无效：" + regex.errorString());
                return;
            }
            // 统计匹配次数
            QRegularExpressionMatchIterator it = regex.globalMatch(text);
            count = 0;
            while (it.hasNext()) {
                it.next();
                count++;
            }
            // 执行替换
            result = text.replace(regex, unescapeString(replaceTextStr));
        } else {
            // 普通模式：支持 \n \t 的全局替换（修复版）
            QString searchText = unescapeString(findText);
            QString replaceText = unescapeString(replaceTextStr);
            if (searchText.isEmpty()) {
                count = 0;
                result = text;
            } else {
                result = text;
                int pos = 0;
                count = 0;
                Qt::CaseSensitivity csOption = cs ? Qt::CaseSensitive : Qt::CaseInsensitive;
                while ((pos = result.indexOf(searchText, pos, csOption)) != -1) {
                    result.replace(pos, searchText.length(), replaceText);
                    pos += replaceText.length();
                    count++;
                }
            }
        }
        // 一次性写回
        if (count > 0) {
            currentEditor->beginUndoAction();
            currentEditor->selectAll();
            currentEditor->replaceSelectedText(result);
            currentEditor->endUndoAction();
            showMessageInCenter("完成", QString("已完成 %1 处替换").arg(count));
        } else {
            QMessageBox::information(m_findReplaceDialog, "全部替换", "未找到匹配项。");
        }
    });
    // 关闭按钮
    QObject::connect(closeButton, &QPushButton::clicked, m_findReplaceDialog, &QDialog::accept);
    connect(m_findReplaceDialog, &QDialog::finished, this, [this]() {
        m_findReplaceDialog = nullptr;
        m_findStarted = false; // 重置状态
    });
    // 回车 = 查找
    QObject::connect(findEdit, &QLineEdit::returnPressed, findButton, &QPushButton::click);
    // 显示为非模态窗口
    m_findReplaceDialog->show();
    findEdit->setFocus(); // 聚焦搜索框
}


QString MainWindow::unescapeString(const QString &input) {
    QString result;
    result.reserve(input.size());
    for (int i = 0; i < input.size(); ++i) {
        QChar c = input[i];
        if (c == '\\' && i < input.size() - 1) {
            QChar next = input[i + 1];
            switch (next.toLatin1()) {
            case 'n':  result += '\n'; i++; break;
            case 't':  result += '\t'; i++; break;
            case 'r':  result += '\r'; i++; break;
            case 's':  result += ' ';  i++; break; // \s 视为空格
            case '\\': result += '\\'; i++; break;
            case '0':  result += '\0'; i++; break;
            default:   result += c; break; // 保留原样，如 \w \d 等（正则中由引擎处理）
            }
        } else {
            result += c;
        }
    }
    return result;
}


// 封装一个函数：在主窗口居中显示消息框
void MainWindow::showMessageInCenter(const QString &title, const QString &text)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setIcon(QMessageBox::Information);
    // 设置对话框在 MainWindow 居中
    msgBox.move(this->x() + this->width()/2 - msgBox.width()/2,
                this->y() + this->height()/2 - msgBox.height()/2);
    msgBox.exec(); // 模态显示，但只阻塞主窗口
}

void MainWindow::openDiffWidget() {


    // 非模态窗口，可以多开
    DiffWidget* w = new DiffWidget();
    w->setAttribute(Qt::WA_DeleteOnClose); // 自动释放
    w->setWindowTitle("行级字符/代码对比工具");
    w->resize(900,600);
    w->show();
    // 如果想要模态弹窗，可以改成QDialog并用 exec()
}

