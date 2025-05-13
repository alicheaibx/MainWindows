#include "colorswatch.h"
#include <QPainter>
#include <QPainterPath>
#include <QMainWindow>
#include <QSignalBlocker>
#include <qevent.h>

QColor bgColorForName(const QString &name)
{
    static const QHash<QString, QColor> colors = {
        {"Black", QColor("#D8D8D8")},
        {"White", QColor("#F1F1F1")},
        {"Red", QColor("#F1D8D8")},
        {"Green", QColor("#D8E4D8")},
        {"Blue", QColor("#D8D8F1")},
        {"Yellow", QColor("#F1F0D8")}
    };
    return colors.value(name, QColor(name).lighter(110));
}

QColor fgColorForName(const QString &name)
{
    static const QHash<QString, QColor> colors = {
        {"Black", QColor("#6C6C6C")},
        {"White", QColor("#F8F8F8")},
        {"Red", QColor("#F86C6C")},
        {"Green", QColor("#6CB26C")},
        {"Blue", QColor("#6C6CF8")},
        {"Yellow", QColor("#F8F76C")}
    };
    return colors.value(name, QColor(name));
}

static void render_qt_text(QPainter *painter, int w, int h, const QColor &color)
{
    QFont font("Times", 10);
    font.setStyleStrategy(QFont::ForceOutline);
    painter->setFont(font);
    painter->setPen(color);

    QPainterPath path;
    path.addText(w/2 - 50, h/2, font, "Qt");
    painter->drawPath(path);
}

ColorDock::ColorDock(const QString &c, QWidget *parent)
    : QWidget(parent), m_color(c)
{
    QFont font = this->font();
    font.setPointSize(8);
    setFont(font);
}

void ColorDock::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), bgColorForName(m_color));
    render_qt_text(&p, width(), height(), fgColorForName(m_color));
}

ColorSwatch::ColorSwatch(const QString &colorName, QMainWindow *parent, Qt::WindowFlags flags)
    : QDockWidget(parent, flags), m_colorName(colorName), m_mainWindow(parent)
{
    setObjectName(colorName + " Dock Widget");
    setWindowTitle(objectName() + " [*]");

    m_colorDock = new ColorDock(colorName);
    setWidget(m_colorDock);

    setupActions();
    setupMenu();
}

ColorSwatch::~ColorSwatch()
{
    delete m_colorDock;
}

QDockWidget::DockWidgetFeatures ColorSwatch::features() const
{
    return QDockWidget::features();
}

Qt::DockWidgetAreas ColorSwatch::allowedAreas() const
{
    return QDockWidget::allowedAreas();
}

void ColorSwatch::setFeatures(QDockWidget::DockWidgetFeatures features)
{
    QDockWidget::setFeatures(features);
    updateContextMenu();
}

void ColorSwatch::setAllowedAreas(Qt::DockWidgetAreas areas)
{
    QDockWidget::setAllowedAreas(areas);
    updateContextMenu();
}

void ColorSwatch::contextMenuEvent(QContextMenuEvent *event)
{
    event->accept();
    m_menu->exec(event->globalPos());
}

void ColorSwatch::setupActions()
{
    m_closableAction = new QAction(tr("Closable"), this);
    m_closableAction->setCheckable(true);
    connect(m_closableAction, &QAction::triggered, this, &ColorSwatch::changeClosable);

    m_movableAction = new QAction(tr("Movable"), this);
    m_movableAction->setCheckable(true);
    connect(m_movableAction, &QAction::triggered, this, &ColorSwatch::changeMovable);

    m_floatableAction = new QAction(tr("Floatable"), this);
    m_floatableAction->setCheckable(true);
    connect(m_floatableAction, &QAction::triggered, this, &ColorSwatch::changeFloatable);

    m_floatingAction = new QAction(tr("Floating"), this);
    m_floatingAction->setCheckable(true);
    connect(m_floatingAction, &QAction::triggered, this, &ColorSwatch::changeFloating);

    m_allowedAreasActions = new QActionGroup(this);
    m_allowedAreasActions->setExclusive(false);

    m_allowLeftAction = new QAction(tr("Allow Left"), this);
    m_allowLeftAction->setCheckable(true);
    connect(m_allowLeftAction, &QAction::triggered, this, &ColorSwatch::allowLeft);

    m_allowRightAction = new QAction(tr("Allow Right"), this);
    m_allowRightAction->setCheckable(true);
    connect(m_allowRightAction, &QAction::triggered, this, &ColorSwatch::allowRight);

    m_allowTopAction = new QAction(tr("Allow Top"), this);
    m_allowTopAction->setCheckable(true);
    connect(m_allowTopAction, &QAction::triggered, this, &ColorSwatch::allowTop);

    m_allowBottomAction = new QAction(tr("Allow Bottom"), this);
    m_allowBottomAction->setCheckable(true);
    connect(m_allowBottomAction, &QAction::triggered, this, &ColorSwatch::allowBottom);

    m_allowedAreasActions->addAction(m_allowLeftAction);
    m_allowedAreasActions->addAction(m_allowRightAction);
    m_allowedAreasActions->addAction(m_allowTopAction);
    m_allowedAreasActions->addAction(m_allowBottomAction);

    m_areaActions = new QActionGroup(this);
    m_areaActions->setExclusive(true);

    m_leftAction = new QAction(tr("Place Left"), this);
    m_leftAction->setCheckable(true);
    connect(m_leftAction, &QAction::triggered, this, &ColorSwatch::placeLeft);

    m_rightAction = new QAction(tr("Place Right"), this);
    m_rightAction->setCheckable(true);
    connect(m_rightAction, &QAction::triggered, this, &ColorSwatch::placeRight);

    m_topAction = new QAction(tr("Place Top"), this);
    m_topAction->setCheckable(true);
    connect(m_topAction, &QAction::triggered, this, &ColorSwatch::placeTop);

    m_bottomAction = new QAction(tr("Place Bottom"), this);
    m_bottomAction->setCheckable(true);
    connect(m_bottomAction, &QAction::triggered, this, &ColorSwatch::placeBottom);

    m_areaActions->addAction(m_leftAction);
    m_areaActions->addAction(m_rightAction);
    m_areaActions->addAction(m_topAction);
    m_areaActions->addAction(m_bottomAction);

    connect(m_movableAction, &QAction::triggered, m_areaActions, &QActionGroup::setEnabled);
    connect(m_movableAction, &QAction::triggered, m_allowedAreasActions, &QActionGroup::setEnabled);
}

void ColorSwatch::setupMenu()
{
    m_menu = new QMenu(m_colorName, this);
    m_menu->addAction(toggleViewAction());
    m_menu->addAction(tr("Raise"), this, &QWidget::raise);
    m_menu->addSeparator();
    m_menu->addAction(m_closableAction);
    m_menu->addAction(m_movableAction);
    m_menu->addAction(m_floatableAction);
    m_menu->addAction(m_floatingAction);
    m_menu->addSeparator();
    m_menu->addActions(m_allowedAreasActions->actions());
    m_menu->addSeparator();
    m_menu->addActions(m_areaActions->actions());

    connect(m_menu, &QMenu::aboutToShow, this, &ColorSwatch::updateContextMenu);
}

void ColorSwatch::updateContextMenu()
{
    const Qt::DockWidgetArea area = m_mainWindow->dockWidgetArea(this);
    const Qt::DockWidgetAreas areas = allowedAreas();

    m_closableAction->setChecked(features() & QDockWidget::DockWidgetClosable);
    m_floatableAction->setChecked(features() & QDockWidget::DockWidgetFloatable);
    m_floatingAction->setChecked(isFloating());
    m_movableAction->setChecked(features() & QDockWidget::DockWidgetMovable);

    m_allowLeftAction->setChecked(areas & Qt::LeftDockWidgetArea);
    m_allowRightAction->setChecked(areas & Qt::RightDockWidgetArea);
    m_allowTopAction->setChecked(areas & Qt::TopDockWidgetArea);
    m_allowBottomAction->setChecked(areas & Qt::BottomDockWidgetArea);

    {
        const QSignalBlocker blocker(m_leftAction);
        m_leftAction->setChecked(area == Qt::LeftDockWidgetArea);
    }
    {
        const QSignalBlocker blocker(m_rightAction);
        m_rightAction->setChecked(area == Qt::RightDockWidgetArea);
    }
    {
        const QSignalBlocker blocker(m_topAction);
        m_topAction->setChecked(area == Qt::TopDockWidgetArea);
    }
    {
        const QSignalBlocker blocker(m_bottomAction);
        m_bottomAction->setChecked(area == Qt::BottomDockWidgetArea);
    }

    m_leftAction->setEnabled(areas & Qt::LeftDockWidgetArea);
    m_rightAction->setEnabled(areas & Qt::RightDockWidgetArea);
    m_topAction->setEnabled(areas & Qt::TopDockWidgetArea);
    m_bottomAction->setEnabled(areas & Qt::BottomDockWidgetArea);
}

void ColorSwatch::changeClosable(bool on)
{
    setFeatures(on ? features() | QDockWidget::DockWidgetClosable
                   : features() & ~QDockWidget::DockWidgetClosable);
}

void ColorSwatch::changeMovable(bool on)
{
    setFeatures(on ? features() | QDockWidget::DockWidgetMovable
                   : features() & ~QDockWidget::DockWidgetMovable);
}

void ColorSwatch::changeFloatable(bool on)
{
    setFeatures(on ? features() | QDockWidget::DockWidgetFloatable
                   : features() & ~QDockWidget::DockWidgetFloatable);
}

void ColorSwatch::changeFloating(bool floating)
{
    setFloating(floating);
}

void ColorSwatch::allowLeft(bool a) { setAllowedAreas(a ? allowedAreas() | Qt::LeftDockWidgetArea
                      : allowedAreas() & ~Qt::LeftDockWidgetArea); }
void ColorSwatch::allowRight(bool a) { setAllowedAreas(a ? allowedAreas() | Qt::RightDockWidgetArea
                      : allowedAreas() & ~Qt::RightDockWidgetArea); }
void ColorSwatch::allowTop(bool a) { setAllowedAreas(a ? allowedAreas() | Qt::TopDockWidgetArea
                      : allowedAreas() & ~Qt::TopDockWidgetArea); }
void ColorSwatch::allowBottom(bool a) { setAllowedAreas(a ? allowedAreas() | Qt::BottomDockWidgetArea
                      : allowedAreas() & ~Qt::BottomDockWidgetArea); }

void ColorSwatch::placeLeft(bool p) { if (p) m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, this); }
void ColorSwatch::placeRight(bool p) { if (p) m_mainWindow->addDockWidget(Qt::RightDockWidgetArea, this); }
void ColorSwatch::placeTop(bool p) { if (p) m_mainWindow->addDockWidget(Qt::TopDockWidgetArea, this); }
void ColorSwatch::placeBottom(bool p) { if (p) m_mainWindow->addDockWidget(Qt::BottomDockWidgetArea, this); }
