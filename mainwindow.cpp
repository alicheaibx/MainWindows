#include "mainwindow.h"
#include "dockmanager.h"
#include "layoutmanager.h"
#include "menumanager.h"
#include <QTextEdit>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QCloseEvent>
#include <QShowEvent>
#include <QDebug>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    setObjectName("MainWindow");
    setWindowTitle("Qt Main Window Example");
    setDockNestingEnabled(true);

    setupCentralWidget();

    // Initialize managers
    m_dockManager = new DockManager(this);
    m_layoutManager = new LayoutManager(this);
    m_menuManager = new MenuManager(this);

    // Connect menu signals
    connect(m_menuManager, &MenuManager::saveLayoutRequested, this, &MainWindow::saveLayout);
    connect(m_menuManager, &MenuManager::saveLayoutAsRequested, this, &MainWindow::saveLayoutAs);
    connect(m_menuManager, &MenuManager::loadLayoutRequested, this, &MainWindow::loadLayout);
    connect(m_menuManager, &MenuManager::loadLayout1Requested, this, &MainWindow::loadLayout1);
    connect(m_menuManager, &MenuManager::loadLayout2Requested, this, &MainWindow::loadLayout2);
    connect(m_menuManager, &MenuManager::loadLayout3Requested, this, &MainWindow::loadLayout3);
    connect(m_menuManager, &MenuManager::loadLayout4Requested, this, &MainWindow::loadLayout4);
    connect(m_menuManager, &MenuManager::loadLayout5Requested, this, &MainWindow::loadLayout5);

    // Connect layout manager signals to dock manager
    connect(m_layoutManager, &LayoutManager::saveDockWidgetsLayoutRequested,
            m_dockManager, &DockManager::saveDockWidgetsLayout);
    connect(m_layoutManager, &LayoutManager::loadDockWidgetsLayoutRequested,
            m_dockManager, &DockManager::loadDockWidgetsLayout);

    // Load default layout if exists
    QFile layoutFile("layout.xml");
    if (layoutFile.exists()) {
        m_layoutManager->loadLayoutFromFile("layout.xml");
    }
}

MainWindow::~MainWindow()
{
    delete m_dockManager;
    delete m_layoutManager;
    delete m_menuManager;
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
                       "Use the buttons below to quickly switch between layouts."));
    setCentralWidget(center);
}

void MainWindow::saveLayout()
{
    m_layoutManager->saveLayoutToFile("layout.xml");
    QMessageBox::information(this, tr("Save Layout"), tr("Layout saved to layout.xml"));
}

void MainWindow::saveLayoutAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Layout As"), "", tr("XML Files (*.xml)"));
    if (!fileName.isEmpty()) {
        m_layoutManager->saveLayoutToFile(fileName);
        QMessageBox::information(this, tr("Save Layout As"), tr("Layout saved to %1").arg(fileName));
    }
}

void MainWindow::loadLayout()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Layout"), "", tr("XML Files (*.xml)"));
    if (!fileName.isEmpty()) {
        m_layoutManager->loadLayoutFromFile(fileName);
        QMessageBox::information(this, tr("Load Layout"), tr("Layout loaded from %1").arg(fileName));
    }
}

void MainWindow::loadLayout1()
{
    if (QFile::exists("layout.xml")) {
        m_layoutManager->loadLayoutFromFile("layout.xml");
    } else {
        QMessageBox::warning(this, tr("Error"), tr("layout.xml not found"));
    }
}

void MainWindow::loadLayout2()
{
    if (QFile::exists("layout2.xml")) {
        m_layoutManager->loadLayoutFromFile("layout2.xml");
    } else {
        QMessageBox::warning(this, tr("Error"), tr("layout2.xml not found"));
    }
}

void MainWindow::loadLayout3()
{
    if (QFile::exists("layout3.xml")) {
        m_layoutManager->loadLayoutFromFile("layout3.xml");
    } else {
        QMessageBox::warning(this, tr("Error"), tr("layout3.xml not found"));
    }
}

void MainWindow::loadLayout4()
{
    if (QFile::exists("layout4.xml")) {
        m_layoutManager->loadLayoutFromFile("layout4.xml");
    } else {
        QMessageBox::warning(this, tr("Error"), tr("layout4.xml not found"));
    }
}

void MainWindow::loadLayout5()
{
    if (QFile::exists("layout5.xml")) {
        m_layoutManager->loadLayoutFromFile("layout5.xml");
    } else {
        QMessageBox::warning(this, tr("Error"), tr("layout5.xml not found"));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    emit shown();
}
