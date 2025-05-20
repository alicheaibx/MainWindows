#ifndef DOCKMANAGER_H
#define DOCKMANAGER_H

#include <QObject>
#include <QDockWidget>
#include <QMap>
#include <QMenu>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMainWindow>
#include "colorswatch.h"

class DockManager : public QObject
{
    Q_OBJECT

public:
    explicit DockManager(QMainWindow *parent = nullptr);
    ~DockManager();

    void setupDockWidgets();
    QMenu* viewMenu() const { return m_viewMenu; }
    QList<ColorSwatch*> dockWidgets() const { return m_dockWidgets; }
    ColorSwatch* dockWidget(const QString &name) const;

    void saveDockWidgetSize(ColorSwatch *swatch);
    QSize savedDockWidgetSize(const QString &name) const;
    void setSizesFixed(bool fixed);

public slots:
    void saveDockWidgetsLayout(QXmlStreamWriter &xmlWriter);
    void loadDockWidgetsLayout(QXmlStreamReader &xmlReader);
    void applySavedSizes();
    void setDockWidgetFeatures(const QString &name, QDockWidget::DockWidgetFeatures features);
    void setDockWidgetAllowedAreas(const QString &name, Qt::DockWidgetAreas areas);
    void setDockWidgetFloating(const QString &name, bool floating);
    void setDockWidgetVisible(const QString &name, bool visible);
    void toggleDockWidget(const QString &name);

signals:
    void dockWidgetCreated(ColorSwatch *swatch);
    void dockWidgetFeaturesChanged(const QString &name, QDockWidget::DockWidgetFeatures features);
    void dockWidgetVisibilityChanged(const QString &name, bool visible);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void toggleDockWidgetVisibility(bool checked);
    void handleDockLocationChanged(Qt::DockWidgetArea area);

private:
    ColorSwatch* createColorSwatch(const QString &colorName, Qt::DockWidgetArea area);
    void setupDockWidgetProperties(ColorSwatch *swatch);
    void updateDockWidgetSizeConstraints(ColorSwatch *swatch);
    void updateTabbedGroupSizes(ColorSwatch *swatch);
    void handleDockWidgetResized(ColorSwatch *swatch);
    void saveWidgetProperties(QXmlStreamWriter &xmlWriter, QWidget *widget);
    void loadWidgetProperties(QXmlStreamReader &xmlReader, QWidget *widget);
    void applySavedSizesDelayed();  // Add this line
    bool m_sizesFixed = true;
    QMainWindow *m_mainWindow;
    QMenu *m_viewMenu;
    QList<ColorSwatch*> m_dockWidgets;
    QMap<QAction*, ColorSwatch*> m_actionToDockWidgetMap;
    QMap<ColorSwatch*, QSize> m_dockWidgetSizes;
    QMap<ColorSwatch*, Qt::DockWidgetArea> m_dockWidgetAreas;
    bool m_blockResizeUpdates = false;
};

#endif // DOCKMANAGER_H
