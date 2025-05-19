#ifndef LAYOUTMANAGER_H
#define LAYOUTMANAGER_H

#include <QObject>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDockWidget>
#include <QMenu>
#include <QMainWindow>
#include "colorswatch.h"

class LayoutManager : public QObject
{
    Q_OBJECT

public:
    explicit LayoutManager(QMainWindow *parent = nullptr);
    ~LayoutManager();

    void saveLayoutToFile(const QString &fileName);
    void loadLayoutFromFile(const QString &fileName);
    void setResizeEnabled(bool enabled);

signals:
    void saveDockWidgetsLayoutRequested(QXmlStreamWriter &xmlWriter);
    void loadDockWidgetsLayoutRequested(QXmlStreamReader &xmlReader);
      void dockWidgetVisibilityChanged(const QString &dockWidgetName, bool visible);

private:
    void saveMainWindowState(QXmlStreamWriter &xmlWriter);
    void loadMainWindowState(QXmlStreamReader &xmlReader);
    void saveCentralWidgetState(QXmlStreamWriter &xmlWriter);
    void loadCentralWidgetState(QXmlStreamReader &xmlReader);
    void saveDockWidgetsLayout(QXmlStreamWriter &xmlWriter);
    void loadDockWidgetsLayout(QXmlStreamReader &xmlReader);
    void saveWidgetProperties(QXmlStreamWriter &xmlWriter, QWidget *widget);
    void loadWidgetProperties(QXmlStreamReader &xmlReader, QWidget *widget);
    ColorSwatch* createColorSwatch(const QString &colorName, Qt::DockWidgetArea area);
    void updateTabbedGroupSizes(ColorSwatch *swatch);
    void setupDockWidgets();

    QMainWindow *m_mainWindow;
    QList<ColorSwatch*> m_dockWidgets;
    QMap<QAction*, ColorSwatch*> m_actionToDockWidgetMap;
    QMap<ColorSwatch*, Qt::DockWidgetArea> m_dockWidgetAreas;
    bool m_resizeEnabled = false;

private slots:
    void toggleDockWidgetVisibility(bool checked);
    void handleDockLocationChanged(Qt::DockWidgetArea area);

};

#endif // LAYOUTMANAGER_H
