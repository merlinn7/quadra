#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_quadrasoftware.h"
#include <windows.h>
#include <qfuture.h>

#include "QuadraMavInterface.h"

// arcgis map sdk
#include <ArcGISRuntimeEnvironment.h>
#include <ArcGISMapImageLayer.h>
#include <Map.h>
#include <Basemap.h>
#include <MapTypes.h>
#include <MapGraphicsView.h>
#include "Viewpoint.h"
#include "Graphic.h"
#include "GraphicsOverlay.h"
#include "GraphicsOverlayListModel.h"
#include "GraphicListModel.h"
#include <SimpleMarkerSymbol.h>
#include "SymbolTypes.h"
#include "SimpleMarkerSymbol.h"
#include "Point.h"
#include "SpatialReference.h"
#include "PictureMarkerSymbol.h"
#include <Polyline.h>
#include <PolylineBuilder.h>
#include "SimpleLineSymbol.h"
#include <TaskWatcher.h>
#include <GeometryEngine.h>

using namespace Esri::ArcGISRuntime;
class quadrasoftware : public QMainWindow
{
    Q_OBJECT

public:
    quadrasoftware(QWidget *parent = nullptr);
    ~quadrasoftware();

    Esri::ArcGISRuntime::Map* m_map = nullptr;
    Esri::ArcGISRuntime::MapGraphicsView* m_mapView = nullptr;
    GraphicsOverlay* map_graphicsOverlay = nullptr;
    bool bTrackPlane = false;

private:
    Ui::quadrasoftwareClass ui;

    Graphic* CreatePictureMarkerSymbolGraphic(std::string imagePath, int width, int height, int angle, Point location);
    Graphic* CreatePolylineSymbolGraphic(Point location1, Point location2, SimpleLineSymbol* lineSymbol);
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
    void on_vtolButton_clicked();
    void mapContextMenuRequest(QPoint pos);
    void setCursorViewpoint(const QPoint& pos);
};


QuadraMavInterface QuadraInterface = QuadraMavInterface( []() {
    // fixed: render ui while processing mavlink
    QApplication::processEvents();
    } );

// semi-auto error handling for quadra return values
// if return value is false then show error and return
#define HANDLE_ERRORS(result, error_msg) if (!result) {MessageBox(NULL, error_msg, L"Error", MB_ICONERROR); return;} 

// error messages
#define TRANSITION_FW_ERROR_MSG L"Error while switching to fixedwing"
#define TRANSITION_MC_ERROR_MSG L"Error while switching to drone"
#define GOTO_LOCATION_ERROR_MSG L"Error while goto_location"