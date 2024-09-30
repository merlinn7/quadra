#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_quadrasoftware.h"

// arcgis map sdk
#include <ArcGISRuntimeEnvironment.h>
#include <ArcGISMapImageLayer.h>
#include <Map.h>
#include <Basemap.h>
#include <MapTypes.h>
#include <MapGraphicsView.h>

class quadrasoftware : public QMainWindow
{
    Q_OBJECT

public:
    quadrasoftware(QWidget *parent = nullptr);
    ~quadrasoftware();

    Esri::ArcGISRuntime::Map* m_map = nullptr;
    Esri::ArcGISRuntime::MapGraphicsView* m_mapView = nullptr;

private:
    Ui::quadrasoftwareClass ui;
protected:
    void closeEvent(QCloseEvent* event) override;
private slots:
    void on_tabButton1_clicked();
    void on_tabButton2_clicked();
    void on_connectButton_clicked();
};
