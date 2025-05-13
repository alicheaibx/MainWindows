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

    QMenu* viewMenu() const { return m_viewMenu; }
    QList<ColorSwatch*> dockWidgets() const { return m_dockWidgets; }
    ColorSwatch* dockWidget(const QString &name) const;

public slots:
    void saveDockWidgetsLayout(QXmlStreamWriter &xmlWriter);
    void loadDockWidgetsLayout(QXmlStreamReader &xmlReader);

signals:
    void dockWidgetCreated(ColorSwatch *swatch);
    void dockWidgetFeaturesChanged(const QString &name, QDockWidget::DockWidgetFeatures features);
    void dockWidgetVisibilityChanged(const QString &name, bool visible);

private slots:
    void toggleDockWidgetVisibility(bool checked);
    void handleDockLocationChanged(Qt::DockWidgetArea area);

private:
    ColorSwatch* createColorSwatch(const QString &colorName, Qt::DockWidgetArea area);
    void setupDockWidgets();
    void updateTabbedGroupSizes(ColorSwatch *swatch);
    void saveWidgetProperties(QXmlStreamWriter &xmlWriter, QWidget *widget);
    void loadWidgetProperties(QXmlStreamReader &xmlReader, QWidget *widget);

    QMainWindow *m_mainWindow;
    QMenu *m_viewMenu;
    QList<ColorSwatch*> m_dockWidgets;
    QMap<QAction*, ColorSwatch*> m_actionToDockWidgetMap;
    QMap<ColorSwatch*, Qt::DockWidgetArea> m_dockWidgetAreas;
};

#endif // DOCKMANAGER_H
