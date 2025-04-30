#include "layoutmanager.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMainWindow>
#include <QMessageBox>
#include <QFile>

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

    // Save dock widgets layout (delegated to DockManager)
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
