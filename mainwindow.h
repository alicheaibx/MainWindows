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
    void windowShown();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void handleLayoutOperation(const QString &fileName);

private:
    void initializeWindow();
    void setupCentralWidget();
    void setupManagers();
    void connectSignals();
    void loadDefaultLayout();

    DockManager *m_dockManager;
    LayoutManager *m_layoutManager;
    MenuManager *m_menuManager;
};

#endif // MAINWINDOW_H
