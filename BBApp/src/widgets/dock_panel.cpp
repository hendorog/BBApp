#include "dock_panel.h"

DockPanel::DockPanel(const QString &title,
                     QWidget *parent)
    : QDockWidget(title, parent, Qt::Widget)
{
    // Minimal features, no floating
    setFeatures(QDockWidget::DockWidgetMovable);
    //            | QDockWidget::DockWidgetClosable);

    setMaximumWidth(DOCK_WIDTH + 20);
    //setMinimumWidth(50);
    //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);

    QString objectName = title + "Panel";
    //setObjectName(objectName);

    scrollArea = new QScrollArea(this);
    scrollArea->move(0, 0);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setWidget(scrollArea);

    scrollWidget = new QWidget(scrollArea);
    scrollWidget->setObjectName("SH_Panel");
    scrollArea->setWidget(scrollWidget);

    //scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setAllowedAreas(Qt::RightDockWidgetArea |
                    Qt::LeftDockWidgetArea);
}

void DockPanel::AddPage(DockPage *page)
{
    connect(page, SIGNAL(tabUpdated()),
            this, SLOT(tabsChanged()));

    tabs.push_back(page);
    page->setParent(scrollArea->widget());

    tabsChanged();
}

void DockPanel::SaveState(QSettings &s)
{
    for(DockPage *page : tabs) {
        QString key = "PanelTabs/" +
                windowTitle() + "/" +
                page->PageName();
        s.setValue(key, page->TabIsOpen());
    }
}

void DockPanel::RestoreState(const QSettings &s)
{
    for(DockPage *page : tabs) {
        QString key = "PanelTabs/" +
                windowTitle() + "/" +
                page->PageName();
        QVariant value = s.value(key, true);
        page->SetOpen(value.toBool());
    }
}

void DockPanel::tabsChanged()
{
    int height = 10;
    int pageHeight = 0;

    for(int i = 0; i < tabs.size(); i++) {
        if(tabs[i]->TabIsOpen()) {
            pageHeight = tabs[i]->GetTotalHeight();
        } else {
            pageHeight = tabs[i]->TabHeight();
        }
        tabs[i]->resize(DOCK_WIDTH, pageHeight);
        tabs[i]->move(0, height);
        height += pageHeight + 1;
    }

    scrollWidget->resize(DOCK_WIDTH, height);
}
