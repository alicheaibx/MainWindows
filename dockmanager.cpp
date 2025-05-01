#include "dockmanager.h"
#include <QTextEdit>
#include <QAction>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QEvent>

DockManager::DockManager(QMainWindow *parent)
    : QObject(parent), m_mainWindow(parent), m_viewMenu(new QMenu(tr("&View"), parent))
{
    setupDockWidgets();
}

DockManager::~DockManager()
{
    qDeleteAll(m_dockWidgets);
}

ColorSwatch* DockManager::dockWidget(const QString &name) const
{
    for (ColorSwatch *swatch : m_dockWidgets) {
        if (swatch->objectName() == name)
            return swatch;
    }
    return nullptr;
}

ColorSwatch* DockManager::createColorSwatch(const QString &colorName, Qt::DockWidgetArea area)
{
    ColorSwatch *swatch = new ColorSwatch(colorName, m_mainWindow);
    swatch->setObjectName(colorName + "Dock");
    swatch->setMinimumSize(150, 150);
    swatch->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    m_dockWidgetAreas[swatch] = area;
    m_mainWindow->addDockWidget(area, swatch);
    m_dockWidgets.append(swatch);

    connect(swatch, &QDockWidget::dockLocationChanged,
            this, &DockManager::handleDockLocationChanged);
    connect(swatch, &QDockWidget::topLevelChanged,
            this, [this, swatch](bool floating) {
                if (!floating) {
                    QTimer::singleShot(0, this, [this, swatch]() {
                        updateDockWidgetSizeConstraints(swatch);
                    });
                }
            });

    swatch->installEventFilter(this);
    emit dockWidgetCreated(swatch);
    return swatch;
}

void DockManager::setupDockWidgets()
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

        QAction *action = m_viewMenu->addAction(setting.name);
        action->setCheckable(true);
        action->setChecked(true);
        m_actionToDockWidgetMap[action] = swatch;
        connect(action, &QAction::toggled, this, &DockManager::toggleDockWidgetVisibility);
    }
}

void DockManager::setDockWidgetFeatures(const QString &name, QDockWidget::DockWidgetFeatures features)
{
    if (ColorSwatch *swatch = dockWidget(name)) {
        swatch->setFeatures(features);
        emit dockWidgetFeaturesChanged(name, features);
    }
}

void DockManager::setDockWidgetAllowedAreas(const QString &name, Qt::DockWidgetAreas areas)
{
    if (ColorSwatch *swatch = dockWidget(name)) {
        swatch->setAllowedAreas(areas);
    }
}

void DockManager::setDockWidgetFloating(const QString &name, bool floating)
{
    if (ColorSwatch *swatch = dockWidget(name)) {
        swatch->setFloating(floating);
    }
}

void DockManager::setDockWidgetVisible(const QString &name, bool visible)
{
    if (ColorSwatch *swatch = dockWidget(name)) {
        swatch->setVisible(visible);
        emit dockWidgetVisibilityChanged(name, visible);
    }
}

void DockManager::toggleDockWidget(const QString &name)
{
    if (ColorSwatch *swatch = dockWidget(name)) {
        bool visible = !swatch->isVisible();
        swatch->setVisible(visible);
        emit dockWidgetVisibilityChanged(name, visible);
    }
}

bool DockManager::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        ColorSwatch *swatch = qobject_cast<ColorSwatch*>(watched);
        if (swatch && !m_blockResizeUpdates && !m_sizesFixed) {
            m_dockWidgetSizes[swatch] = swatch->frameGeometry().size();
            updateTabbedGroupSizes(swatch);
        }
    }
    return QObject::eventFilter(watched, event);
}

void DockManager::handleDockWidgetResized(ColorSwatch *swatch)
{
    Qt::DockWidgetArea area = m_dockWidgetAreas.value(swatch, Qt::NoDockWidgetArea);
    m_dockWidgetSizes[swatch] = swatch->frameGeometry().size();
    updateTabbedGroupSizes(swatch);
}

void DockManager::updateTabbedGroupSizes(ColorSwatch *swatch)
{
    QList<QDockWidget*> tabbedGroup = m_mainWindow->tabifiedDockWidgets(swatch);
    if (!tabbedGroup.isEmpty()) {
        m_blockResizeUpdates = true;
        for (QDockWidget *tabbedDock : tabbedGroup) {
            ColorSwatch *tabbedSwatch = qobject_cast<ColorSwatch*>(tabbedDock);
            if (tabbedSwatch) {
                tabbedSwatch->resize(swatch->size());
                m_dockWidgetSizes[tabbedSwatch] = swatch->frameGeometry().size();
            }
        }
        m_blockResizeUpdates = false;
    }
}

void DockManager::toggleDockWidgetVisibility(bool checked)
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        if (m_actionToDockWidgetMap.contains(action)) {
            ColorSwatch *swatch = m_actionToDockWidgetMap[action];
            swatch->setVisible(checked);
            emit dockWidgetVisibilityChanged(swatch->objectName(), checked);
        }
    }
}

void DockManager::handleDockLocationChanged(Qt::DockWidgetArea area)
{
    if (ColorSwatch *swatch = qobject_cast<ColorSwatch*>(sender())) {
        m_dockWidgetAreas[swatch] = area;
        updateDockWidgetSizeConstraints(swatch);
    }
}

void DockManager::updateDockWidgetSizeConstraints(ColorSwatch *swatch)
{
    if (!swatch) return;

    if (m_sizesFixed) {
        if (m_dockWidgetSizes.contains(swatch)) {
            QSize frameSize = m_dockWidgetSizes[swatch];
            QSize clientSize = frameSize - (swatch->frameGeometry().size() - swatch->size());
            swatch->setMinimumSize(clientSize);
            swatch->setMaximumSize(clientSize);
        }
    } else {
        swatch->setMinimumSize(150, 150);
        swatch->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    }
}

void DockManager::saveDockWidgetsLayout(QXmlStreamWriter &xmlWriter)
{
    xmlWriter.writeStartElement("DockWidgets");

    QSet<QDockWidget*> savedDockWidgets;
    for (ColorSwatch *dockWidget : m_dockWidgets) {
        if (savedDockWidgets.contains(dockWidget)) continue;

        xmlWriter.writeStartElement("DockWidget");
        xmlWriter.writeAttribute("name", dockWidget->objectName());

        QSize size = dockWidget->frameGeometry().size();
        xmlWriter.writeStartElement("Size");
        xmlWriter.writeTextElement("width", QString::number(size.width()));
        xmlWriter.writeTextElement("height", QString::number(size.height()));
        xmlWriter.writeEndElement();

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

void DockManager::loadDockWidgetsLayout(QXmlStreamReader &xmlReader)
{
    m_sizesFixed = true;

    QMap<QString, ColorSwatch*> dockWidgetMap;
    for (ColorSwatch *dockWidget : m_dockWidgets) {
        dockWidgetMap[dockWidget->objectName()] = dockWidget;
    }

    QMap<ColorSwatch*, QList<ColorSwatch*>> tabbedGroups;

    while (xmlReader.readNextStartElement()) {
        if (xmlReader.name() == "DockWidget") {
            QString name = xmlReader.attributes().value("name").toString();
            QString title;
            bool visible = true;
            bool floating = false;
            QSize size(200, 200);
            QString dockAreaStr;
            QPoint floatingPos;
            QDockWidget::DockWidgetFeatures features = QDockWidget::DockWidgetClosable |
                                                       QDockWidget::DockWidgetMovable |
                                                       QDockWidget::DockWidgetFloatable;
            Qt::DockWidgetAreas allowedAreas = Qt::AllDockWidgetAreas;

            while (xmlReader.readNextStartElement()) {
                QString elementName = xmlReader.name().toString();
                if (elementName == "Title") {
                    title = xmlReader.readElementText();
                } else if (elementName == "Visible") {
                    visible = xmlReader.readElementText() == "true";
                } else if (elementName == "Floating") {
                    floating = xmlReader.readElementText() == "true";
                } else if (elementName == "Features") {
                    features = static_cast<QDockWidget::DockWidgetFeatures>(xmlReader.readElementText().toInt());
                } else if (elementName == "AllowedAreas") {
                    allowedAreas = static_cast<Qt::DockWidgetAreas>(xmlReader.readElementText().toInt());
                } else if (elementName == "Size") {
                    while (xmlReader.readNextStartElement()) {
                        if (xmlReader.name() == "width") {
                            size.setWidth(xmlReader.readElementText().toInt());
                        } else if (xmlReader.name() == "height") {
                            size.setHeight(xmlReader.readElementText().toInt());
                        }
                    }
                } else if (elementName == "DockArea") {
                    dockAreaStr = xmlReader.readElementText();
                } else if (elementName == "Geometry") {
                    while (xmlReader.readNextStartElement()) {
                        if (xmlReader.name() == "x") {
                            floatingPos.setX(xmlReader.readElementText().toInt());
                        } else if (xmlReader.name() == "y") {
                            floatingPos.setY(xmlReader.readElementText().toInt());
                        }
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
                m_blockResizeUpdates = true;

                dockWidget->setWindowTitle(title);
                dockWidget->setVisible(visible);
                dockWidget->setFeatures(features);
                dockWidget->setAllowedAreas(allowedAreas);
                m_dockWidgetSizes[dockWidget] = size;

                if (floating) {
                    dockWidget->setFloating(true);
                    if (!floatingPos.isNull()) {
                        dockWidget->move(floatingPos);
                    }
                } else {
                    Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;
                    if (dockAreaStr == "Right") area = Qt::RightDockWidgetArea;
                    else if (dockAreaStr == "Top") area = Qt::TopDockWidgetArea;
                    else if (dockAreaStr == "Bottom") area = Qt::BottomDockWidgetArea;

                    m_mainWindow->addDockWidget(area, dockWidget);
                    m_dockWidgetAreas[dockWidget] = area;
                }

                m_blockResizeUpdates = false;
                updateDockWidgetSizeConstraints(dockWidget);
            }
        }
    }

    for (auto it = tabbedGroups.begin(); it != tabbedGroups.end(); ++it) {
        for (ColorSwatch *tabbedDock : it.value()) {
            m_mainWindow->tabifyDockWidget(it.key(), tabbedDock);
        }
    }

    applySavedSizes();

    QTimer::singleShot(30, this, [this]() {
        m_sizesFixed = false;
        for (ColorSwatch *dockWidget : m_dockWidgets) {
            updateDockWidgetSizeConstraints(dockWidget);
        }
    });
}

void DockManager::applySavedSizes()
{
    m_blockResizeUpdates = true;
    for (ColorSwatch *dockWidget : m_dockWidgets) {
        if (m_dockWidgetSizes.contains(dockWidget)) {
            QSize frameSize = m_dockWidgetSizes[dockWidget];
            QSize clientSize = frameSize - (dockWidget->frameGeometry().size() - dockWidget->size());
            dockWidget->setMinimumSize(clientSize);
            dockWidget->setMaximumSize(clientSize);
            dockWidget->resize(clientSize);
        }
    }
    m_blockResizeUpdates = false;
}

void DockManager::saveDockWidgetSize(ColorSwatch *swatch)
{
    if (swatch) {
        m_dockWidgetSizes[swatch] = swatch->frameGeometry().size();
        updateTabbedGroupSizes(swatch);
    }
}

QSize DockManager::savedDockWidgetSize(const QString &name) const
{
    if (ColorSwatch *swatch = dockWidget(name)) {
        return m_dockWidgetSizes.value(swatch, swatch->frameGeometry().size());
    }
    return QSize();
}

void DockManager::setSizesFixed(bool fixed)
{
    if (m_sizesFixed != fixed) {
        m_sizesFixed = fixed;
        for (ColorSwatch *dockWidget : m_dockWidgets) {
            updateDockWidgetSizeConstraints(dockWidget);
        }
    }
}
