#include "quadra_ui.h"
#include "QuadraMavInterface.h"
#include <windows.h>
#include "connectDialog.h"
#include "takeoffInputDialog.h"
#include <qtimer.h>

//
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
#include <qfuture.h>

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

	// map way point

	QString imagePath = "images/way-point.png";
	QString imagePathplane = "images/plane_icon.png";

	const Viewpoint viewpoint(37.78304072903069, 29.09632386893564, 50000);
	m_mapView->setViewpointAsync(viewpoint);

	static double latitude = 37.79262103901701;
	static double longitude = 28.96567444077879;

	auto graphicsOverlay = new GraphicsOverlay(m_mapView);

	PictureMarkerSymbol* waypoint = new PictureMarkerSymbol(QUrl::fromLocalFile(imagePath), this);
	waypoint->setWidth(30);
	waypoint->setHeight(30);

	Point point(longitude, latitude, SpatialReference::wgs84());
	Graphic* waypointPng = new Graphic(point, waypoint);

	static double latitude2 = 38.08300286281516;
	static double longitude2 = 29.39729159313304;

	PictureMarkerSymbol* plane = new PictureMarkerSymbol(QUrl::fromLocalFile(imagePathplane), this);
	plane->setWidth(60);
	plane->setHeight(60);
	plane->setAngle(0);

	Point point2(longitude2, latitude2, SpatialReference::wgs84());
	Graphic* ucakpng = new Graphic(point2, plane);

	Graphic* pointgraphic = new Graphic(new PictureMarkerSymbol(QUrl::fromLocalFile(""), this));
	graphicsOverlay->graphics()->append(pointgraphic);
	PolylineBuilder* polyline_builder = new PolylineBuilder(SpatialReference::wgs84(), this);

	polyline_builder->addPoint(longitude, latitude);
	polyline_builder->addPoint(longitude2, latitude2);
	SimpleLineSymbol* line_symbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(Qt::yellow), 3, this);
	Graphic* waypointLine = new Graphic(polyline_builder->toGeometry(), line_symbol, this);

	graphicsOverlay->graphics()->append(ucakpng);
	graphicsOverlay->graphics()->append(waypointPng);
	graphicsOverlay->graphics()->append(waypointLine);
	m_mapView->graphicsOverlays()->append(graphicsOverlay);


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
	ui.landingButton->setIconSize(QSize(75, 75));

	ui.takeoffButton->setIcon(QIcon("images/takeoff.png"));
	ui.takeoffButton->setIconSize(QSize(64, 64));

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
			}
			else
			{
				ui.connectButton->setIcon(QIcon("images/disconnected.png"));
				ui.connectButton->setIconSize(QSize(70, 70));

				ui.armButton->setText("DISARMED");
				ui.armButton->setStyleSheet("QPushButton{background-color: rgb(50, 0, 0);border-radius: 5px; border: 2px solid red;color: white;} QPushButton:pressed{ background-color: rgb(30, 0, 0); }");
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

		MessageBox(NULL, L"Disconnection Success", L"Success", MB_OK);
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
	if (!QuadraInterface.Land())
	{
		MessageBox(NULL, L"Error while landing", L"Error", MB_OK | MB_ICONERROR);
	}
}

// this functions is responsible from panel button effects
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