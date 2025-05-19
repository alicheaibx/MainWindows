#include "layoutmanager.h"
#include <QFile>
#include <QMessageBox>
#include <QTextEdit>
#include <QDockWidget>
#include <QMenu>
#include <qmenubar.h>
#include <QSet>
LayoutManager::LayoutManager(QMainWindow *parent)
    : QObject(parent), m_mainWindow(parent)
{
    setupDockWidgets();
}

LayoutManager::~LayoutManager()
{
    qDeleteAll(m_dockWidgets);
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

    const QRect geometry = m_mainWindow->frameGeometry();
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

    if (QTextEdit *textEdit = qobject_cast<QTextEdit*>(central)) {
        textEdit->setPlainText(text);
        textEdit->setReadOnly(readOnly);
    }
}

void LayoutManager::setupDockWidgets()
{
    static const struct {
        const char *name;
        Qt::DockWidgetArea area;
    } dockSettings[] = {
        {"Black", Qt::LeftDockWidgetArea},
        {"White", Qt::RightDockWidgetArea},
        {"Red", Qt::TopDockWidgetArea},
        {"Green", Qt::TopDockWidgetArea},
        {"Blue", Qt::BottomDockWidgetArea},
        {"Yellow", Qt::BottomDockWidgetArea}
    };

    for (const auto &setting : dockSettings) {
        ColorSwatch *swatch = createColorSwatch(setting.name, setting.area);
        QAction *action = m_mainWindow->menuBar()->addAction(setting.name);
        action->setCheckable(true);
        action->setChecked(true);
        m_actionToDockWidgetMap[action] = swatch;
        connect(action, &QAction::toggled, this, &LayoutManager::toggleDockWidgetVisibility);
    }
}

ColorSwatch* LayoutManager::createColorSwatch(const QString &colorName, Qt::DockWidgetArea area)
{
    ColorSwatch *swatch = new ColorSwatch(colorName, m_mainWindow);
    swatch->setObjectName(colorName + "Dock");
    swatch->setMinimumSize(150, 150);
    swatch->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    m_dockWidgetAreas[swatch] = area;
    m_mainWindow->addDockWidget(area, swatch);
    m_dockWidgets.append(swatch);

    connect(swatch, &QDockWidget::dockLocationChanged,
            this, &LayoutManager::handleDockLocationChanged);


    return swatch;
}

void LayoutManager::saveDockWidgetsLayout(QXmlStreamWriter &xmlWriter)
{
    xmlWriter.writeStartElement("DockWidgets");
    xmlWriter.writeTextElement("ResizeEnabled", m_resizeEnabled ? "true" : "false");

    QSet<QDockWidget*> savedDockWidgets;
    for (ColorSwatch *dockWidget : m_dockWidgets) {
        if (savedDockWidgets.contains(dockWidget)) continue;

        xmlWriter.writeStartElement("DockWidget");
        xmlWriter.writeAttribute("name", dockWidget->objectName());

        saveWidgetProperties(xmlWriter, dockWidget->widget());

        xmlWriter.writeTextElement("Title", dockWidget->windowTitle());
        xmlWriter.writeTextElement("Visible", dockWidget->isVisible() ? "true" : "false");
        xmlWriter.writeTextElement("Floating", dockWidget->isFloating() ? "true" : "false");
        xmlWriter.writeTextElement("Features", QString::number(static_cast<int>(dockWidget->features())));
        xmlWriter.writeTextElement("AllowedAreas", QString::number(static_cast<int>(dockWidget->allowedAreas())));

        if (dockWidget->isFloating()) {
            QRect geo = dockWidget->frameGeometry();
            xmlWriter.writeStartElement("Geometry");
            xmlWriter.writeTextElement("x", QString::number(geo.x()));
            xmlWriter.writeTextElement("y", QString::number(geo.y()));
            xmlWriter.writeTextElement("width", QString::number(geo.width()));
            xmlWriter.writeTextElement("height", QString::number(geo.height()));
            xmlWriter.writeEndElement();
        } else {
            QString areaStr;
            switch (m_mainWindow->dockWidgetArea(dockWidget)) {
            case Qt::LeftDockWidgetArea: areaStr = "Left"; break;
            case Qt::RightDockWidgetArea: areaStr = "Right"; break;
            case Qt::TopDockWidgetArea: areaStr = "Top"; break;
            case Qt::BottomDockWidgetArea: areaStr = "Bottom"; break;
            default: areaStr = "Floating";
            }
            xmlWriter.writeTextElement("DockArea", areaStr);
        }

        QList<QDockWidget*> tabbedGroup = m_mainWindow->tabifiedDockWidgets(dockWidget);
        if (!tabbedGroup.isEmpty()) {
            xmlWriter.writeStartElement("TabbedGroup");
            for (QDockWidget *tabbedDock : tabbedGroup) {
                xmlWriter.writeTextElement("DockWidget", tabbedDock->objectName());
                savedDockWidgets.insert(tabbedDock);
            }
            xmlWriter.writeEndElement();
        }

        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
}

void LayoutManager::loadDockWidgetsLayout(QXmlStreamReader &xmlReader)
{
    QMap<QString, ColorSwatch*> dockWidgetMap;
    for (ColorSwatch *dockWidget : m_dockWidgets) {
        dockWidgetMap[dockWidget->objectName()] = dockWidget;
    }

    QMap<ColorSwatch*, QList<ColorSwatch*>> tabbedGroups;
    bool resizeEnabled = m_resizeEnabled;

    while (xmlReader.readNextStartElement()) {
        if (xmlReader.name() == "DockWidgets") {
            while (xmlReader.readNextStartElement()) {
                if (xmlReader.name() == "ResizeEnabled") {
                    resizeEnabled = xmlReader.readElementText() == "true";
                    setResizeEnabled(resizeEnabled);
                }
                else if (xmlReader.name() == "DockWidget") {
                    QString name = xmlReader.attributes().value("name").toString();
                    QString title;
                    bool visible = true;
                    bool floating = false;
                    QString dockAreaStr;
                    QPoint floatingPos;
                    QDockWidget::DockWidgetFeatures features = QDockWidget::DockWidgetClosable |
                                                               QDockWidget::DockWidgetMovable |
                                                               QDockWidget::DockWidgetFloatable;
                    Qt::DockWidgetAreas allowedAreas = Qt::AllDockWidgetAreas;

                    while (xmlReader.readNextStartElement()) {
                        QString elementName = xmlReader.name().toString();
                        if (elementName == "WidgetProperties") {
                            if (dockWidgetMap.contains(name)) {
                                loadWidgetProperties(xmlReader, dockWidgetMap[name]->widget());
                            } else {
                                xmlReader.skipCurrentElement();
                            }
                        } else if (elementName == "Title") {
                            title = xmlReader.readElementText();
                        } else if (elementName == "Visible") {
                            visible = xmlReader.readElementText() == "true";
                        } else if (elementName == "Floating") {
                            floating = xmlReader.readElementText() == "true";
                        } else if (elementName == "Features") {
                            features = static_cast<QDockWidget::DockWidgetFeatures>(xmlReader.readElementText().toInt());
                        } else if (elementName == "AllowedAreas") {
                            allowedAreas = static_cast<Qt::DockWidgetAreas>(xmlReader.readElementText().toInt());
                        } else if (elementName == "DockArea") {
                            dockAreaStr = xmlReader.readElementText();
                        } else if (elementName == "Geometry") {
                            int x = 0, y = 0, width = 0, height = 0;
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
                            floatingPos = QPoint(x, y);
                            if (dockWidgetMap.contains(name)) {
                                ColorSwatch *dockWidget = dockWidgetMap[name];
                                dockWidget->setFloating(true);
                                dockWidget->setGeometry(QRect(floatingPos, QSize(width, height)));
                            }
                        } else if (elementName == "TabbedGroup") {
                            QList<ColorSwatch*> tabbedGroup;
                            while (xmlReader.readNextStartElement()) {
                                if (xmlReader.name() == "DockWidget") {
                                    QString tabbedName = xmlReader.readElementText();
                                    if (dockWidgetMap.contains(tabbedName)) {
                                        tabbedGroup.append(dockWidgetMap[tabbedName]);
                                    }
                                }
                            }
                            if (dockWidgetMap.contains(name)) {
                                tabbedGroups[dockWidgetMap[name]] = tabbedGroup;
                            }
                        }
                    }

                    if (dockWidgetMap.contains(name)) {
                        ColorSwatch *dockWidget = dockWidgetMap[name];
                        dockWidget->setWindowTitle(title);
                        dockWidget->setVisible(visible);
                        dockWidget->setFeatures(features);
                        dockWidget->setAllowedAreas(allowedAreas);

                        if (!floating) {
                            Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;
                            if (dockAreaStr == "Right") area = Qt::RightDockWidgetArea;
                            else if (dockAreaStr == "Top") area = Qt::TopDockWidgetArea;
                            else if (dockAreaStr == "Bottom") area = Qt::BottomDockWidgetArea;

                            m_mainWindow->addDockWidget(area, dockWidget);
                            m_dockWidgetAreas[dockWidget] = area;
                        }
                    }
                }
            }
        }
    }

    // Apply tabbed groups
    for (auto it = tabbedGroups.begin(); it != tabbedGroups.end(); ++it) {
        for (ColorSwatch *tabbedDock : it.value()) {
            m_mainWindow->tabifyDockWidget(it.key(), tabbedDock);
        }
    }
}

void LayoutManager::saveWidgetProperties(QXmlStreamWriter &xmlWriter, QWidget *widget)
{
    if (!widget) return;

    xmlWriter.writeStartElement("WidgetProperties");

    xmlWriter.writeTextElement("ObjectName", widget->objectName());
    xmlWriter.writeTextElement("Geometry", QString("%1,%2,%3,%4")
                                               .arg(widget->geometry().x())
                                               .arg(widget->geometry().y())
                                               .arg(widget->geometry().width())
                                               .arg(widget->geometry().height()));

    if (ColorDock *colorDock = qobject_cast<ColorDock*>(widget)) {
        xmlWriter.writeTextElement("Color", colorDock->property("colorName").toString());
    }

    xmlWriter.writeEndElement();
}

void LayoutManager::loadWidgetProperties(QXmlStreamReader &xmlReader, QWidget *widget)
{
    if (!widget) return;

    QString objectName;
    QRect geometry;
    QString color;

    while (xmlReader.readNextStartElement()) {
        if (xmlReader.name() == "ObjectName") {
            objectName = xmlReader.readElementText();
        } else if (xmlReader.name() == "Geometry") {
            QStringList geo = xmlReader.readElementText().split(',');
            if (geo.size() == 4) {
                geometry = QRect(geo[0].toInt(), geo[1].toInt(),
                                 geo[2].toInt(), geo[3].toInt());
            }
        } else if (xmlReader.name() == "Color") {
            color = xmlReader.readElementText();
        } else {
            xmlReader.skipCurrentElement();
        }
    }

    widget->setObjectName(objectName);
    if (!geometry.isNull()) widget->setGeometry(geometry);
}

void LayoutManager::setResizeEnabled(bool enabled)
{
    m_resizeEnabled = enabled;
    for (ColorSwatch *swatch : m_dockWidgets) {
        if (enabled) {
            swatch->setFeatures(swatch->features() | QDockWidget::DockWidgetMovable);
            swatch->setMinimumSize(0, 0);
            swatch->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        } else {
            swatch->setFeatures(swatch->features() & ~QDockWidget::DockWidgetMovable);
            swatch->setFixedSize(swatch->size());
        }
    }
}

void LayoutManager::toggleDockWidgetVisibility(bool checked)
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        if (m_actionToDockWidgetMap.contains(action)) {
            ColorSwatch *swatch = m_actionToDockWidgetMap[action];
            swatch->setVisible(checked);
            emit dockWidgetVisibilityChanged(swatch->objectName(), checked);
        }
    }
}

void LayoutManager::handleDockLocationChanged(Qt::DockWidgetArea area)
{
    if (ColorSwatch *swatch = qobject_cast<ColorSwatch*>(sender())) {
        m_dockWidgetAreas[swatch] = area;
        updateTabbedGroupSizes(swatch);
    }
}

void LayoutManager::updateTabbedGroupSizes(ColorSwatch *swatch)
{
    QList<QDockWidget*> tabbedGroup = m_mainWindow->tabifiedDockWidgets(swatch);
    if (!tabbedGroup.isEmpty()) {
        for (QDockWidget *tabbedDock : tabbedGroup) {
            // Handle tabbed group sizes if needed
        }
    }
}
