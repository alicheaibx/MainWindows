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

signals:
    void layoutOperationRequested(const QString &fileName);

private:
    void setupMenuBar();
    void setupLayoutToolBar();

    QMainWindow *m_mainWindow;
};

#endif // MENUMANAGER_H
