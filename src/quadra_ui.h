#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_quadrasoftware.h"
#include <windows.h>
#include <qfuture.h>
#include <vector>
#include "QuadraMavInterface.h"
#include <iostream>

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
#include <map>
#include "CircularProgressBar.hpp"
#include <qscrollbar.h>
using namespace Esri::ArcGISRuntime;

class QuadraMissionItem
{
public:
	QuadraMissionItem(
		const std::string& cmd_ = "UNKNOWN",
		double lat_ = 0,
		double lon_ = 0,
		float alt_ = 0,
		float p1_ = 0,
		float p2_ = 0,
		float p3_ = 0,
		float p4_ = NAN,
		bool auto_continue_ = true)
		: cmd(cmd_), lat(lat_), lon(lon_), alt(alt_), p1(p1_), p2(p2_), p3(p3_), p4(p4_), auto_continue(auto_continue_)
	{
	}

	std::string cmd;
	double lat;
	double lon;
	float alt;
	float p1;
	float p2;
	float p3;
	float p4;
	bool auto_continue;
};

class UavGraphics
{
public:
	UavGraphics(std::string i_connection_url, Graphic* i_waypointGraphic, Graphic* i_planeGraphic, Graphic* i_targetLineGraphic)
	{
		waypointGraphic = i_waypointGraphic;
		planeGraphic = i_planeGraphic;
		targetLineGraphic = i_targetLineGraphic;
		connection_url = i_connection_url;
	}
	Graphic* waypointGraphic;
	Graphic* planeGraphic;
	Graphic* targetLineGraphic;
	std::string connection_url;
};
bool refresh_map = false;
static std::vector<UavGraphics> uav_graphics;

class MissionGraphics
{
public:
	Graphic* line_graphic;
	Graphic* circle_graphic;
	Graphic* text_graphic;
};
bool refresh_mission_map = false;
bool refresh_mission = false;
static std::vector<MissionGraphics> mission_graphics;

class SettingBase {
public:
	virtual ~SettingBase() = default;
	virtual std::string get_name() const = 0;
	virtual std::string get_value() const = 0;
	virtual void set_value(const std::string new_value) = 0;
};

template <typename T>
class Setting : public SettingBase
{
public:
	Setting(std::string name, T* value) : name(name), value(value) {}

	std::string get_value() const override {
		return std::to_string(*value);
	}

	void set_value(const std::string newValue) override {
		std::stringstream ss(newValue);
		ss >> *value;
	}

	std::string get_name() const override {
		return name;
	}

	std::string name;
	T* value;
};

class quadrasoftware : public QMainWindow
{
	Q_OBJECT

public:
	quadrasoftware(QWidget* parent = nullptr);
	~quadrasoftware();

	Esri::ArcGISRuntime::Map* m_map = nullptr;
	Esri::ArcGISRuntime::MapGraphicsView* m_mapView = nullptr;
	GraphicsOverlay* map_graphicsOverlay = nullptr;
	bool bTrackPlane = false;

	Esri::ArcGISRuntime::Map* m_map2 = nullptr;
	Esri::ArcGISRuntime::MapGraphicsView* m_mapView2 = nullptr;
	GraphicsOverlay* map_graphicsOverlay2 = nullptr;

	int currentSystem;

private:
	Ui::quadrasoftwareClass ui;

	bool refresh_settings = false;
	std::vector<std::unique_ptr<SettingBase>> settings;
	void InitSettings();

	void AiAssistant();
	void RefreshParams();
	bool refresh_params = true;

	int max_rpm = 2000;
	XQt::CircularProgressBar* progressBar1;
	XQt::CircularProgressBar* progressBar2;
	XQt::CircularProgressBar* progressBar3;
	XQt::CircularProgressBar* progressBar4;
	XQt::CircularProgressBar* progressBar5;

	Graphic* CreatePictureMarkerSymbolGraphic(std::string imagePath, int width, int height, int angle, Point location, MapGraphicsView* map);
	Graphic* CreatePolylineSymbolGraphic(Point location1, Point location2, SimpleLineSymbol* lineSymbol);

	void mapContextMenuRequest(QPoint pos);
	void map2ContextMenuRequest(QPoint pos);
	void setCursorViewpoint(const QPoint& pos);

	// Mission
	std::vector<QuadraMissionItem> ParseMission(std::vector<MissionRaw::MissionItem> mission);
	std::vector<MissionRaw::MissionItem> ParseMission(std::vector<QuadraMissionItem> mission);
	std::vector<QuadraMissionItem> GetLocalMission();

	void AddMissionItem(QuadraMissionItem item);
	void RefreshMission();
	void ClearMission();
	void UploadMission();
	void RenderWaypoint(MissionRaw::MissionItem mission_item, Telemetry::Position& current_position, int mission_index);

	void HandleIndicators();

protected:
	void closeEvent(QCloseEvent* event) override;
private slots:
	void on_tabButton1_clicked();
	void on_tabButton2_clicked();
	void on_tabButton3_clicked();
	void on_connectButton_clicked();
	void on_connectButton_2_clicked();
	void on_armButton_clicked();
	void on_takeoffButton_clicked();
	void on_landingButton_clicked();
	void on_startMissionBtn_clicked();
	void on_deleteMissionBtn_clicked();
	void on_refreshMissionBtn_clicked();
	void on_clearMissionBtn_clicked();
	void on_uploadMissionBtn_clicked();
	void on_documentsBtn_clicked();
	void on_vtolButton_clicked();
	void on_holdButton_clicked();
	void on_rtlButton_clicked();
	void on_shutdownBtn_clicked();
	void on_rebootBtn_clicked();
	void on_aiButton_clicked();
	void on_logFrame_textChanged();
	void on_paramFilter_textChanged();
	void on_settingsTable_itemChanged(QTableWidgetItem* item);
	void on_systemsComboBox_currentIndexChanged(int index);
	void on_waypointTable_2_itemChanged(QTableWidgetItem* item);
	void on_paramTable_itemChanged(QTableWidgetItem* item);
};

class OutputStream : public std::streambuf
{
public:
	OutputStream(QTextEdit* textEdit) : textEdit(textEdit) {}

protected:
	virtual int overflow(int c) override
	{
		if (c != EOF) {
			if (c == '\n') {
				buffer += "\n";
				textEdit->append(QString::fromStdString(buffer));
				buffer.clear();
			}
			else {
				buffer += static_cast<char>(c);
			}
		}
		return c;
	}

private:
	QTextEdit* textEdit;
	std::string buffer;
};

QuadraMavInterface* QuadraInterface = new QuadraMavInterface([]() {
	// fixed: render ui while processing mavlink
	QApplication::processEvents();
	});

std::map<std::string, QuadraMavInterface*> systems;

// semi-auto error handling for interface results
// if return value is false then show error and return
#define HANDLE_ERRORS(result, error_msg) if (!result) {MessageBox(NULL, error_msg, L"Error", MB_ICONERROR); return;} 
#define NEW_THREAD(func_addr, ...) {std::thread thr(func_addr, __VA_ARGS__); thr.detach();}

// error messages
#define CONNECTION_ERROR_MSG L"Connect to PX4 first!"
#define TRANSITION_FW_ERROR_MSG L"Error while switching to fixedwing"
#define TRANSITION_MC_ERROR_MSG L"Error while switching to drone"
#define GOTO_LOCATION_ERROR_MSG L"Error while goto_location"
#define CLEAR_MISSION_ERROR_MSG L"Couldn't clear mission"
#define DOWNLOAD_MISSION_ERROR_MSG L"Couldn't download mission"
#define UPLOAD_MISSION_ERROR_MSG L"Couldn't upload mission"
#define HOLD_ERROR_MSG L"Error while hold"
#define RTL_ERROR_MSG L"Error while RTL"
#define SHUTDOWN_ERROR_MSG L"Couldn't shutdown system"
#define REBOOT_ERROR_MSG L"Couldn't reboot system"