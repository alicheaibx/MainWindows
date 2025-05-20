#ifndef LAYOUTMANAGER_H
#define LAYOUTMANAGER_H

#include <QObject>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

class QMainWindow;

class LayoutManager : public QObject
{
    Q_OBJECT

public:
    explicit LayoutManager(QMainWindow *parent = nullptr);
    void saveLayoutToFile(const QString &fileName);
    void loadLayoutFromFile(const QString &fileName);

signals:
    void saveDockWidgetsLayoutRequested(QXmlStreamWriter &xmlWriter);
    void loadDockWidgetsLayoutRequested(QXmlStreamReader &xmlReader);

private:
    void saveMainWindowGeometry(QXmlStreamWriter &xmlWriter);
    void loadMainWindowGeometry(QXmlStreamReader &xmlReader);
    void saveCentralWidgetProperties(QXmlStreamWriter &xmlWriter);
    void loadCentralWidgetProperties(QXmlStreamReader &xmlReader);

    QMainWindow *m_mainWindow;
};

#endif // LAYOUTMANAGER_H
