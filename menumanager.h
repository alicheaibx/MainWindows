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
    void setupMenuBar();
    void setupLayoutToolBar();

signals:
    void saveLayoutRequested();
    void saveLayoutAsRequested();
    void loadLayoutRequested();
    void loadLayout1Requested();
    void loadLayout2Requested();
    void loadLayout3Requested();
    void loadLayout4Requested();
    void loadLayout5Requested();

private:
    QMainWindow *m_mainWindow;
    QToolBar *m_layoutToolBar;
};

#endif // MENUMANAGER_H
