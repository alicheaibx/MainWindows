// layoutmanager.cpp
#include "layoutmanager.h"
#include <QMainWindow>
#include <QMessageBox>
#include <QFile>
#include <QApplication>
#include <QStyle>

LayoutManager::LayoutManager(QMainWindow *parent)
    : QObject(parent), m_mainWindow(parent)
{
}

void LayoutManager::registerWidget(const QString &name, QWidget *widget)
{
    m_registeredWidgets[name] = widget;
}

void LayoutManager::unregisterWidget(const QString &name)
{
    m_registeredWidgets.remove(name);
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
    xmlWriter.writeStartElement("Layout");

    saveMainWindowState(xmlWriter);
    emit saveWidgetsLayoutRequested(xmlWriter);

    // Save registered widgets
    xmlWriter.writeStartElement("RegisteredWidgets");
    for (auto it = m_registeredWidgets.constBegin(); it != m_registeredWidgets.constEnd(); ++it) {
        xmlWriter.writeStartElement("Widget");
        xmlWriter.writeAttribute("name", it.key());
        saveWidgetProperties(xmlWriter, it.value());
        xmlWriter.writeEndElement(); // Widget
    }
    xmlWriter.writeEndElement(); // RegisteredWidgets

    xmlWriter.writeEndElement(); // Layout
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
        if (xmlReader.isStartElement() && xmlReader.name() == "Layout") {
            while (xmlReader.readNextStartElement()) {
                if (xmlReader.name() == "MainWindow") {
                    loadMainWindowState(xmlReader);
                } else if (xmlReader.name() == "DockWidgets") {
                    emit loadWidgetsLayoutRequested(xmlReader);
                } else if (xmlReader.name() == "RegisteredWidgets") {
                    while (xmlReader.readNextStartElement()) {
                        if (xmlReader.name() == "Widget") {
                            QString name = xmlReader.attributes().value("name").toString();
                            if (m_registeredWidgets.contains(name)) {
                                loadWidgetProperties(xmlReader, m_registeredWidgets[name]);
                            } else {
                                xmlReader.skipCurrentElement();
                            }
                        }
                    }
                } else {
                    xmlReader.skipCurrentElement();
                }
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
    xmlWriter.writeStartElement("MainWindow");

    // Save geometry and state - FIXED: Use QByteArray directly
    xmlWriter.writeStartElement("Geometry");
    QByteArray geometryData = m_mainWindow->saveGeometry();
    xmlWriter.writeTextElement("data", geometryData.toBase64());
    xmlWriter.writeEndElement(); // Geometry

    xmlWriter.writeStartElement("State");
    QByteArray stateData = m_mainWindow->saveState();
    xmlWriter.writeTextElement("data", stateData.toBase64());
    xmlWriter.writeEndElement(); // State

    // Save window state (normal, maximized, minimized, fullscreen)
    xmlWriter.writeTextElement("WindowState", QString::number(m_mainWindow->windowState()));

    xmlWriter.writeEndElement(); // MainWindow
}

void LayoutManager::loadMainWindowState(QXmlStreamReader &xmlReader)
{
    QByteArray geometryData;
    QByteArray stateData;
    Qt::WindowStates windowState = Qt::WindowNoState;

    while (xmlReader.readNextStartElement()) {
        if (xmlReader.name() == "Geometry") {
            while (xmlReader.readNextStartElement()) {
                if (xmlReader.name() == "data") {
                    geometryData = QByteArray::fromBase64(xmlReader.readElementText().toLatin1());
                }
            }
        } else if (xmlReader.name() == "State") {
            while (xmlReader.readNextStartElement()) {
                if (xmlReader.name() == "data") {
                    stateData = QByteArray::fromBase64(xmlReader.readElementText().toLatin1());
                }
            }
        } else if (xmlReader.name() == "WindowState") {
            windowState = static_cast<Qt::WindowStates>(xmlReader.readElementText().toInt());
        } else {
            xmlReader.skipCurrentElement();
        }
    }

    if (!geometryData.isEmpty()) {
        m_mainWindow->restoreGeometry(geometryData);
    }
    if (!stateData.isEmpty()) {
        m_mainWindow->restoreState(stateData);
    }
    m_mainWindow->setWindowState(windowState);
}

void LayoutManager::saveWidgetProperties(QXmlStreamWriter &xmlWriter, QWidget *widget)
{
    // Save geometry
    xmlWriter.writeStartElement("Geometry");
    if (widget->isWindow()) {
        // For top-level widgets, save frame geometry
        QRect frameGeo = widget->frameGeometry();
        xmlWriter.writeTextElement("x", QString::number(frameGeo.x()));
        xmlWriter.writeTextElement("y", QString::number(frameGeo.y()));
        xmlWriter.writeTextElement("width", QString::number(frameGeo.width()));
        xmlWriter.writeTextElement("height", QString::number(frameGeo.height()));
    } else {
        // For child widgets, save relative geometry
        QRect geo = widget->geometry();
        xmlWriter.writeTextElement("x", QString::number(geo.x()));
        xmlWriter.writeTextElement("y", QString::number(geo.y()));
        xmlWriter.writeTextElement("width", QString::number(geo.width()));
        xmlWriter.writeTextElement("height", QString::number(geo.height()));
    }
    xmlWriter.writeEndElement(); // Geometry

    // Save window state
    if (widget->isWindow()) {
        xmlWriter.writeTextElement("WindowState", QString::number(widget->windowState()));
        xmlWriter.writeTextElement("Visible", widget->isVisible() ? "true" : "false");
    }

    // Save other properties
    xmlWriter.writeTextElement("Enabled", widget->isEnabled() ? "true" : "false");
    xmlWriter.writeTextElement("StyleSheet", widget->styleSheet());
}

void LayoutManager::loadWidgetProperties(QXmlStreamReader &xmlReader, QWidget *widget)
{
    while (xmlReader.readNextStartElement()) {
        if (xmlReader.name() == "Geometry") {
            int x = 0, y = 0, width = 100, height = 30;
            while (xmlReader.readNextStartElement()) {
                if (xmlReader.name() == "x") {
                    x = xmlReader.readElementText().toInt();
                } else if (xmlReader.name() == "y") {
                    y = xmlReader.readElementText().toInt();
                } else if (xmlReader.name() == "width") {
                    width = xmlReader.readElementText().toInt();
                } else if (xmlReader.name() == "height") {
                    height = xmlReader.readElementText().toInt();
                }
            }
            if (widget->isWindow()) {
                widget->setGeometry(x, y, width, height);
            } else {
                widget->setGeometry(QRect(x, y, width, height));
            }
        } else if (xmlReader.name() == "WindowState") {
            if (widget->isWindow()) {
                widget->setWindowState(static_cast<Qt::WindowStates>(xmlReader.readElementText().toInt()));
            }
        } else if (xmlReader.name() == "Visible") {
            if (widget->isWindow()) {
                widget->setVisible(xmlReader.readElementText() == "true");
            }
        } else if (xmlReader.name() == "Enabled") {
            widget->setEnabled(xmlReader.readElementText() == "true");
        } else if (xmlReader.name() == "StyleSheet") {
            widget->setStyleSheet(xmlReader.readElementText());
        } else {
            xmlReader.skipCurrentElement();
        }
    }
}
