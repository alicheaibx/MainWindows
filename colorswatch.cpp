#include "colorswatch.h"
#include <QPainter>
#include <QPainterPath>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QSpinBox>
#include <QLabel>
#include <QSignalBlocker>
#include <QPushButton>
#include <QMainWindow>
#include <QDebug>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QBitmap>
#include <QFrame>

QColor bgColorForName(const QString &name)
{
    if (name == "Black") return QColor("#D8D8D8");
    if (name == "White") return QColor("#F1F1F1");
    if (name == "Red") return QColor("#F1D8D8");
    if (name == "Green") return QColor("#D8E4D8");
    if (name == "Blue") return QColor("#D8D8F1");
    if (name == "Yellow") return QColor("#F1F0D8");
    return QColor(name).lighter(110);
}

QColor fgColorForName(const QString &name)
{
    if (name == "Black") return QColor("#6C6C6C");
    if (name == "White") return QColor("#F8F8F8");
    if (name == "Red") return QColor("#F86C6C");
    if (name == "Green") return QColor("#6CB26C");
    if (name == "Blue") return QColor("#6C6CF8");
    if (name == "Yellow") return QColor("#F8F76C");
    return QColor(name);
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
    : QFrame(parent), m_color(c)
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
    static_cast<QFrame*>(m_colorDock)->setFrameStyle(QFrame::Box | QFrame::Sunken);
    QDockWidget::setWidget(m_colorDock);

    setupActions();
    setupMenus();

    if (colorName == "Black") {
        m_leftAction->setShortcut(Qt::CTRL | Qt::Key_W);
        m_rightAction->setShortcut(Qt::CTRL | Qt::Key_E);
        toggleViewAction()->setShortcut(Qt::CTRL | Qt::Key_R);
    }
}

ColorSwatch::~ColorSwatch()
{
    delete m_colorDock;
}

void ColorSwatch::setFeatures(QDockWidget::DockWidgetFeatures features)
{
    QDockWidget::setFeatures(features);
    updateContextMenu();
}

QDockWidget::DockWidgetFeatures ColorSwatch::features() const
{
    return QDockWidget::features();
}

void ColorSwatch::setAllowedAreas(Qt::DockWidgetAreas areas)
{
    QDockWidget::setAllowedAreas(areas);
    updateContextMenu();
}

Qt::DockWidgetAreas ColorSwatch::allowedAreas() const
{
    return QDockWidget::allowedAreas();
}

bool ColorSwatch::isAreaAllowed(Qt::DockWidgetArea area) const
{
    return allowedAreas() & area;
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

    m_verticalTitleBarAction = new QAction(tr("Vertical title bar"), this);
    m_verticalTitleBarAction->setCheckable(true);
    connect(m_verticalTitleBarAction, &QAction::triggered, this, &ColorSwatch::changeVerticalTitleBar);

    m_allowedAreasActions = new QActionGroup(this);
    m_allowedAreasActions->setExclusive(false);

    m_allowLeftAction = new QAction(tr("Allow on Left"), this);
    m_allowLeftAction->setCheckable(true);
    connect(m_allowLeftAction, &QAction::triggered, this, &ColorSwatch::allowLeft);

    m_allowRightAction = new QAction(tr("Allow on Right"), this);
    m_allowRightAction->setCheckable(true);
    connect(m_allowRightAction, &QAction::triggered, this, &ColorSwatch::allowRight);

    m_allowTopAction = new QAction(tr("Allow on Top"), this);
    m_allowTopAction->setCheckable(true);
    connect(m_allowTopAction, &QAction::triggered, this, &ColorSwatch::allowTop);

    m_allowBottomAction = new QAction(tr("Allow on Bottom"), this);
    m_allowBottomAction->setCheckable(true);
    connect(m_allowBottomAction, &QAction::triggered, this, &ColorSwatch::allowBottom);

    m_allowedAreasActions->addAction(m_allowLeftAction);
    m_allowedAreasActions->addAction(m_allowRightAction);
    m_allowedAreasActions->addAction(m_allowTopAction);
    m_allowedAreasActions->addAction(m_allowBottomAction);

    m_areaActions = new QActionGroup(this);
    m_areaActions->setExclusive(true);

    m_leftAction = new QAction(tr("Place on Left"), this);
    m_leftAction->setCheckable(true);
    connect(m_leftAction, &QAction::triggered, this, &ColorSwatch::placeLeft);

    m_rightAction = new QAction(tr("Place on Right"), this);
    m_rightAction->setCheckable(true);
    connect(m_rightAction, &QAction::triggered, this, &ColorSwatch::placeRight);

    m_topAction = new QAction(tr("Place on Top"), this);
    m_topAction->setCheckable(true);
    connect(m_topAction, &QAction::triggered, this, &ColorSwatch::placeTop);

    m_bottomAction = new QAction(tr("Place on Bottom"), this);
    m_bottomAction->setCheckable(true);
    connect(m_bottomAction, &QAction::triggered, this, &ColorSwatch::placeBottom);

    m_areaActions->addAction(m_leftAction);
    m_areaActions->addAction(m_rightAction);
    m_areaActions->addAction(m_topAction);
    m_areaActions->addAction(m_bottomAction);

    connect(m_movableAction, &QAction::triggered, m_areaActions, &QActionGroup::setEnabled);
    connect(m_movableAction, &QAction::triggered, m_allowedAreasActions, &QActionGroup::setEnabled);
    connect(m_floatableAction, &QAction::triggered, m_floatingAction, &QAction::setEnabled);
    connect(m_floatingAction, &QAction::triggered, m_floatableAction, &QAction::setDisabled);
    connect(m_movableAction, &QAction::triggered, m_floatableAction, &QAction::setEnabled);
}

void ColorSwatch::setupMenus()
{
    m_tabMenu = new QMenu(tr("Tab into"), this);
    connect(m_tabMenu, &QMenu::triggered, this, &ColorSwatch::tabInto);

    m_splitHMenu = new QMenu(tr("Split horizontally into"), this);
    connect(m_splitHMenu, &QMenu::triggered, this, &ColorSwatch::splitInto);

    m_splitVMenu = new QMenu(tr("Split vertically into"), this);
    connect(m_splitVMenu, &QMenu::triggered, this, &ColorSwatch::splitInto);

    QAction *windowModifiedAction = new QAction(tr("Modified"), this);
    windowModifiedAction->setCheckable(true);
    windowModifiedAction->setChecked(false);
    connect(windowModifiedAction, &QAction::toggled, this, &QWidget::setWindowModified);

    m_menu = new QMenu(m_colorName, this);
    m_menu->addAction(toggleViewAction());
    m_menu->addAction(tr("Raise"), this, &QWidget::raise);
    m_menu->addSeparator();
    m_menu->addAction(m_closableAction);
    m_menu->addAction(m_movableAction);
    m_menu->addAction(m_floatableAction);
    m_menu->addAction(m_floatingAction);
    m_menu->addAction(m_verticalTitleBarAction);
    m_menu->addSeparator();
    m_menu->addActions(m_allowedAreasActions->actions());
    m_menu->addSeparator();
    m_menu->addActions(m_areaActions->actions());
    m_menu->addSeparator();
    m_menu->addMenu(m_splitHMenu);
    m_menu->addMenu(m_splitVMenu);
    m_menu->addMenu(m_tabMenu);
    m_menu->addSeparator();
    m_menu->addAction(windowModifiedAction);

    connect(m_menu, &QMenu::aboutToShow, this, &ColorSwatch::updateContextMenu);
}

void ColorSwatch::updateContextMenu()
{
    const Qt::DockWidgetArea area = m_mainWindow->dockWidgetArea(this);
    const Qt::DockWidgetAreas areas = allowedAreas();

    m_closableAction->setChecked(features() & QDockWidget::DockWidgetClosable);
    if (windowType() == Qt::Drawer) {
        m_floatableAction->setEnabled(false);
        m_floatingAction->setEnabled(false);
        m_movableAction->setEnabled(false);
        m_verticalTitleBarAction->setChecked(false);
    } else {
        m_floatableAction->setChecked(features() & QDockWidget::DockWidgetFloatable);
        m_floatingAction->setChecked(isFloating());
        m_movableAction->setChecked(features() & QDockWidget::DockWidgetMovable);
        m_verticalTitleBarAction->setChecked(features() & QDockWidget::DockWidgetVerticalTitleBar);
    }

    m_allowLeftAction->setChecked(isAreaAllowed(Qt::LeftDockWidgetArea));
    m_allowRightAction->setChecked(isAreaAllowed(Qt::RightDockWidgetArea));
    m_allowTopAction->setChecked(isAreaAllowed(Qt::TopDockWidgetArea));
    m_allowBottomAction->setChecked(isAreaAllowed(Qt::BottomDockWidgetArea));

    if (m_allowedAreasActions->isEnabled()) {
        m_allowLeftAction->setEnabled(area != Qt::LeftDockWidgetArea);
        m_allowRightAction->setEnabled(area != Qt::RightDockWidgetArea);
        m_allowTopAction->setEnabled(area != Qt::TopDockWidgetArea);
        m_allowBottomAction->setEnabled(area != Qt::BottomDockWidgetArea);
    }

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

    if (m_areaActions->isEnabled()) {
        m_leftAction->setEnabled(areas & Qt::LeftDockWidgetArea);
        m_rightAction->setEnabled(areas & Qt::RightDockWidgetArea);
        m_topAction->setEnabled(areas & Qt::TopDockWidgetArea);
        m_bottomAction->setEnabled(areas & Qt::BottomDockWidgetArea);
    }

    m_tabMenu->clear();
    m_splitHMenu->clear();
    m_splitVMenu->clear();
    const QList<ColorSwatch *> dockList = m_mainWindow->findChildren<ColorSwatch*>();
    for (const ColorSwatch *dock : dockList) {
        if (dock != this) {
            m_tabMenu->addAction(dock->objectName());
            m_splitHMenu->addAction(dock->objectName());
            m_splitVMenu->addAction(dock->objectName());
        }
    }
}

static ColorSwatch *findByName(const QMainWindow *mainWindow, const QString &name)
{
    const QList<ColorSwatch *> dockList = mainWindow->findChildren<ColorSwatch*>();
    for (ColorSwatch *dock : dockList) {
        if (name == dock->objectName())
            return dock;
    }
    return nullptr;
}

void ColorSwatch::splitInto(QAction *action)
{
    ColorSwatch *target = findByName(m_mainWindow, action->text());
    if (!target)
        return;

    const Qt::Orientation o = action->parent() == m_splitHMenu
                                  ? Qt::Horizontal : Qt::Vertical;
    m_mainWindow->splitDockWidget(target, this, o);
}

void ColorSwatch::tabInto(QAction *action)
{
    if (ColorSwatch *target = findByName(m_mainWindow, action->text()))
        m_mainWindow->tabifyDockWidget(target, this);
}

#ifndef QT_NO_CONTEXTMENU
void ColorSwatch::contextMenuEvent(QContextMenuEvent *event)
{
    event->accept();
    m_menu->exec(event->globalPos());
}
#endif

void ColorSwatch::resizeEvent(QResizeEvent *e)
{
    QDockWidget::resizeEvent(e);
}

void ColorSwatch::allow(Qt::DockWidgetArea area, bool a)
{
    Qt::DockWidgetAreas areas = allowedAreas();
    areas = a ? areas | area : areas & ~area;
    setAllowedAreas(areas);

    if (m_areaActions->isEnabled()) {
        m_leftAction->setEnabled(areas & Qt::LeftDockWidgetArea);
        m_rightAction->setEnabled(areas & Qt::RightDockWidgetArea);
        m_topAction->setEnabled(areas & Qt::TopDockWidgetArea);
        m_bottomAction->setEnabled(areas & Qt::BottomDockWidgetArea);
    }
}

void ColorSwatch::place(Qt::DockWidgetArea area, bool p)
{
    if (!p)
        return;

    m_mainWindow->addDockWidget(area, this);

    if (m_allowedAreasActions->isEnabled()) {
        m_allowLeftAction->setEnabled(area != Qt::LeftDockWidgetArea);
        m_allowRightAction->setEnabled(area != Qt::RightDockWidgetArea);
        m_allowTopAction->setEnabled(area != Qt::TopDockWidgetArea);
        m_allowBottomAction->setEnabled(area != Qt::BottomDockWidgetArea);
    }
}

void ColorSwatch::changeClosable(bool on)
{
    setFeatures(on ? features() | QDockWidget::DockWidgetClosable : features() & ~QDockWidget::DockWidgetClosable);
}

void ColorSwatch::changeMovable(bool on)
{
    setFeatures(on ? features() | QDockWidget::DockWidgetMovable : features() & ~QDockWidget::DockWidgetMovable);
}

void ColorSwatch::changeFloatable(bool on)
{
    setFeatures(on ? features() | QDockWidget::DockWidgetFloatable : features() & ~QDockWidget::DockWidgetFloatable);
}

void ColorSwatch::changeFloating(bool floating)
{
    setFloating(floating);
}

void ColorSwatch::allowLeft(bool a)
{
    allow(Qt::LeftDockWidgetArea, a);
}

void ColorSwatch::allowRight(bool a)
{
    allow(Qt::RightDockWidgetArea, a);
}

void ColorSwatch::allowTop(bool a)
{
    allow(Qt::TopDockWidgetArea, a);
}

void ColorSwatch::allowBottom(bool a)
{
    allow(Qt::BottomDockWidgetArea, a);
}

void ColorSwatch::placeLeft(bool p)
{
    place(Qt::LeftDockWidgetArea, p);
}

void ColorSwatch::placeRight(bool p)
{
    place(Qt::RightDockWidgetArea, p);
}

void ColorSwatch::placeTop(bool p)
{
    place(Qt::TopDockWidgetArea, p);
}

void ColorSwatch::placeBottom(bool p)
{
    place(Qt::BottomDockWidgetArea, p);
}

void ColorSwatch::changeVerticalTitleBar(bool on)
{
    setFeatures(on ? features() | QDockWidget::DockWidgetVerticalTitleBar
                   : features() & ~QDockWidget::DockWidgetVerticalTitleBar);
}

BlueTitleBar::BlueTitleBar(QWidget *parent)
    : QWidget(parent),
    m_leftPm(QPixmap(":/res/titlebarLeft.png")),
    m_centerPm(QPixmap(":/res/titlebarCenter.png")),
    m_rightPm(QPixmap(":/res/titlebarRight.png"))
{
}

QSize BlueTitleBar::minimumSizeHint() const
{
    QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
    if (!dw) return QSize();

    QSize result(m_leftPm.width() + m_rightPm.width(), m_centerPm.height());
    if (dw->features() & QDockWidget::DockWidgetVerticalTitleBar)
        result.transpose();
    return result;
}

void BlueTitleBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QRect rect = this->rect();

    QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
    if (!dw) return;

    if (dw->features() & QDockWidget::DockWidgetVerticalTitleBar) {
        QSize s = rect.size();
        s.transpose();
        rect.setSize(s);

        painter.translate(rect.left(), rect.top() + rect.width());
        painter.rotate(-90);
        painter.translate(-rect.left(), -rect.top());
    }

    painter.drawPixmap(rect.topLeft(), m_leftPm);
    painter.drawPixmap(rect.topRight() - QPoint(m_rightPm.width() - 1, 0), m_rightPm);
    QBrush brush(m_centerPm);
    painter.fillRect(rect.left() + m_leftPm.width(), rect.top(),
                     rect.width() - m_leftPm.width() - m_rightPm.width(),
                     m_centerPm.height(), m_centerPm);
}

void BlueTitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    QRect rect = this->rect();

    QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
    if (!dw) return;

    if (dw->features() & QDockWidget::DockWidgetVerticalTitleBar) {
        QPoint p = pos;
        pos.setX(rect.left() + rect.bottom() - p.y());
        pos.setY(rect.top() + p.x() - rect.left());

        QSize s = rect.size();
        s.transpose();
        rect.setSize(s);
    }

    const int buttonRight = 7;
    const int buttonWidth = 20;
    int right = rect.right() - pos.x();
    int button = (right - buttonRight)/buttonWidth;
    switch (button) {
    case 0:
        event->accept();
        dw->close();
        break;
    case 1:
        event->accept();
        dw->setFloating(!dw->isFloating());
        break;
    case 2: {
        event->accept();
        QDockWidget::DockWidgetFeatures features = dw->features();
        if (features & QDockWidget::DockWidgetVerticalTitleBar)
            features &= ~QDockWidget::DockWidgetVerticalTitleBar;
        else
            features |= QDockWidget::DockWidgetVerticalTitleBar;
        dw->setFeatures(features);
        break;
    }
    default:
        event->ignore();
        break;
    }
}

void BlueTitleBar::updateMask()
{
    QDockWidget *dw = qobject_cast<QDockWidget*>(parent());
    if (!dw) return;

    QRect rect = dw->rect();
    QPixmap bitmap(dw->size());

    {
        QPainter painter(&bitmap);
        painter.fillRect(rect, Qt::color0);

        QRect contents = rect;
        contents.setTopLeft(geometry().bottomLeft());
        contents.setRight(geometry().right());
        contents.setBottom(contents.bottom()-y());
        painter.fillRect(contents, Qt::color1);

        QRect titleRect = this->geometry();

        if (dw->features() & QDockWidget::DockWidgetVerticalTitleBar) {
            QSize s = rect.size();
            s.transpose();
            rect.setSize(s);

            QSize s2 = size();
            s2.transpose();
            titleRect.setSize(s2);

            painter.translate(rect.left(), rect.top() + rect.width());
            painter.rotate(-90);
            painter.translate(-rect.left(), -rect.top());
        }

        contents.setTopLeft(titleRect.bottomLeft());
        contents.setRight(titleRect.right());
        contents.setBottom(rect.bottom()-y());

        rect = titleRect;

        painter.drawPixmap(rect.topLeft(), m_leftPm.mask());
        painter.fillRect(rect.left() + m_leftPm.width(), rect.top(),
                         rect.width() - m_leftPm.width() - m_rightPm.width(),
                         m_centerPm.height(), Qt::color1);
        painter.drawPixmap(rect.topRight() - QPoint(m_rightPm.width() - 1, 0), m_rightPm.mask());
        painter.fillRect(contents, Qt::color1);
    }

    dw->setMask(bitmap);
}
