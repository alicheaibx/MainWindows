// layoutmanager.h
#ifndef LAYOUTMANAGER_H
#define LAYOUTMANAGER_H

#include <QObject>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMap>

class QMainWindow;
class QWidget;

class LayoutManager : public QObject
{
    Q_OBJECT

public:
    explicit LayoutManager(QMainWindow *parent = nullptr);
    void saveLayoutToFile(const QString &fileName);
    void loadLayoutFromFile(const QString &fileName);

    void registerWidget(const QString &name, QWidget *widget);
    void unregisterWidget(const QString &name);

signals:
    void saveWidgetsLayoutRequested(QXmlStreamWriter &xmlWriter);
    void loadWidgetsLayoutRequested(QXmlStreamReader &xmlReader);

private:
    void saveMainWindowState(QXmlStreamWriter &xmlWriter);
    void loadMainWindowState(QXmlStreamReader &xmlReader);
    void saveWidgetProperties(QXmlStreamWriter &xmlWriter, QWidget *widget);
    void loadWidgetProperties(QXmlStreamReader &xmlReader, QWidget *widget);

    QMainWindow *m_mainWindow;
    QMap<QString, QWidget*> m_registeredWidgets;
};

#endif // LAYOUTMANAGER_H
