#include "menumanager.h"
#include <QMenuBar>
#include <QAction>
#include <QToolBar>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>

MenuManager::MenuManager(QMainWindow *parent)
    : QObject(parent),
    m_mainWindow(parent)
{
    setupMenuBar();
    setupLayoutToolBar();
}

void MenuManager::setupMenuBar()
{
    QMenu *fileMenu = m_mainWindow->menuBar()->addMenu(tr("&File"));

    QAction *saveAction = fileMenu->addAction(tr("Save Layout As..."));
    connect(saveAction, &QAction::triggered, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(m_mainWindow,
                                                        tr("Save Layout"), "", tr("Layout Files (*.xml)"));
        if (!fileName.isEmpty()) {
            emit layoutOperationRequested("save:" + fileName);
        }
    });

    QAction *loadAction = fileMenu->addAction(tr("Load Layout..."));
    connect(loadAction, &QAction::triggered, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(m_mainWindow,
                                                        tr("Load Layout"), "", tr("Layout Files (*.xml)"));
        if (!fileName.isEmpty()) {
            emit layoutOperationRequested(fileName);
        }
    });

    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), m_mainWindow, &QWidget::close);

    // Add resize toggle to View menu
    QMenu *viewMenu = m_mainWindow->menuBar()->addMenu(tr("&View"));
    m_resizeAction = viewMenu->addAction(tr("Allow Resizing"));
    m_resizeAction->setCheckable(true);
    m_resizeAction->setChecked(m_resizeEnabled);
    connect(m_resizeAction, &QAction::toggled, this, &MenuManager::setResizeEnabled);
}

void MenuManager::setupLayoutToolBar()
{
    QToolBar *toolBar = new QToolBar(tr("Layouts"), m_mainWindow);
    toolBar->setObjectName("LayoutToolBar");
    toolBar->setMovable(false);
    m_mainWindow->addToolBar(Qt::TopToolBarArea, toolBar);

    // Add preset buttons
    toolBar->addWidget(new QLabel(tr("Presets:")));

    for (int i = 1; i <= 5; ++i) {
        QPushButton *btn = new QPushButton(tr("Layout %1").arg(i), m_mainWindow);
        connect(btn, &QPushButton::clicked, this, [this, i]() {
            emit layoutOperationRequested(QString("layout%1.xml").arg(i));
        });
        toolBar->addWidget(btn);
    }

    toolBar->addSeparator();

    // Add save/load buttons
    QPushButton *saveBtn = new QPushButton(tr("Save Current"), m_mainWindow);
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(m_mainWindow,
                                                        tr("Save Layout"), "", tr("Layout Files (*.xml)"));
        if (!fileName.isEmpty()) {
            emit layoutOperationRequested("save:" + fileName);
        }
    });

    QPushButton *loadBtn = new QPushButton(tr("Load Custom"), m_mainWindow);
    connect(loadBtn, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(m_mainWindow,
                                                        tr("Load Layout"), "", tr("Layout Files (*.xml)"));
        if (!fileName.isEmpty()) {
            emit layoutOperationRequested(fileName);
        }
    });

    toolBar->addWidget(saveBtn);
    toolBar->addWidget(loadBtn);
}

void MenuManager::setResizeEnabled(bool enabled)
{
    if (m_resizeEnabled != enabled) {
        m_resizeEnabled = enabled;
        m_resizeAction->setChecked(enabled);
        emit resizeEnabledChanged(enabled);
    }
}
