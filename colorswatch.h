#ifndef COLORSWATCH_H
#define COLORSWATCH_H

#include <QDockWidget>
#include <QAction>
#include <QActionGroup>
#include <QMenu>

class ColorDock;

class ColorSwatch : public QDockWidget
{
    Q_OBJECT

public:
    explicit ColorSwatch(const QString &colorName, QMainWindow *parent = nullptr,
                         Qt::WindowFlags flags = Qt::WindowFlags());
    ~ColorSwatch();

    void setFeatures(QDockWidget::DockWidgetFeatures features);
    QDockWidget::DockWidgetFeatures features() const;

    void setAllowedAreas(Qt::DockWidgetAreas areas);
    Qt::DockWidgetAreas allowedAreas() const;

    QString colorName() const { return m_colorName; }

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void changeClosable(bool on);
    void changeMovable(bool on);
    void changeFloatable(bool on);
    void changeFloating(bool on);

    void allowLeft(bool a);
    void allowRight(bool a);
    void allowTop(bool a);
    void allowBottom(bool a);

    void placeLeft(bool p);
    void placeRight(bool p);
    void placeTop(bool p);
    void placeBottom(bool p);

private:
    void setupActions();
    void setupMenu();
    void updateContextMenu();

    QString m_colorName;
    QMainWindow *m_mainWindow;
    ColorDock *m_colorDock;
    QMenu *m_menu;

    QAction *m_closableAction;
    QAction *m_movableAction;
    QAction *m_floatableAction;
    QAction *m_floatingAction;

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
};

class ColorDock : public QWidget
{
    Q_OBJECT
public:
    explicit ColorDock(const QString &color, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    const QString m_color;
};

#endif // COLORSWATCH_H
