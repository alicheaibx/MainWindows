#include "layoutmanager.h"
#include <QFile>
#include <QMessageBox>
#include <QTextEdit>
#include <qmainwindow.h>

LayoutManager::LayoutManager(QMainWindow *parent)
    : QObject(parent), m_mainWindow(parent)
{
}

void LayoutManager::saveLayoutToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(m_mainWindow, tr("Error"),
                             tr("Failed to open %1 for writing").arg(fileName));
        return;
    }

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("MainWindowLayout");

    saveMainWindowState(xmlWriter);
    saveCentralWidgetState(xmlWriter);
    emit saveDockWidgetsLayoutRequested(xmlWriter);

    xmlWriter.writeEndElement(); // MainWindowLayout
    xmlWriter.writeEndDocument();
    file.close();
}

void LayoutManager::loadLayoutFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(m_mainWindow, tr("Error"),
                             tr("Failed to open %1 for reading").arg(fileName));
        return;
    }

    QXmlStreamReader xmlReader(&file);
    while (!xmlReader.atEnd() && !xmlReader.hasError()) {
        xmlReader.readNext();
        if (xmlReader.isStartElement() && xmlReader.name() == "MainWindowLayout") {
            while (xmlReader.readNextStartElement()) {
                if (xmlReader.name() == "MainWindowState")
                    loadMainWindowState(xmlReader);
                else if (xmlReader.name() == "CentralWidget")
                    loadCentralWidgetState(xmlReader);
                else if (xmlReader.name() == "DockWidgets")
                    emit loadDockWidgetsLayoutRequested(xmlReader);
                else
                    xmlReader.skipCurrentElement();
            }
        }
    }

    if (xmlReader.hasError()) {
        QMessageBox::warning(m_mainWindow, tr("Error"),
                             tr("Failed to parse XML file: %1").arg(xmlReader.errorString()));
    }

    file.close();
}

void LayoutManager::saveMainWindowState(QXmlStreamWriter &xmlWriter)
{
    xmlWriter.writeStartElement("MainWindowState");

    const QRect geometry = m_mainWindow->geometry();
    xmlWriter.writeTextElement("Geometry",
                               QString("%1,%2,%3,%4")
                                   .arg(geometry.x())
                                   .arg(geometry.y())
                                   .arg(geometry.width())
                                   .arg(geometry.height()));

    xmlWriter.writeTextElement("NestedDocking",
                               m_mainWindow->isDockNestingEnabled() ? "true" : "false");

    xmlWriter.writeEndElement();
}

void LayoutManager::loadMainWindowState(QXmlStreamReader &xmlReader)
{
    QRect geometry;
    bool nestedDocking = false;

    while (xmlReader.readNextStartElement()) {
        if (xmlReader.name() == "Geometry") {
            QStringList geo = xmlReader.readElementText().split(',');
            if (geo.size() == 4) {
                geometry = QRect(geo[0].toInt(), geo[1].toInt(),
                                 geo[2].toInt(), geo[3].toInt());
            }
        } else if (xmlReader.name() == "NestedDocking") {
            nestedDocking = (xmlReader.readElementText() == "true");
        } else {
            xmlReader.skipCurrentElement();
        }
    }

    if (geometry.isValid()) {
        m_mainWindow->setGeometry(geometry);
    }
    m_mainWindow->setDockNestingEnabled(nestedDocking);
}

void LayoutManager::saveCentralWidgetState(QXmlStreamWriter &xmlWriter)
{
    QWidget *central = m_mainWindow->centralWidget();
    if (!central) return;

    xmlWriter.writeStartElement("CentralWidget");

    xmlWriter.writeTextElement("ObjectName", central->objectName());

    const QRect geometry = central->geometry();
    xmlWriter.writeTextElement("Geometry",
                               QString("%1,%2,%3,%4")
                                   .arg(geometry.x())
                                   .arg(geometry.y())
                                   .arg(geometry.width())
                                   .arg(geometry.height()));

    xmlWriter.writeTextElement("MinimumSize",
                               QString("%1,%2")
                                   .arg(central->minimumSize().width())
                                   .arg(central->minimumSize().height()));

    if (QTextEdit *textEdit = qobject_cast<QTextEdit*>(central)) {
        xmlWriter.writeTextElement("Text", textEdit->toPlainText());
        xmlWriter.writeTextElement("ReadOnly",
                                   textEdit->isReadOnly() ? "true" : "false");
    }

    xmlWriter.writeEndElement();
}

void LayoutManager::loadCentralWidgetState(QXmlStreamReader &xmlReader)
{
    QWidget *central = m_mainWindow->centralWidget();
    if (!central) return;

    QString objectName;
    QRect geometry;
    QSize minSize;
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
        } else if (xmlReader.name() == "Text") {
            text = xmlReader.readElementText();
        } else if (xmlReader.name() == "ReadOnly") {
            readOnly = (xmlReader.readElementText() == "true");
        } else {
            xmlReader.skipCurrentElement();
        }
    }

    central->setObjectName(objectName);
    if (!geometry.isNull()) central->setGeometry(geometry);
    if (!minSize.isNull()) central->setMinimumSize(minSize);

    if (QTextEdit *textEdit = qobject_cast<QTextEdit*>(central)) {
        textEdit->setPlainText(text);
        textEdit->setReadOnly(readOnly);
    }
}
