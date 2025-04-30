#include "menumanager.h"
#include <QMenuBar>
#include <QAction>
#include <QMainWindow>
#include <QToolBar>
#include <QPushButton>
#include <QLabel>

MenuManager::MenuManager(QMainWindow *parent)
    : QObject(parent),
    m_mainWindow(parent),
    m_layoutToolBar(nullptr)
{
    setupMenuBar();
    setupLayoutToolBar();
}

void MenuManager::setupMenuBar()
{
    QMenu *fileMenu = m_mainWindow->menuBar()->addMenu(tr("&File"));

    QAction *saveLayoutAction = fileMenu->addAction(tr("Save Layout"));
    connect(saveLayoutAction, &QAction::triggered, this, &MenuManager::saveLayoutRequested);

    QAction *saveLayoutAsAction = fileMenu->addAction(tr("Save Layout As..."));
    connect(saveLayoutAsAction, &QAction::triggered, this, &MenuManager::saveLayoutAsRequested);

    QAction *loadLayoutAction = fileMenu->addAction(tr("Load Layout..."));
    connect(loadLayoutAction, &QAction::triggered, this, &MenuManager::loadLayoutRequested);

    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), m_mainWindow, &QWidget::close);
}

void MenuManager::setupLayoutToolBar()
{
    // Create the toolbar with improved styling
    m_layoutToolBar = new QToolBar(tr("Layouts"), m_mainWindow);
    m_layoutToolBar->setObjectName("LayoutToolBar");
    m_layoutToolBar->setStyleSheet(
        "QToolBar {"
        "   background: #f0f0f0;"
        "   border-top: 1px solid #ccc;"
        "   border-bottom: 1px solid #ccc;"
        "   spacing: 5px;"
        "   padding: 3px;"
        "}"
        "QToolButton {"
        "   padding: 5px;"
        "   margin: 2px;"
        "}"
        );
    m_layoutToolBar->setMovable(false);
    m_layoutToolBar->setFloatable(false);
    m_layoutToolBar->setAllowedAreas(Qt::TopToolBarArea);
    m_mainWindow->addToolBar(Qt::TopToolBarArea, m_layoutToolBar);

    // Button style
    QString buttonStyle =
        "QPushButton {"
        "   padding: 6px;"
        "   margin: 2px;"
        "   min-width: 80px;"
        "   background: #e0e0e0;"
        "   border: 1px solid #aaa;"
        "   border-radius: 3px;"
        "}"
        "QPushButton:hover {"
        "   background: #d0d0d0;"
        "}"
        "QPushButton:pressed {"
        "   background: #c0c0c0;"
        "}";

    // Create layout buttons
    QPushButton *btn1 = new QPushButton(tr("Layout 1"), m_mainWindow);
    QPushButton *btn2 = new QPushButton(tr("Layout 2"), m_mainWindow);
    QPushButton *btn3 = new QPushButton(tr("Layout 3"), m_mainWindow);
    QPushButton *btn4 = new QPushButton(tr("Layout 4"), m_mainWindow);
    QPushButton *btn5 = new QPushButton(tr("Layout 5"), m_mainWindow);

    // Apply style to buttons
    btn1->setStyleSheet(buttonStyle);
    btn2->setStyleSheet(buttonStyle);
    btn3->setStyleSheet(buttonStyle);
    btn4->setStyleSheet(buttonStyle);
    btn5->setStyleSheet(buttonStyle);

    // Connect buttons
    connect(btn1, &QPushButton::clicked, this, &MenuManager::loadLayout1Requested);
    connect(btn2, &QPushButton::clicked, this, &MenuManager::loadLayout2Requested);
    connect(btn3, &QPushButton::clicked, this, &MenuManager::loadLayout3Requested);
    connect(btn4, &QPushButton::clicked, this, &MenuManager::loadLayout4Requested);
    connect(btn5, &QPushButton::clicked, this, &MenuManager::loadLayout5Requested);

    // Add buttons to toolbar
    m_layoutToolBar->addWidget(new QLabel(tr("Presets:")));
    m_layoutToolBar->addWidget(btn1);
    m_layoutToolBar->addWidget(btn2);
    m_layoutToolBar->addWidget(btn3);
    m_layoutToolBar->addWidget(btn4);
    m_layoutToolBar->addWidget(btn5);

    // Add separator
    m_layoutToolBar->addSeparator();

    // Add save/load buttons
    QPushButton *saveBtn = new QPushButton(tr("Save Current"), m_mainWindow);
    QPushButton *loadBtn = new QPushButton(tr("Load Custom"), m_mainWindow);
    saveBtn->setStyleSheet(buttonStyle);
    loadBtn->setStyleSheet(buttonStyle);
    connect(saveBtn, &QPushButton::clicked, this, &MenuManager::saveLayoutAsRequested);
    connect(loadBtn, &QPushButton::clicked, this, &MenuManager::loadLayoutRequested);
    m_layoutToolBar->addWidget(saveBtn);
    m_layoutToolBar->addWidget(loadBtn);
}
