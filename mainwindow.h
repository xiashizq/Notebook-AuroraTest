#pragma once
#include <QMainWindow>
#include <QDebug>
#include <QFileInfo>
#include <QEvent>
#include "DiffFunction/DiffWidget.h"
#include "folderDiff/CompareDialog.h"
class QsciScintilla;
class SqlHighlighter;
class SqlLexer;
class DiffWidget;
class CompareDialog;
#pragma execution_character_set("utf-8")

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    //    void onCursorChanged();
    void openNewWindow();
    void updateLineNumberWidth(QsciScintilla *textEdit);
    void addNewTabMneu();
    void addNewTab(const QString &filePath);
    void closeTab(int index);
    bool saveFile(int index);
    int extractNumberFromUntitled(const QString &filename);
    void openRecentFile();

    void openFileNoPar();           // 无参数：弹出文件选择对话框
    void openFile(const QString &path); // 有路径：直接打开
    void onTabMoved(int from, int to);

    void codeHightLightFunction(const QString& language);

    void openDiffWidget();
    void onOpenFolderCompareTool();


protected:
    void closeEvent(QCloseEvent *event) override;  //  声明为 protected

private:
    QsciScintilla *textEdit;
    SqlHighlighter *highlighter;
    QsciScintilla* getCurrentEditor();
    int m_untitledCount;
    bool m_findStarted = false;
    QFont m_font(const int &fontsize);
    struct TabInfo {
        QsciScintilla *editor;
        QString filePath;     // 实际文件路径，恢复的未命名文件为空
        QString baseName;     // 显示名称，如 "未命名1" 或 "report.sql"
        bool isModified;
        QString autoSavePath;
        bool neverSavedByUser = true;
        QString language;
        // 获取显示名称（用于 tab 标签）
        QString displayName() const {
            return baseName.isEmpty() ? "新文件" : baseName;
        }
    };
    QVector<TabInfo> m_tabs;        // 存储所有 tab 信息
    QTabWidget *m_tabWidget;        // 指向 tab widget

    QTimer *m_autoSaveTimer; // 自动保存定时器
    QString autoSaveDir();   // 返回自动保存目录（如 temp 或 app dir）
    void autoSaveAll();      // 保存所有可保存的 tab
    QString autoSavePathForTab(const TabInfo &info); // 为每个 tab 生成自动保存路径

    void saveSession();    // 关闭前保存当前会话
    void restoreSession();    // 恢复会话
    QStringList getLastOpenFiles(); // 从设置中读取
    void setLastOpenFiles(const QStringList &files); // 写入设置
    bool closeTabWithPrompt(int index); //  返回 false 表示取消关闭

    bool formatJson(); //JSON美化
    bool formatXml();  //XML美化

    QMenu *m_recentFilesMenu;
    void setupRecentFilesMenu();
    void updateRecentFilesMenu();

    void setEditorLexer(QsciScintilla *editor, const QString &language);
    void setLexerForLanguage(QsciScintilla *editor, const QString &language);

    void onSave();           // Ctrl+S
    void onSaveAs();         // Ctrl+Shift+S
    void onCloseTab();       // Ctrl+W
    void onNewFile();        // Ctrl+N
    void onOpenFile();       // Ctrl+O

    QDialog *m_findReplaceDialog = nullptr; // 查找替换窗口（非模态）
    void openFindReplaceDialog(); //查找替换窗口（非模态）
    void showMessageInCenter(const QString &title, const QString &text); //主窗口弹窗效果
    QString unescapeString(const QString& s);

};
