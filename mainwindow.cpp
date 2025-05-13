#include "mainwindow.h"
#include "dockmanager.h"
#include "layoutmanager.h"
#include "menumanager.h"
#include <QTextEdit>
#include <QFile>
#include <QMessageBox>
#include <QTimer>
#include<QCloseEvent>
MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    setObjectName("MainWindow");
    setWindowTitle("Qt Main Window Example");
    setDockNestingEnabled(true);

    initializeWindow();
}

MainWindow::~MainWindow()
{
    delete m_dockManager;
    delete m_layoutManager;
    delete m_menuManager;
}

void MainWindow::initializeWindow()
{
    setupCentralWidget();
    setupManagers();
    connectSignals();
}

void MainWindow::setupCentralWidget()
{
    QTextEdit *center = new QTextEdit(this);
    center->setReadOnly(true);
    center->setMinimumSize(400, 205);
    center->setText(tr("This is the central widget.\n\n"
                       "You can dock other widgets around this area.\n"
                       "Use the View menu to toggle dock widgets.\n"
                       "Layouts can be saved and loaded from the File menu."));
    setCentralWidget(center);
}

void MainWindow::setupManagers()
{
    m_dockManager = new DockManager(this);
    m_layoutManager = new LayoutManager(this);
    m_menuManager = new MenuManager(this);
}

void MainWindow::connectSignals()
{
    // Connect menu signals
    connect(m_menuManager, &MenuManager::layoutOperationRequested,
            this, &MainWindow::handleLayoutOperation);

    // Connect layout manager signals to dock manager
    connect(m_layoutManager, &LayoutManager::saveDockWidgetsLayoutRequested,
            m_dockManager, &DockManager::saveDockWidgetsLayout);
    connect(m_layoutManager, &LayoutManager::loadDockWidgetsLayoutRequested,
            m_dockManager, &DockManager::loadDockWidgetsLayout);

    // Load default layout after window is shown
    connect(this, &MainWindow::windowShown, this, &MainWindow::loadDefaultLayout);
}

void MainWindow::loadDefaultLayout()
{
    QTimer::singleShot(100, this, [this]() {
        if (QFile::exists("layout.xml")) {
            m_layoutManager->loadLayoutFromFile("layout.xml");
        }
    });
}

void MainWindow::handleLayoutOperation(const QString &fileName)
{
    if (fileName.isEmpty()) return;

    if (fileName.startsWith("save:")) {
        QString saveFile = fileName.mid(5);
        m_layoutManager->saveLayoutToFile(saveFile);
        QMessageBox::information(this, tr("Success"), tr("Layout saved to %1").arg(saveFile));
    }
    else if (QFile::exists(fileName)) {
        m_layoutManager->loadLayoutFromFile(fileName);
        QMessageBox::information(this, tr("Success"), tr("Layout loaded from %1").arg(fileName));
    }
    else {
        QMessageBox::warning(this, tr("Error"), tr("File not found: %1").arg(fileName));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    emit windowShown();
}
