#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class DockManager;
class LayoutManager;
class MenuManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~MainWindow();

signals:
    void shown();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void saveLayout();
    void saveLayoutAs();
    void loadLayout();
    void loadLayout1();
    void loadLayout2();
    void loadLayout3();
    void loadLayout4();
    void loadLayout5();

private:
    void setupCentralWidget();

    DockManager *m_dockManager;
    LayoutManager *m_layoutManager;
    MenuManager *m_menuManager;
};

#endif // MAINWINDOW_H
