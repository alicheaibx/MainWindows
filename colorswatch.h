#ifndef COLORSWATCH_H
#define COLORSWATCH_H

#include <QDockWidget>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QFrame>
#include <QDebug>

class ColorDock;
class BlueTitleBar;

class ColorSwatch : public QDockWidget
{
    Q_OBJECT

public:
    explicit ColorSwatch(const QString &colorName, QMainWindow *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~ColorSwatch();

    // Property accessors
    void setCustomSizeHint(const QSize &size);
    QSize customSizeHint() const;

    void setFeatures(QDockWidget::DockWidgetFeatures features);
    QDockWidget::DockWidgetFeatures features() const;

    void setAllowedAreas(Qt::DockWidgetAreas areas);
    Qt::DockWidgetAreas allowedAreas() const;

    void setTitleBarWidget(QWidget *widget);
    QWidget *titleBarWidget() const;

    QMenu *contextMenu() const { return m_menu; }
    QString colorName() const { return m_colorName; }

    // Utility methods
    bool isAreaAllowed(Qt::DockWidgetArea area) const;
    void updateContextMenu();

public slots:
    void changeSizeHints();

protected:
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif
    void resizeEvent(QResizeEvent *e) override;

private slots:
    // Feature control
    void changeClosable(bool on);
    void changeMovable(bool on);
    void changeFloatable(bool on);
    void changeFloating(bool on);
    void changeVerticalTitleBar(bool on);

    // Area control
    void allowLeft(bool a);
    void allowRight(bool a);
    void allowTop(bool a);
    void allowBottom(bool a);

    void placeLeft(bool p);
    void placeRight(bool p);
    void placeTop(bool p);
    void placeBottom(bool p);

    // Docking operations
    void splitInto(QAction *action);
    void tabInto(QAction *action);

private:
    void allow(Qt::DockWidgetArea area, bool allow);
    void place(Qt::DockWidgetArea area, bool place);
    void setupActions();
    void setupMenus();

    QString m_colorName;
    QMainWindow *m_mainWindow;
    ColorDock *m_colorDock;

    // Actions
    QAction *m_closableAction;
    QAction *m_movableAction;
    QAction *m_floatableAction;
    QAction *m_floatingAction;
    QAction *m_verticalTitleBarAction;

    QActionGroup *m_allowedAreasActions;
    QAction *m_allowLeftAction;
    QAction *m_allowRightAction;
    QAction *m_allowTopAction;
    QAction *m_allowBottomAction;

    QActionGroup *m_areaActions;
    QAction *m_leftAction;
    QAction *m_rightAction;
    QAction *m_topAction;
    QAction *m_bottomAction;

    // Menus
    QMenu *m_tabMenu;
    QMenu *m_splitHMenu;
    QMenu *m_splitVMenu;
    QMenu *m_menu;
};

class BlueTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit BlueTitleBar(QWidget *parent = nullptr);

    QSize sizeHint() const override { return minimumSizeHint(); }
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public slots:
    void updateMask();

private:
    const QPixmap m_leftPm;
    const QPixmap m_centerPm;
    const QPixmap m_rightPm;
};

class ColorDock : public QFrame
{
    Q_OBJECT
public:
    explicit ColorDock(const QString &color, QWidget *parent = nullptr);

    QSize sizeHint() const override { return m_szHint; }
    QSize minimumSizeHint() const override { return m_minSzHint; }

    void setCustomSizeHint(const QSize &size);
    QSize customSizeHint() const { return m_szHint; }

public slots:
    void changeSizeHints();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    const QString m_color;
    QSize m_szHint;
    QSize m_minSzHint;
};

#endif // COLORSWATCH_H
