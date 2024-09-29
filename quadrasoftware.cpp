#include "quadrasoftware.h"
#include <windows.h>
#include <ArcGISRuntimeEnvironment.h>
#include <ArcGISMapImageLayer.h>
#include <Map.h>
#include <Basemap.h>
#include <MapTypes.h>
#include <MapGraphicsView.h>

Esri::ArcGISRuntime::Map* m_map = nullptr;
Esri::ArcGISRuntime::MapGraphicsView* m_mapView = nullptr;

using namespace Esri::ArcGISRuntime;
quadrasoftware::quadrasoftware(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    ui.horizontalFrame->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76;  background-color:#424242 ;}");
    ui.gridFrame->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76;  background-color:#424242 ;}");
    ui.frame->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76;  background-color:#424242 ;}");
    ui.frame_2->setStyleSheet("QFrame{border-radius: 12px; border: 0px solid #686D76;  background-color:#424242 ;}");
    ui.frame_3->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76; background-color:#424242 ;}");
    ui.stackedWidget->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76; background-color:#424242 ;}");

    const QString esriApiKey = QString("AAPKcd30878169c549719c3a517fa3d217582uWHsyC--Qvd0GlBFmU-rmNXhspCWZPP59m0PMLtN6ItSN8QnvpiTcB8uh0RBp9w");
    ArcGISRuntimeEnvironment::setApiKey(esriApiKey);

    m_map = new Map(BasemapStyle::ArcGISStreetsNight, this);
    m_mapView = new MapGraphicsView(ui.frame_2);

    m_mapView->setMap(m_map);
    QSizePolicy sizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
    sizePolicy.setHeightForWidth(m_mapView->sizePolicy().hasHeightForWidth());
    m_mapView->setSizePolicy(sizePolicy);
    ui.gridLayout_3->addWidget(m_mapView);
}

quadrasoftware::~quadrasoftware()
{}

void quadrasoftware::on_tabButton1_clicked()
{
    ui.stackedWidget->setCurrentIndex(0);
}


void quadrasoftware::on_tabButton2_clicked()
{
    ui.stackedWidget->setCurrentIndex(1);
}
