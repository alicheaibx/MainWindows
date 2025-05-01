#include "layoutmanager.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMainWindow>
#include <QMessageBox>
#include <QFile>
#include <QTextEdit>

LayoutManager::LayoutManager(QMainWindow *parent)
    : QObject(parent), m_mainWindow(parent)
{
}

void LayoutManager::saveLayoutToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(m_mainWindow, tr("Error"), tr("Failed to open %1 for writing").arg(fileName));
        return;
    }

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("MainWindowLayout");

    saveMainWindowGeometry(xmlWriter);
    saveCentralWidgetProperties(xmlWriter);
    emit saveDockWidgetsLayoutRequested(xmlWriter);

    xmlWriter.writeEndElement(); // MainWindowLayout
    xmlWriter.writeEndDocument();
    file.close();
}

void LayoutManager::loadLayoutFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(m_mainWindow, tr("Error"), tr("Failed to open %1 for reading").arg(fileName));
        return;
    }

    QXmlStreamReader xmlReader(&file);
    while (!xmlReader.atEnd() && !xmlReader.hasError()) {
        xmlReader.readNext();
        if (xmlReader.isStartElement() && xmlReader.name() == "MainWindowLayout") {
            while (xmlReader.readNextStartElement()) {
                if (xmlReader.name() == "MainWindowGeometry")
                    loadMainWindowGeometry(xmlReader);
                else if (xmlReader.name() == "CentralWidget")
                    loadCentralWidgetProperties(xmlReader);
                else if (xmlReader.name() == "DockWidgets")
                    emit loadDockWidgetsLayoutRequested(xmlReader);
                else
                    xmlReader.skipCurrentElement();
            }
        }
    }

    if (xmlReader.hasError()) {
        QMessageBox::warning(m_mainWindow, tr("Error"), tr("Failed to parse XML file: %1").arg(xmlReader.errorString()));
    }

    file.close();
}

void LayoutManager::saveMainWindowGeometry(QXmlStreamWriter &xmlWriter)
{
    xmlWriter.writeStartElement("MainWindowGeometry");
    QRect geometry = m_mainWindow->geometry();
    xmlWriter.writeTextElement("x", QString::number(geometry.x()));
    xmlWriter.writeTextElement("y", QString::number(geometry.y()));
    xmlWriter.writeTextElement("width", QString::number(geometry.width()));
    xmlWriter.writeTextElement("height", QString::number(geometry.height()));
    xmlWriter.writeTextElement("NestedDocking", m_mainWindow->isDockNestingEnabled() ? "true" : "false");
    xmlWriter.writeTextElement("GroupMovement", (m_mainWindow->dockOptions() & QMainWindow::AllowNestedDocks) ? "true" : "false");
    xmlWriter.writeEndElement(); // MainWindowGeometry
}

void LayoutManager::loadMainWindowGeometry(QXmlStreamReader &xmlReader)
{
    int x = 0, y = 0, width = 800, height = 600;
    bool nestedDocking = false;
    bool groupMovement = false;

    while (xmlReader.readNextStartElement()) {
        if (xmlReader.name() == "x")
            x = xmlReader.readElementText().toInt();
        else if (xmlReader.name() == "y")
            y = xmlReader.readElementText().toInt();
        else if (xmlReader.name() == "width")
            width = xmlReader.readElementText().toInt();
        else if (xmlReader.name() == "height")
            height = xmlReader.readElementText().toInt();
        else if (xmlReader.name() == "NestedDocking")
            nestedDocking = (xmlReader.readElementText() == "true");
        else if (xmlReader.name() == "GroupMovement")
            groupMovement = (xmlReader.readElementText() == "true");
        else
            xmlReader.skipCurrentElement();
    }

    m_mainWindow->setGeometry(x, y, width, height);
    m_mainWindow->setDockNestingEnabled(nestedDocking);

    QMainWindow::DockOptions options = m_mainWindow->dockOptions();
    if (groupMovement) {
        options |= QMainWindow::AllowNestedDocks;
    } else {
        options &= ~QMainWindow::AllowNestedDocks;
    }
    m_mainWindow->setDockOptions(options);
}

void LayoutManager::saveCentralWidgetProperties(QXmlStreamWriter &xmlWriter)
{
    QWidget *central = m_mainWindow->centralWidget();
    if (!central) return;

    xmlWriter.writeStartElement("CentralWidget");

    xmlWriter.writeTextElement("ObjectName", central->objectName());
    xmlWriter.writeTextElement("Geometry", QString("%1,%2,%3,%4")
                                               .arg(central->geometry().x())
                                               .arg(central->geometry().y())
                                               .arg(central->geometry().width())
                                               .arg(central->geometry().height()));
    xmlWriter.writeTextElement("MinimumSize", QString("%1,%2")
                                                  .arg(central->minimumSize().width())
                                                  .arg(central->minimumSize().height()));
    xmlWriter.writeTextElement("MaximumSize", QString("%1,%2")
                                                  .arg(central->maximumSize().width())
                                                  .arg(central->maximumSize().height()));

    if (QTextEdit *textEdit = qobject_cast<QTextEdit*>(central)) {
        xmlWriter.writeTextElement("Text", textEdit->toPlainText());
        xmlWriter.writeTextElement("ReadOnly", textEdit->isReadOnly() ? "true" : "false");
    }

    xmlWriter.writeEndElement(); // CentralWidget
}

void LayoutManager::loadCentralWidgetProperties(QXmlStreamReader &xmlReader)
{
    QWidget *central = m_mainWindow->centralWidget();
    if (!central) return;

    QString objectName;
    QRect geometry;
    QSize minSize, maxSize;
    QString text;
    bool readOnly = false;

    while (xmlReader.readNextStartElement()) {
        if (xmlReader.name() == "ObjectName") {
            objectName = xmlReader.readElementText();
        } else if (xmlReader.name() == "Geometry") {
            QStringList geo = xmlReader.readElementText().split(',');
            if (geo.size() == 4) {
                geometry = QRect(geo[0].toInt(), geo[1].toInt(),
                                 geo[2].toInt(), geo[3].toInt());
            }
        } else if (xmlReader.name() == "MinimumSize") {
            QStringList size = xmlReader.readElementText().split(',');
            if (size.size() == 2) {
                minSize = QSize(size[0].toInt(), size[1].toInt());
            }
        } else if (xmlReader.name() == "MaximumSize") {
            QStringList size = xmlReader.readElementText().split(',');
            if (size.size() == 2) {
                maxSize = QSize(size[0].toInt(), size[1].toInt());
            }
        } else if (xmlReader.name() == "Text") {
            text = xmlReader.readElementText();
        } else if (xmlReader.name() == "ReadOnly") {
            readOnly = xmlReader.readElementText() == "true";
        } else {
            xmlReader.skipCurrentElement();
        }
    }

    central->setObjectName(objectName);
    if (!geometry.isNull()) central->setGeometry(geometry);
    if (!minSize.isNull()) central->setMinimumSize(minSize);
    if (!maxSize.isNull()) central->setMaximumSize(maxSize);

    if (QTextEdit *textEdit = qobject_cast<QTextEdit*>(central)) {
        textEdit->setPlainText(text);
        textEdit->setReadOnly(readOnly);
    }
}
