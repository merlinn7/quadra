#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_quadrasoftware.h"
#include <windows.h>

#include "QuadraMavInterface.h"

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
    void on_tabButton3_clicked();
    void on_connectButton_clicked();
    void on_armButton_clicked();
    void on_takeoffButton_clicked();
    void on_landingButton_clicked();

};


QuadraMavInterface QuadraInterface = QuadraMavInterface( []() {
    // fixed: render ui while processing mavlink
    QApplication::processEvents();
    } );
