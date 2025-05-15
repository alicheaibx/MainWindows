#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <QObject>
#include <QMainWindow>

class QToolBar;
class QPushButton;

class MenuManager : public QObject
{
    Q_OBJECT

public:
    explicit MenuManager(QMainWindow *parent = nullptr);

    bool isResizeEnabled() const { return m_resizeEnabled; }

signals:
    void layoutOperationRequested(const QString &fileName);
    void resizeEnabledChanged(bool enabled);

private slots:
    void setResizeEnabled(bool enabled);

private:
    void setupMenuBar();
    void setupLayoutToolBar();

    QMainWindow *m_mainWindow;
    QAction *m_resizeAction;
    bool m_resizeEnabled = false;
};

#endif // MENUMANAGER_H
