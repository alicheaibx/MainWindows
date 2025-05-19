#include "mainwindow.h"
#include "layoutmanager.h"
#include "menumanager.h"
#include <QTextEdit>
#include <QFile>
#include <QMessageBox>
#include <QTimer>
#include <QCloseEvent>

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
                       "Layouts can be saved and loaded from the File menu.\n"
                       "Use the 'Allow Resizing' option to control widget movement."));
    setCentralWidget(center);
}

void MainWindow::setupManagers()
{
    m_layoutManager = new LayoutManager(this);
    m_menuManager = new MenuManager(this);
}

void MainWindow::connectSignals()
{
    // Connect menu signals
    connect(m_menuManager, &MenuManager::layoutOperationRequested,
            this, &MainWindow::handleLayoutOperation);

    // Connect resize signal directly to layout manager
    connect(m_menuManager, &MenuManager::resizeEnabledChanged,
            m_layoutManager, &LayoutManager::setResizeEnabled);

    // Load default layout after window is shown
    connect(this, &MainWindow::windowShown, this, &MainWindow::loadDefaultLayout);
}

void MainWindow::loadDefaultLayout()
{
    QTimer::singleShot(100, this, [this]() {
        if (QFile::exists("layout1.xml")) {
            m_layoutManager->loadLayoutFromFile("layout1.xml");
        }
    });
}

void MainWindow::handleLayoutOperation(const QString &fileName)
{
    if (fileName.isEmpty()) return;

    if (fileName.startsWith("save:")) {
        QString saveFile = fileName.mid(5);
        m_layoutManager->saveLayoutToFile(saveFile);
    }
    else if (QFile::exists(fileName)) {
        m_layoutManager->loadLayoutFromFile(fileName);
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
