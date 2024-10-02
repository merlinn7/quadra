#include "quadra_ui.h"
#include "QuadraMavInterface.h"
#include <windows.h>
#include "connectDialog.h"
#include "takeoffInputDialog.h"
#include <qtimer.h>

using namespace Esri::ArcGISRuntime;
quadrasoftware::quadrasoftware(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// UI QSS applies
	ui.horizontalFrame->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76;  background-color:#424242 ;}");
	ui.gridFrame->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76;  background-color:#424242 ;}");
	ui.frame->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76;  background-color:#424242 ;}");
	ui.frame_2->setStyleSheet("QFrame{border-radius: 12px; border: 0px solid #686D76;  background-color:#424242 ;}");
	ui.frame_3->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76; background-color:#424242 ;}");
	ui.stackedWidget->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76; background-color:#424242 ;}");

	// map api key
	const QString esriApiKey = QString("AAPKcd30878169c549719c3a517fa3d217582uWHsyC--Qvd0GlBFmU-rmNXhspCWZPP59m0PMLtN6ItSN8QnvpiTcB8uh0RBp9w");
	ArcGISRuntimeEnvironment::setApiKey(esriApiKey);

	// map object 
	m_map = new Map(BasemapStyle::ArcGISStreetsNight, this);
	m_mapView = new MapGraphicsView(ui.frame_2);
	m_mapView->setMap(m_map);

	// init arcgis map graphics overlay
	map_graphicsOverlay = new GraphicsOverlay(m_mapView);
	m_mapView->graphicsOverlays()->append(map_graphicsOverlay);

	// create map way point
	const Viewpoint viewpoint(37.78304072903069, 29.09632386893564, 50000);
	m_mapView->setViewpointAsync(viewpoint);

	//Point goveclik = Point(28.96567444077879, 37.79262103901701, SpatialReference::wgs84());
	//Point cal = Point(29.39729159313304, 38.08300286281516, SpatialReference::wgs84());

	static Graphic* waypointGraphic = CreatePictureMarkerSymbolGraphic("images/way-point.png", 30, 30, 0, Point());
	static Graphic* planeGraphic = CreatePictureMarkerSymbolGraphic("images/plane_icon.png", 60, 60, 0, Point());
	static Graphic* lineGraphic = CreatePolylineSymbolGraphic(Point(), Point(), new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(Qt::white), 3, this));

	map_graphicsOverlay->graphics()->append(planeGraphic);
	map_graphicsOverlay->graphics()->append(lineGraphic);
	map_graphicsOverlay->graphics()->append(waypointGraphic);

	// for making the map responsive
	QSizePolicy sizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
	sizePolicy.setHeightForWidth(m_mapView->sizePolicy().hasHeightForWidth());
	m_mapView->setSizePolicy(sizePolicy);
	ui.gridLayout_3->addWidget(m_mapView);

	// buttons
	ui.tabButton1->setIcon(QIcon("images/map_icon.png"));
	ui.tabButton1->setIconSize(QSize(64, 64));

	ui.tabButton2->setIcon(QIcon("images/plane_icon.png"));
	ui.tabButton2->setIconSize(QSize(75, 75));

	ui.tabButton3->setIcon(QIcon("images/settings.png"));
	ui.tabButton3->setIconSize(QSize(75, 75));

	ui.landingButton->setIcon(QIcon("images/landing.png"));
	ui.landingButton->setIconSize(QSize(85, 85));

	ui.takeoffButton->setIcon(QIcon("images/takeoff.png"));
	ui.takeoffButton->setIconSize(QSize(64, 64));

	ui.vtolButton->setIcon(QIcon("images/drone.png"));
	ui.vtolButton->setIconSize(QSize(64, 64));

	static bool init = false;
	auto timer = new QTimer(parent);

	// main loop, runs on every frame
	connect(timer, &QTimer::timeout, [this]
		{
			if (QuadraInterface.IsConnected())
			{
				ui.connectButton->setIcon(QIcon("images/connected.png"));
				ui.connectButton->setIconSize(QSize(70, 70));

				// check arm state and update ui
				if (QuadraInterface.IsArmed())
				{
					ui.armButton->setText("ARMED");
					ui.armButton->setStyleSheet("QPushButton{background-color: rgb(0, 50, 0);border-radius: 5px; border: 2px solid green;color: white;} QPushButton:pressed{ background-color: rgb(0, 30, 0); }");
				}
				else
				{
					ui.armButton->setText("DISARMED");
					ui.armButton->setStyleSheet("QPushButton{background-color: rgb(50, 0, 0);border-radius: 5px; border: 2px solid red;color: white;} QPushButton:pressed{ background-color: rgb(30, 0, 0); }");
				}

				// handle vtol state indicator
				Telemetry::VtolState vtolState = QuadraInterface.GetVtolState();
				if (vtolState == Telemetry::VtolState::Fw || vtolState == Telemetry::VtolState::TransitionToFw)
				{
					ui.vtolButton->setIcon(QIcon("images/plane_icon.png"));
					ui.vtolButton->setIconSize(QSize(64, 64));
				}
				else if (vtolState == Telemetry::VtolState::Mc || vtolState == Telemetry::VtolState::TransitionToMc)
				{
					ui.vtolButton->setIcon(QIcon("images/drone.png"));
					ui.vtolButton->setIconSize(QSize(64, 64));
				}

				// update symbol positions on map to real positions
				Telemetry::Position position = QuadraInterface.GetPosition();
				Telemetry::Position targetPosition = QuadraInterface.GetTargetPosition();
				Telemetry::EulerAngle angles = QuadraInterface.GetAngles();

				// plane symbol
				PictureMarkerSymbol* newPlaneSymbol = reinterpret_cast<PictureMarkerSymbol*>(planeGraphic->symbol());
				newPlaneSymbol->setAngle(angles.yaw_deg);
				planeGraphic->setGeometry(Point(position.longitude_deg, position.latitude_deg, SpatialReference::wgs84()));
				planeGraphic->setSymbol(newPlaneSymbol);

				// line symbol
				PolylineBuilder* lineSymbol_builder= new PolylineBuilder(SpatialReference::wgs84(), this);
				lineSymbol_builder->addPoint(position.longitude_deg, position.latitude_deg);
				lineSymbol_builder->addPoint(targetPosition.longitude_deg, targetPosition.latitude_deg);
				lineGraphic->setGeometry(lineSymbol_builder->toGeometry());

				// waypoint symbol
				PictureMarkerSymbol* newWaypointSymbol = reinterpret_cast<PictureMarkerSymbol*>(waypointGraphic->symbol());
				waypointGraphic->setGeometry(Point(targetPosition.longitude_deg, targetPosition.latitude_deg, SpatialReference::wgs84()));
				waypointGraphic->setSymbol(newWaypointSymbol);
			}
			else
			{
				ui.connectButton->setIcon(QIcon("images/disconnected.png"));
				ui.connectButton->setIconSize(QSize(70, 70));

				ui.armButton->setText("DISARMED");
				ui.armButton->setStyleSheet("QPushButton{background-color: rgb(50, 0, 0);border-radius: 5px; border: 2px solid red;color: white;} QPushButton:pressed{ background-color: rgb(30, 0, 0); }");
			
				// if not connected, reset map symbols
				planeGraphic->setGeometry(Point());
				lineGraphic->setGeometry(Point());
				waypointGraphic->setGeometry(Point());
			}
		});
	timer->start();
}

quadrasoftware::~quadrasoftware()
{}

// TO:DO change warnings to custom messageboxes
void quadrasoftware::on_connectButton_clicked()
{
	if (QuadraInterface.IsConnected())
	{
		if (!QuadraInterface.Disconnect())
		{
			MessageBox(NULL, L"Not connected to system!", L"Error", MB_ICONERROR);
			return;
		}

		//MessageBox(NULL, L"Disconnection Success", L"Success", MB_OK);
	}
	else
	{
		connectDialog conDialog;
		int result = conDialog.exec();

		// check pressed ok or cancel
		if (result != QDialog::Accepted)
			return;

		if (QuadraInterface.Connect(conDialog.getConnectionURL().toStdString()))
		{
			//MessageBox(NULL, L"Connection Success", L"Success", MB_OK);
		}
		else
		{
			MessageBox(NULL, L"Connection Failed", L"Failed", MB_OK);
		}
	}
}

void quadrasoftware::on_armButton_clicked()
{
	if (!QuadraInterface.IsConnected())
	{
		MessageBox(NULL, L"Connect to PX4 first!", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	if (QuadraInterface.IsArmed())
	{
		if (!QuadraInterface.Disarm())
		{
			MessageBox(NULL, L"Error while disarm", L"Error", MB_ICONERROR);
			return;
		}
	}
	else
	{
		if (!QuadraInterface.Arm())
		{
			MessageBox(NULL, L"Error while arm", L"Error", MB_ICONERROR);
			return;
		}
	}
}

void quadrasoftware::on_takeoffButton_clicked()
{
	if (!QuadraInterface.IsConnected())
	{
		MessageBox(NULL, L"Connect to PX4 first!", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	if (!QuadraInterface.IsArmed())
	{
		MessageBox(NULL, L"Arm your vehicle first!", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	// open input dialog here
	takeoffInputDialog inputDlg;
	auto result = inputDlg.exec();

	if (result != QDialog::Accepted)
		return;

	if (!QuadraInterface.Takeoff(inputDlg.GetAltitude()))
	{
		MessageBox(NULL, L"Error while takeoff", L"Error", MB_OK | MB_ICONERROR);
	}
}

void quadrasoftware::on_landingButton_clicked()
{
	if (!QuadraInterface.IsConnected())
	{
		MessageBox(NULL, L"Connect to PX4 first!", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	if (!QuadraInterface.Land())
	{
		MessageBox(NULL, L"Error while landing", L"Error", MB_OK | MB_ICONERROR);
	}
}

void quadrasoftware::on_vtolButton_clicked()
{

}

// this function is responsible from panel button effects
void HandleTabButtons(Ui::quadrasoftwareClass ui)
{
	QString activeStyleSheet = "QPushButton{background-color: rgb(97, 97, 97);border-radius: 5px;} QPushButton:pressed{background-color: rgb(70, 70, 70);};";
	QString deactiveStyleSheet = "QPushButton{background-color: rgb(66, 66, 66);border-radius: 5px;} QPushButton:pressed{background-color: rgb(70, 70, 70);};";

	switch (ui.stackedWidget->currentIndex())
	{
	case 0: // when map page is active 
		ui.tabButton1->setStyleSheet(activeStyleSheet);
		ui.tabButton2->setStyleSheet(deactiveStyleSheet);
		ui.tabButton3->setStyleSheet(deactiveStyleSheet);
		break;
	case 1: // when data page is active
		ui.tabButton1->setStyleSheet(deactiveStyleSheet);
		ui.tabButton2->setStyleSheet(activeStyleSheet);
		ui.tabButton3->setStyleSheet(deactiveStyleSheet);
		break;
	case 2:
		ui.tabButton1->setStyleSheet(deactiveStyleSheet);
		ui.tabButton2->setStyleSheet(deactiveStyleSheet);
		ui.tabButton3->setStyleSheet(activeStyleSheet);
		break;
	default:
		break;
	}
}

void quadrasoftware::on_tabButton1_clicked()
{
	// set panel to map screen
	ui.stackedWidget->setCurrentIndex(0);
	HandleTabButtons(ui);
}

Graphic* quadrasoftware::CreatePictureMarkerSymbolGraphic(std::string imagePath, int width, int height, int angle, Point location = Point(0, 0, SpatialReference::wgs84()))
{
	PictureMarkerSymbol* waypoint = new PictureMarkerSymbol(QUrl::fromLocalFile(imagePath.c_str()), this);
	waypoint->setWidth(width);
	waypoint->setHeight(height);

	Graphic* waypointPng = new Graphic(location, waypoint);
	return waypointPng;
}

Graphic* quadrasoftware::CreatePolylineSymbolGraphic(Point location1, Point location2, SimpleLineSymbol* lineSymbol)
{
	PolylineBuilder* polyline_builder = new PolylineBuilder(SpatialReference::wgs84(), this);
	polyline_builder->addPoint(location1);
	polyline_builder->addPoint(location2);

	return new Graphic(polyline_builder->toGeometry(), lineSymbol, this);
}

void quadrasoftware::closeEvent(QCloseEvent* event)
{
	// fixme: crash when window closed
	QuadraInterface.Disconnect();
	exit(0);
}

void quadrasoftware::on_tabButton2_clicked()
{
	// set panel to map screen
	ui.stackedWidget->setCurrentIndex(1);
	HandleTabButtons(ui);
}
void quadrasoftware::on_tabButton3_clicked()
{
	// set panel to map screen
	ui.stackedWidget->setCurrentIndex(2);
	HandleTabButtons(ui);
}