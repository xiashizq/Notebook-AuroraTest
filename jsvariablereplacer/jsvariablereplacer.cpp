#include "jsvariablereplacer.h"

#include "../CodeEditor/CodeEditor.h"
#include "../SqlFormat/SqlHighlighter.h"
#include "../SqlFormat/SqlParser.h"

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qsciapis.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QAction>
#include <QJSEngine>
#include <QJSValue>
#include <QJSValueIterator>
#include <QMap>
#include <QMessageBox>
#include <QFont>
#include <QDebug>
#include <QCloseEvent>
#include <QApplication>
#include <QStyle> // For standard icons
#include <QUuid> // 保留，以防未来需要在 C++ 端注入
#include <QRegularExpression>
#include <QPlainTextEdit>

#pragma execution_character_set("utf-8")



// --- Define the Workspace structure ---
struct Workspace {
    QWidget *containerWidget;
    QTabWidget *innerTabWidget;
    QWidget *scriptEditorTab;
    QWidget *runTemplateTab;
    QWidget *resultTab;
    QsciScintilla *scriptEditorTextEdit;
    CodeEditor *templateTextEdit;
    QPushButton *runButton;
    QTextEdit *resultDisplayTextEdit;
    SqlHighlighter *highlighter;


    Workspace() :
        containerWidget(nullptr),
        innerTabWidget(nullptr),
        scriptEditorTab(nullptr),
        runTemplateTab(nullptr),
        resultTab(nullptr),
        scriptEditorTextEdit(nullptr),
        templateTextEdit(nullptr),
        runButton(nullptr),
        resultDisplayTextEdit(nullptr),
        highlighter(nullptr){}
};

// --- Constants for QSettings keys ---
const QString SETTINGS_WORKSPACE_COUNT_KEY = "workspace_count";
const QString SETTINGS_WORKSPACE_SCRIPT_KEY_PREFIX = "workspace_script_";
const QString SETTINGS_WORKSPACE_TEMPLATE_KEY_PREFIX = "workspace_template_";
const QString SETTINGS_ACTIVE_WORKSPACE_INDEX_KEY = "active_workspace_index";


QFont JSVariableReplacer::m_font(const int &fontsize){
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

// --- Member variables (assuming they are declared in the header) ---
// QTabWidget *mainWorkspaceTabWidget;
// QAction *addAction;
// QAction *removeAction; // This one needs to be accessible in updateButtonStates
// QMap<int, Workspace*> workspaces;
// int workspaceCounter;
// QSettings settings;

// --- Constructor ---
// Assumes QCoreApplication name/org are set in main()
JSVariableReplacer::JSVariableReplacer(QWidget *parent)
    : QWidget(parent)
    , mainWorkspaceTabWidget(nullptr)
    , addAction(nullptr)
    , removeAction(nullptr) // Initialize member
    , workspaceCounter(1)
    , settings()
{
    qDebug() << "Constructor: Start";
    setupUI();
    loadWorkspacesFromSettings();
    loadActiveWorkspaceIndex();
    if (mainWorkspaceTabWidget->count() == 0) {
        addNewWorkspace();
    }
    setWindowTitle("JS Variable Replacer - Multi-Workspace");
    resize(900, 700);
    updateButtonStates(); // Ensure state is correct after initial setup
    qDebug() << "Constructor: Finish";
}

// --- Destructor ---
JSVariableReplacer::~JSVariableReplacer()
{
    qDebug() << "Destructor: Start";
    saveAllWorkspacesToSettings();
    saveActiveWorkspaceIndex();
    qDeleteAll(workspaces);
    workspaces.clear();
    qDebug() << "Destructor: Finish";
}

// --- closeEvent Override ---
void JSVariableReplacer::closeEvent(QCloseEvent *event)
{
    qDebug() << "closeEvent: Start";
    saveAllWorkspacesToSettings();
    saveActiveWorkspaceIndex();
    this->hide();
    event->ignore();
    qDebug() << "closeEvent: Finish (window hidden)";
}

// --- setupUI ---
void JSVariableReplacer::setupUI()
{
    qDebug() << "setupUI: Start";





    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // --- 使用自定义图标 (确保资源文件正确) ---
    QIcon addIcon(":/imageSrc/add.png");
    QIcon removeIcon(":/imageSrc/remove.png");

    QPushButton *addButton = new QPushButton(addIcon, "Add", this);    // 或 "新增"
    QPushButton *removeButton = new QPushButton(removeIcon, "Remove", this); // 或 "删除"
    removeButton->setEnabled(false); // Initially disabled

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // --- 创建带图标的 QAction ---
    addAction = new QAction(addIcon, "Add Workspace", this);
    removeAction = new QAction(removeIcon, "Remove Workspace", this);
    removeAction->setEnabled(false); // Initially disabled

    // --- 连接按钮点击到 Action ---
    connect(addButton, &QPushButton::clicked, addAction, &QAction::trigger);
    connect(removeButton, &QPushButton::clicked, removeAction, &QAction::trigger);

    // --- 连接 Action 到槽函数 ---
    connect(addAction, &QAction::triggered, this, &JSVariableReplacer::addNewWorkspace);
    connect(removeAction, &QAction::triggered, this, &JSVariableReplacer::removeCurrentWorkspace);

    mainWorkspaceTabWidget = new QTabWidget(this);
    mainWorkspaceTabWidget->setStyleSheet(
        "QTabBar::tab { background: #f3f3f3; color: #666666; border: 1px solid #cccccc; border-bottom: none; padding: 4px; }"
        "QTabBar::tab:selected { background: #b3d8fd; color: #222222; }"
        "QTabWidget::pane { border: 1px solid #cccccc; top: -1px; }"
        );
    mainWorkspaceTabWidget->setTabsClosable(true);
    connect(mainWorkspaceTabWidget, &QTabWidget::tabCloseRequested, this, &JSVariableReplacer::removeCurrentWorkspace);
    connect(mainWorkspaceTabWidget, &QTabWidget::currentChanged, this, &JSVariableReplacer::onTabChanged);
    mainLayout->addWidget(mainWorkspaceTabWidget);

    setLayout(mainLayout);
    qDebug() << "setupUI: Finish";
}

// --- updateButtonStates (重点修改) ---
void JSVariableReplacer::updateButtonStates()
{
    // --- 添加调试输出 ---
    qDebug() << "updateButtonStates: Called";

    // --- 检查关键指针 ---
    if (!mainWorkspaceTabWidget) {
        qDebug() << "updateButtonStates: mainWorkspaceTabWidget is null!";
        return;
    }

    // 计算是否应该启用 Remove 按钮
    bool hasMultipleWorkspaces = mainWorkspaceTabWidget->count() > 1;
    qDebug() << "updateButtonStates: Count =" << mainWorkspaceTabWidget->count() << ", Should Enable Remove =" << hasMultipleWorkspaces;

    // --- 更新 QAction 状态 ---
    if (removeAction) {
        removeAction->setEnabled(hasMultipleWorkspaces);
        qDebug() << "updateButtonStates: Set removeAction enabled =" << hasMultipleWorkspaces;
    } else {
        qDebug() << "updateButtonStates: removeAction is null!";
    }

    // --- 关键修改: 直接查找并更新 QPushButton 的状态 ---
    // 遍历子控件找到 Remove 按钮
    QList<QPushButton*> allButtons = this->findChildren<QPushButton*>();
    QPushButton* targetRemoveButton = nullptr;
    for (QPushButton* btn : allButtons) {
        // 通过文本或图标（如果唯一）来识别按钮
        // 注意：如果文本可能变化，最好在创建时保存按钮指针为成员变量
        if (btn->text() == "Remove" || btn->text() == "删除") { // Adjust text if needed
            targetRemoveButton = btn;
            break;
        }
        // 或者，如果图标是唯一的标识符 (需要更复杂的比较)
        // if (btn->icon().cacheKey() == removeIcon.cacheKey()) { ... }
    }

    if (targetRemoveButton) {
        targetRemoveButton->setEnabled(hasMultipleWorkspaces);
        qDebug() << "updateButtonStates: Set removeButton (found via findChildren) enabled =" << hasMultipleWorkspaces;
    } else {
        qDebug() << "updateButtonStates: Could not find remove QPushButton via findChildren.";
        // --- 备选方案: 如果您将 removeButton 保存为成员变量 ---
        // if (removeButtonMemberVariable) {
        //     removeButtonMemberVariable->setEnabled(hasMultipleWorkspaces);
        //     qDebug() << "updateButtonStates: Set removeButton (member) enabled =" << hasMultipleWorkspaces;
        // } else {
        //     qDebug() << "updateButtonStates: removeButtonMemberVariable is null.";
        // }
    }
}


// --- createWorkspace ---
void JSVariableReplacer::createWorkspace(int index, const QString& scriptContent, const QString& templateContent)
{
    qDebug() << "createWorkspace: Start for index" << index;
    Workspace *ws = new Workspace();




    ws->containerWidget = new QWidget(mainWorkspaceTabWidget);
    ws->innerTabWidget = new QTabWidget(ws->containerWidget);
    ws->innerTabWidget->setStyleSheet(
        "QTabBar::tab { background: #f3f3f3; color: #666666; border: 1px solid #cccccc; border-bottom: none; padding: 4px; }"
        "QTabBar::tab:selected { background: #b3d8fd; color: #222222; }"
        "QTabWidget::pane { border: 1px solid #cccccc; top: -1px; }"
        );

    ws->scriptEditorTab = new QWidget(ws->innerTabWidget);
    ws->runTemplateTab = new QWidget(ws->innerTabWidget);
    ws->resultTab = new QWidget(ws->innerTabWidget);

    ws->innerTabWidget->addTab(ws->runTemplateTab, "Run & Template");
    ws->innerTabWidget->addTab(ws->scriptEditorTab, "Script Editor");
    ws->innerTabWidget->addTab(ws->resultTab, "Result");

    QVBoxLayout *containerLayout = new QVBoxLayout(ws->containerWidget);
    containerLayout->addWidget(ws->innerTabWidget);
    ws->containerWidget->setLayout(containerLayout);

    // --- Script Editor Tab ---
    QVBoxLayout *scriptEditorTabLayout = new QVBoxLayout(ws->scriptEditorTab);
    QLabel *scriptLabel_editor = new QLabel("JavaScript Script (must return an object):", ws->scriptEditorTab);
    scriptEditorTabLayout->addWidget(scriptLabel_editor);

    ws->scriptEditorTextEdit = new QsciScintilla(ws->scriptEditorTab);
    QString exampleScript = R"(// 定义返回结果的对象
var scriptResult = {
    // --- 静态变量部分 ---
    vars: {
        appName: 'MyApplication',
        version: '2.1.0',
        // 在脚本执行时生成一次的静态随机数
        buildNumber: Math.floor(Math.random() * 10000),
        // 其他任何你希望在本次运行中保持不变的值
        author: 'Your Name'
    },

    // --- 动态函数部分 ---
    functions: {
        // 每次调用都生成新的 UUID
        newUuid: function() {
            // 简单的 UUID 模拟，或使用更复杂的库
            return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
                var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8);
                return v.toString(16);
            });
        },

        // 每次调用都获取当前时间戳
        currentTimestamp: function() {
            return Date.now().toString();
        },

        // 每次调用都生成新的随机数
        getRandomInt: function() {
             return Math.floor(Math.random() * 100000);
        },

        // 可以根据需要添加更多函数
        getGreeting: function() {
            var hours = new Date().getHours();
            if (hours < 12) return "Good Morning";
            else if (hours < 18) return "Good Afternoon";
            else return "Good Evening";
        }
    }
};

// 返回这个定义好的对象
scriptResult;)";
    ws->scriptEditorTextEdit->setText(scriptContent.isEmpty() ? exampleScript : scriptContent);
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
//    ws->scriptEditorTextEdit->setFont(font);

    QsciLexerJavaScript *jsLexer = new QsciLexerJavaScript(ws->scriptEditorTextEdit);
    jsLexer->setFont(m_font(mfontContentSize));
    ws->scriptEditorTextEdit->setLexer(jsLexer);
    ws->scriptEditorTextEdit->setFont(m_font(mfontContentSize));
    ws->scriptEditorTextEdit->setWrapMode(QsciScintilla::WrapCharacter);

    scriptEditorTabLayout->addWidget(ws->scriptEditorTextEdit);
    scriptEditorTabLayout->setStretch(1, 1);
    connect(ws->scriptEditorTextEdit, &QsciScintilla::textChanged, this, &JSVariableReplacer::onScriptChanged);
    ws->scriptEditorTab->setLayout(scriptEditorTabLayout);

    // --- Run & Template Tab ---
    QVBoxLayout *runTemplateTabLayout = new QVBoxLayout(ws->runTemplateTab);
    QLabel *templateLabel = new QLabel("Template Text (use {{key}} as placeholders):", ws->runTemplateTab);
    runTemplateTabLayout->addWidget(templateLabel);

    ws->templateTextEdit = new CodeEditor(ws->runTemplateTab);



    ws->highlighter = new SqlHighlighter(ws->templateTextEdit->document());
    connect(ws->templateTextEdit, &QPlainTextEdit::cursorPositionChanged, this, &JSVariableReplacer::onCursorChanged);

    QString exampleTemplate = R"(Application Name: {{appName}}
Version: {{version}}
Build Number (Static): {{buildNumber}}
Author: {{author}}

--- Dynamic Values ---
Session ID 1: {{newUuid}}
Session ID 2: {{newUuid}}
Timestamp 1: {{currentTimestamp}}
Timestamp 2: {{currentTimestamp}}
Random Number 1: {{getRandomInt}}
Random Number 2: {{getRandomInt}}
Greeting: {{getGreeting}})";
    ws->templateTextEdit->setPlainText(templateContent.isEmpty() ? exampleTemplate : templateContent);
    runTemplateTabLayout->addWidget(ws->templateTextEdit);
    runTemplateTabLayout->setStretch(1, 1);
    ws->templateTextEdit->setFont(m_font(mfontContentSize));
    connect(ws->templateTextEdit, &CodeEditor::textChanged, this, &JSVariableReplacer::onTemplateChanged);

    ws->runButton = new QPushButton("Run Script & Replace", ws->runTemplateTab);
    connect(ws->runButton, &QPushButton::clicked, this, &JSVariableReplacer::runScriptAndReplace);
    runTemplateTabLayout->addWidget(ws->runButton);
    ws->runTemplateTab->setLayout(runTemplateTabLayout);

    // --- Result Tab ---
    QVBoxLayout *resultTabLayout = new QVBoxLayout(ws->resultTab);
    QLabel *outputLabel = new QLabel("Result:", ws->resultTab);
    resultTabLayout->addWidget(outputLabel);

    ws->resultDisplayTextEdit = new QTextEdit(ws->resultTab);
    ws->resultDisplayTextEdit->setReadOnly(true);
    ws->resultDisplayTextEdit->setFont(font);
    resultTabLayout->addWidget(ws->resultDisplayTextEdit);
    resultTabLayout->setStretch(1, 1);
    ws->resultTab->setLayout(resultTabLayout);

    mainWorkspaceTabWidget->insertTab(index, ws->containerWidget, "Workspace " + QString::number(index + 1));
    workspaces.insert(index, ws);

    updateButtonStates(); // Update button state after adding
    qDebug() << "createWorkspace: Finish for index" << index;
}

// --- Slots and other methods ---
void JSVariableReplacer::addNewWorkspace()
{
    qDebug() << "addNewWorkspace: Start";
    int newIndex = mainWorkspaceTabWidget->count();
    createWorkspace(newIndex);
    workspaceCounter = mainWorkspaceTabWidget->count() + 1;
    mainWorkspaceTabWidget->setCurrentIndex(newIndex);
    saveAllWorkspacesToSettings();
    qDebug() << "addNewWorkspace: Finish";
    // updateButtonStates is called inside createWorkspace
}

void JSVariableReplacer::removeCurrentWorkspace()
{
    qDebug() << "removeCurrentWorkspace: Start";
    int currentIndex = mainWorkspaceTabWidget->currentIndex();
    if (currentIndex != -1 && mainWorkspaceTabWidget->count() > 1) {
        QWidget *widgetToRemove = mainWorkspaceTabWidget->widget(currentIndex);
        mainWorkspaceTabWidget->removeTab(currentIndex);

        auto it = workspaces.find(currentIndex);
        if (it != workspaces.end()) {
            delete it.value();
            workspaces.erase(it);
        }

        // Re-index remaining workspaces
        QMap<int, Workspace*> reindexedWorkspaces;
        for (int i = 0; i < mainWorkspaceTabWidget->count(); ++i) {
            QWidget *tabWidget = mainWorkspaceTabWidget->widget(i);
            for (auto ws_it = workspaces.begin(); ws_it != workspaces.end(); ) {
                if (ws_it.value()->containerWidget == tabWidget) {
                    Workspace *ws = ws_it.value();
                    workspaces.erase(ws_it);
                    mainWorkspaceTabWidget->setTabText(i, "Workspace " + QString::number(i + 1));
                    reindexedWorkspaces.insert(i, ws);
                    break;
                } else {
                    ++ws_it;
                }
            }
        }
        workspaces = reindexedWorkspaces;
        workspaceCounter = mainWorkspaceTabWidget->count() + 1;

        saveAllWorkspacesToSettings();
        updateButtonStates(); // Update button state after removing
    } else {
        QMessageBox::information(this, "Cannot Remove", "You must have at least one workspace.");
    }
    qDebug() << "removeCurrentWorkspace: Finish";
}

Workspace* JSVariableReplacer::getCurrentWorkspace() const
{
    int currentIndex = mainWorkspaceTabWidget->currentIndex();
    if (currentIndex == -1) return nullptr;
    auto it = workspaces.constFind(currentIndex);
    return (it != workspaces.constEnd()) ? it.value() : nullptr;
}

void JSVariableReplacer::onScriptChanged() { saveAllWorkspacesToSettings(); }
void JSVariableReplacer::onTemplateChanged() { saveAllWorkspacesToSettings(); }
void JSVariableReplacer::onTabChanged(int index) { Q_UNUSED(index); saveActiveWorkspaceIndex(); }

void JSVariableReplacer::saveAllWorkspacesToSettings()
{
    settings.setValue(SETTINGS_WORKSPACE_COUNT_KEY, mainWorkspaceTabWidget->count());
    QStringList keys = settings.allKeys();
    for(const QString &key : keys) {
        if (key.startsWith(SETTINGS_WORKSPACE_SCRIPT_KEY_PREFIX) || key.startsWith(SETTINGS_WORKSPACE_TEMPLATE_KEY_PREFIX)) {
            settings.remove(key);
        }
    }
    for (auto it = workspaces.constBegin(); it != workspaces.constEnd(); ++it) {
        int index = it.key();
        Workspace *ws = it.value();
        if (ws && ws->scriptEditorTextEdit && ws->templateTextEdit) {
            settings.setValue(SETTINGS_WORKSPACE_SCRIPT_KEY_PREFIX + QString::number(index), ws->scriptEditorTextEdit->text());
            settings.setValue(SETTINGS_WORKSPACE_TEMPLATE_KEY_PREFIX + QString::number(index), ws->templateTextEdit->toPlainText());
        }
    }
    settings.sync();
}

void JSVariableReplacer::loadWorkspacesFromSettings()
{
    int count = settings.value(SETTINGS_WORKSPACE_COUNT_KEY, 0).toInt();
    for (int i = 0; i < count; ++i) {
        QString scriptKey = SETTINGS_WORKSPACE_SCRIPT_KEY_PREFIX + QString::number(i);
        QString templateKey = SETTINGS_WORKSPACE_TEMPLATE_KEY_PREFIX + QString::number(i);
        QString scriptContent = settings.value(scriptKey, QString()).toString();
        QString templateContent = settings.value(templateKey, QString()).toString();
        createWorkspace(i, scriptContent, templateContent);
    }
    workspaceCounter = count + 1;
    updateButtonStates(); // Ensure state is correct after loading
}

void JSVariableReplacer::saveActiveWorkspaceIndex()
{
    int currentIndex = mainWorkspaceTabWidget->currentIndex();
    settings.setValue(SETTINGS_ACTIVE_WORKSPACE_INDEX_KEY, currentIndex);
    settings.sync();
}

void JSVariableReplacer::loadActiveWorkspaceIndex()
{
    int savedIndex = settings.value(SETTINGS_ACTIVE_WORKSPACE_INDEX_KEY, 0).toInt();
    if (savedIndex >= 0 && savedIndex < mainWorkspaceTabWidget->count()) {
        mainWorkspaceTabWidget->setCurrentIndex(savedIndex);
    }
}

void JSVariableReplacer::runScriptAndReplace()
{
    Workspace* currentWS = getCurrentWorkspace();
    if (!currentWS) {
        QMessageBox::critical(this, "Error", "Could not find the current workspace.");
        return;
    }

    QString script = currentWS->scriptEditorTextEdit->text();
    QString templateText = currentWS->templateTextEdit->toPlainText();

    if (script.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please define a JavaScript script.");
        currentWS->innerTabWidget->setCurrentWidget(currentWS->scriptEditorTab);
        return;
    }
    if (templateText.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a template text.");
        return;
    }

    QJSEngine engine;
    QJSValue result = engine.evaluate(script);

    if (result.isError()) {
        QString errorMessage = QString("JS Error: %1\nLine: %2\nFile: %3")
                                   .arg(result.toString())
                                   .arg(result.property("lineNumber").toInt())
                                   .arg(result.property("fileName").toString());
        currentWS->resultDisplayTextEdit->setPlainText(errorMessage);
        currentWS->innerTabWidget->setCurrentWidget(currentWS->resultTab);
        return;
    }

    // --- 核心修改：支持 vars 中的函数和 functions 中的函数 ---
    QMap<QString, QString> staticVariables;    // 存储最终的静态值 (字符串)
    QMap<QString, QJSValue> callableVariables; // 存储来自 result.functions 的函数

    if (result.isObject()) {
        // 1. 提取并处理静态变量 (vars)
        //    - 直接量存入 staticVariables
        //    - 函数立即执行一次，结果存入 staticVariables
        QJSValue varsObject = result.property("vars");
        if (varsObject.isObject()) {
            QJSValueIterator varsIt(varsObject);
            while (varsIt.hasNext()) {
                varsIt.next();
                QString varName = varsIt.name();
                QJSValue varValue = varsIt.value();

                if (varValue.isCallable()) {
                    // --- 关键点：如果 vars 中的值是函数，则立即调用 ---
                    qDebug() << "[JS vars] Found callable in 'vars' for key:" << varName << ". Executing once to get static value.";
                    QJSValue callResult = varValue.call(); // 立即调用
                    if (callResult.isError()) {
                        QString funcErrorMsg = QString("JS Function Error in 'vars.%1': %2\nLine: %3\nFile: %4")
                                                   .arg(varName)
                                                   .arg(callResult.toString())
                                                   .arg(callResult.property("lineNumber").toInt())
                                                   .arg(callResult.property("fileName").toString());
                        qDebug() << funcErrorMsg;
                        // 将错误信息作为该静态变量的值存储，或可以选择跳过
                        staticVariables.insert(varName, "[Error calling vars." + varName + "]");
                    } else {
                        // 将函数调用的结果（转换为字符串）作为静态变量存储
                        staticVariables.insert(varName, callResult.toString());
                        qDebug() << "[JS vars] Stored result of 'vars." << varName << "' as static variable:" << callResult.toString();
                        // 注意：如果需要保留复杂对象结构，这里需要更复杂的处理，例如 JSON.stringify
                    }
                } else {
                    // 如果不是函数，直接作为静态值存储
                    staticVariables.insert(varName, varValue.toString());
                    qDebug() << "[JS vars] Found static variable:" << varName << "=" << varValue.toString();
                }
            }
        } else if (!varsObject.isUndefined()) {
            QString errorMsg = "If present, 'vars' property in script result must be an object.";
            currentWS->resultDisplayTextEdit->setPlainText(errorMsg);
            currentWS->innerTabWidget->setCurrentWidget(currentWS->resultTab);
            return;
        }

        // 2. 提取可调用函数 (functions)
        //    - 只存储函数引用，供模板替换时调用
        QJSValue functionsObject = result.property("functions");
        if (functionsObject.isObject()) {
            QJSValueIterator funcsIt(functionsObject);
            while (funcsIt.hasNext()) {
                funcsIt.next();
                QString funcName = funcsIt.name();
                QJSValue funcValue = funcsIt.value();

                if (funcValue.isCallable()) {
                    callableVariables.insert(funcName, funcValue);
                    qDebug() << "[JS functions] Found callable variable:" << funcName;
                } else {
                    qDebug() << "Warning: Non-callable property '" << funcName << "' found in 'functions' object, ignoring.";
                }
            }
        } else if (!functionsObject.isUndefined()) {
            QString errorMsg = "If present, 'functions' property in script result must be an object.";
            currentWS->resultDisplayTextEdit->setPlainText(errorMsg);
            currentWS->innerTabWidget->setCurrentWidget(currentWS->resultTab);
            return;
        }

    } else {
        QString errorMsg = "Script must return an object with optional 'vars' and 'functions' properties.";
        currentWS->resultDisplayTextEdit->setPlainText(errorMsg);
        currentWS->innerTabWidget->setCurrentWidget(currentWS->resultTab);
        return;
    }

    // --- 3. 处理模板替换 ---
    QString outputText = templateText;
    QRegularExpression placeholderRegex(R"(\{\{([^}]+)\}\})");
    QRegularExpressionMatchIterator matchIterator = placeholderRegex.globalMatch(outputText);

    QStringList allMatches;
    QList<int> matchStarts, matchLengths;

    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        allMatches.append(match.captured(1));
        matchStarts.append(match.capturedStart());
        matchLengths.append(match.capturedLength());
    }

    // 反向遍历进行替换，避免索引偏移
    for (int i = allMatches.size() - 1; i >= 0; --i) {
        QString placeholderName = allMatches[i];
        int start = matchStarts[i];
        int length = matchLengths[i];
        QString replacementValue;

        // --- 4. 替换逻辑：优先调用 functions 中的函数，再查找 vars 中的静态变量 ---
        // a. 首先检查是否是 JS functions 中的可调用函数 (动态)
        auto callableIt = callableVariables.find(placeholderName);
        if (callableIt != callableVariables.end()) {
            QJSValue function = callableIt.value();
            QJSValue functionResult = function.call(); // 每次都调用
            if (functionResult.isError()) {
                QString funcErrorMsg = QString("JS Function Error in '%1': %2\nLine: %3\nFile: %4")
                                           .arg(placeholderName)
                                           .arg(functionResult.toString())
                                           .arg(functionResult.property("lineNumber").toInt())
                                           .arg(functionResult.property("fileName").toString());
                qDebug() << funcErrorMsg;
                replacementValue = "[Function Error: " + placeholderName + "]";
            } else {
                replacementValue = functionResult.toString();
                qDebug() << "[Template Replace] Executed JS function" << placeholderName << "with result:" << replacementValue;
            }
        } else {
            // b. 如果不是动态函数，检查 JS vars 中的静态变量
            auto staticIt = staticVariables.find(placeholderName);
            if (staticIt != staticVariables.end()) {
                replacementValue = staticIt.value();
                qDebug() << "[Template Replace] Used JS static variable" << placeholderName << "=" << replacementValue;
            } else {
                // c. 如果都找不到，保留原占位符
                replacementValue = "{{" + placeholderName + "}}";
                qDebug() << "[Template Replace] Warning: Placeholder '{{" << placeholderName << "}}' not found in JS vars or functions.";
            }
        }

        outputText.replace(start, length, replacementValue);
    }

    currentWS->resultDisplayTextEdit->setPlainText(outputText);
    currentWS->innerTabWidget->setCurrentWidget(currentWS->resultTab);
}


void JSVariableReplacer::onCursorChanged()
{
    Workspace *ws = getCurrentWorkspace(); // 获取当前 workspace
    if (!ws || !ws->templateTextEdit || !ws->highlighter) { // 检查有效性
        qDebug() << "onCursorChanged: Invalid workspace or components";
        return;
    }
    QString sql = ws->templateTextEdit->toPlainText();
    QTextCursor cursor = ws->templateTextEdit->textCursor();
    int pos = cursor.position();

    // 使用当前 workspace 的 highlighter
    // ... (使用 ws->highlighter 替代原来的 highlighter) ...
    QList<int> stmtStarts, stmtEnds;
    int last = 0;
    for (int i=0; i<sql.length(); ++i) {
        if (sql[i] == ';') {
            stmtStarts << last;
            stmtEnds << i;
            last = i+1;
        }
    }

    int stmtIdx = -1;
    for (int i=0; i<stmtStarts.size(); ++i) {
        if (pos >= stmtStarts[i] && pos <= stmtEnds[i]) {
            stmtIdx = i;
            break;
        }
    }
    if(stmtIdx == -1) {
        ws->highlighter->clearHighlight(); // <-- 使用 ws->highlighter
        return;
    }

    int stmtStart = stmtStarts[stmtIdx];
    int stmtEnd = stmtEnds[stmtIdx];
    QString stmt = sql.mid(stmtStart, stmtEnd-stmtStart+1);
    QList<FieldValuePos> pairs = SqlParser::parseInsertWithPos(stmt);

    for(auto &pair:pairs) {
        pair.fieldStart += stmtStart;
        pair.fieldEnd += stmtStart;
        pair.valueStart += stmtStart;
        pair.valueEnd += stmtStart;
    }

    bool found = false;
    for (const auto &pair : pairs) {
        if ((pair.fieldStart <= pos && pos <= pair.fieldEnd) ||
            (pair.valueStart <= pos && pos <= pair.valueEnd)) {
            QColor color = QColor::fromHsv(qHash(pair.field) % 360, 180, 230);
            // 使用当前 workspace 的 highlighter
            ws->highlighter->setHighlightRegion(pair.fieldStart, pair.fieldEnd, pair.valueStart, pair.valueEnd, color); // <-- 使用 ws->highlighter
            found = true;
            break;
        }
    }
    if (!found) {
        ws->highlighter->clearHighlight(); // <-- 使用 ws->highlighter
    }
}
