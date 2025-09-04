#ifndef JSVARIABLEREPLACER_H
#define JSVARIABLEREPLACER_H

#include <QWidget>
#include <QMap>
#include <QSettings>

class QTextEdit;
class QPushButton;
class QTabWidget;
class QAction;

class CodeEditor;
class SqlHighlighter;
class SqlLexer;

struct Workspace;

class JSVariableReplacer : public QWidget
{
    Q_OBJECT

public:
    explicit JSVariableReplacer(QWidget *parent = nullptr);
    ~JSVariableReplacer();

protected:
    void closeEvent(QCloseEvent *event) override;


private slots:
    void runScriptAndReplace();
    void addNewWorkspace();
    void removeCurrentWorkspace();
    void onScriptChanged();
    void onTemplateChanged();
    void onTabChanged(int index);

    void onCursorChanged();

private:
    void setupUI();
    void createWorkspace(int index, const QString& scriptContent = QString(), const QString& templateContent = QString());
    void saveAllWorkspacesToSettings();
    void loadWorkspacesFromSettings();
    void saveActiveWorkspaceIndex();
    void loadActiveWorkspaceIndex();
    Workspace* getCurrentWorkspace() const;
    void updateButtonStates(); // Make sure this is declared

    QTabWidget *mainWorkspaceTabWidget;
    QAction *addAction;
    QAction *removeAction;

    QMap<int, Workspace*> workspaces;
    int workspaceCounter;

    QSettings settings; // This will use QCoreApplication name/org set in main()

    QFont m_font(const int &fontsize);

    int mfontContentSize = 11;

};

#endif // JSVARIABLEREPLACER_H



