#include "assistant.h"
#include "quadra_ui.h"
#include "QuadraMavInterface.h"
#include <windows.h>
#include "connectDialog.h"
#include "takeoffInputDialog.h"
#include <qtimer.h>
#include <QCheckBox>
#include <QWidgetAction>
#include <QOpenGLWidget>
#include "gyroscope.hpp"
#include <thread>
#include <qclipboard.h>
#include <TextSymbol.h>
#include <iostream>
#include <qmessagebox.h>
#include "math.hpp"

using namespace Esri::ArcGISRuntime;
using namespace std::chrono;

quadrasoftware::quadrasoftware(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	AllocConsole();
	freopen("CONOUT$", "w", stdout);

	std::cout << "[!] Initializing Quadra Control System. Please wait . . ." << std::endl;

	Sleep(2000);
	system("pip install -r requirements.txt");

	ai_assistant = std::make_unique<PythonWrapper>();
	ShowWindow(GetConsoleWindow(), 0);

	// actuator outputs
	progressBar1 = new XQt::CircularProgressBar(this);
	progressBar2 = new XQt::CircularProgressBar(this);
	progressBar3 = new XQt::CircularProgressBar(this);
	progressBar4 = new XQt::CircularProgressBar(this);
	progressBar5 = new XQt::CircularProgressBar(this);

	progressBar1->setMaximumWidth(90);
	progressBar2->setMaximumWidth(90);
	progressBar3->setMaximumWidth(90);
	progressBar4->setMaximumWidth(90);
	progressBar5->setMaximumWidth(90);

	progressBar1->setMinimumHeight(70);
	progressBar2->setMinimumHeight(70);
	progressBar3->setMinimumHeight(70);
	progressBar4->setMinimumHeight(70);
	progressBar5->setMinimumHeight(70);

	ui.gridLayout->addWidget(progressBar1, 0, 0);
	ui.gridLayout->addWidget(progressBar2, 0, 2);
	ui.gridLayout->addWidget(progressBar3, 2, 0);
	ui.gridLayout->addWidget(progressBar4, 2, 2);
	ui.gridLayout->addWidget(progressBar5, 1, 1); // center

	// redirect console outputs to textedit (logs)
	OutputStream* outputStream = new OutputStream(ui.logFrame);
	std::streambuf* oldCoutBuffer = std::cout.rdbuf(outputStream);

	ui.waypointTable_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.waypointTable_2->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

	ui.paramTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.paramTable->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

	ui.settingsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.settingsTable->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

	ui.settingsTable->clearContents();
	ui.settingsTable->setRowCount(0);

	InitSettings();

	// UI QSS applies
	ui.horizontalFrame->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76;  background-color:#424242 ;}");
	ui.gridFrame->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76;  background-color:#424242 ;}");
	ui.frame_2->setStyleSheet("QFrame{border-radius: 12px; border: 0px solid #686D76;  background-color:#424242 ;}");
	ui.frame_3->setStyleSheet("QFrame{border-radius: 12px; border: 0px solid #686D76; background-color:#424242 ;}");
	ui.stackedWidget->setStyleSheet("QFrame{border-radius: 12px; border: 1px solid #686D76; background-color:#424242 ;}");

	// map api key
	const QString esriApiKey = QString("your-api-key");
	ArcGISRuntimeEnvironment::setApiKey(esriApiKey);

	// map object 
	m_map = new Map(BasemapStyle::OsmDarkGray, this);
	m_mapView = new MapGraphicsView(ui.frame_2);
	m_mapView->setMap(m_map);

	// for making the map responsive
	QSizePolicy sizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
	sizePolicy.setHeightForWidth(m_mapView->sizePolicy().hasHeightForWidth());
	m_mapView->setSizePolicy(sizePolicy);
	ui.gridLayout_3->addWidget(m_mapView);

	// map custom context menu (right-click menu)
	m_mapView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	connect(m_mapView, &MapGraphicsView::customContextMenuRequested, this, &quadrasoftware::mapContextMenuRequest);

	// init arcgis map graphics overlay
	map_graphicsOverlay = new GraphicsOverlay(m_mapView);
	m_mapView->graphicsOverlays()->append(map_graphicsOverlay);

	// create map 2 (mission map)
	m_map2 = new Map(BasemapStyle::ArcGISStreetsNight, this);
	m_mapView2 = new MapGraphicsView(ui.frame_4);
	m_mapView2->setMap(m_map2);
	ui.verticalLayout_10->addWidget(m_mapView2);

	map_graphicsOverlay2 = new GraphicsOverlay(m_mapView2);
	m_mapView2->graphicsOverlays()->append(map_graphicsOverlay2);

	sizePolicy.setHeightForWidth(m_mapView2->sizePolicy().hasHeightForWidth());
	m_mapView2->setSizePolicy(sizePolicy);

	m_mapView2->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	connect(m_mapView2, &MapGraphicsView::customContextMenuRequested, this, &quadrasoftware::map2ContextMenuRequest);

	// compass indicator
	QGraphicsScene* compassIndicatorScene = new QGraphicsScene(this);
	compassIndicatorScene->addPixmap(QPixmap("images/compass.png").scaled(220, 220));
	ui.compassIndicator->setScene(compassIndicatorScene);

	// compass plane icon
	QGraphicsScene* compassPlaneIndicatorScene = new QGraphicsScene(this);
	compassPlaneIndicatorScene->addPixmap(QPixmap("images/plane_icon.png").scaled(101, 121));
	ui.compassPlane->setScene(compassPlaneIndicatorScene);
	// set render options for better quality
	ui.compassPlane->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, false);
	ui.compassPlane->setRenderHint(QPainter::Antialiasing, true);
	ui.compassPlane->setRenderHint(QPainter::SmoothPixmapTransform, true);

	// gyroscope indicator
	QGraphicsScene* gyroscopeIndicatorScene = new QGraphicsScene(this);
	gyroscopeIndicatorScene->addPixmap(QPixmap("images/gyroscope.png"));
	ui.gyroscopeIndicator->setScene(gyroscopeIndicatorScene);
	// set render options for better quality
	ui.gyroscopeIndicator->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, false);
	ui.gyroscopeIndicator->setRenderHint(QPainter::Antialiasing, true);
	ui.gyroscopeIndicator->setRenderHint(QPainter::SmoothPixmapTransform, true);

	// pitch indicator
	PitchIndicator* pitchWidget = new PitchIndicator(ui.frame_5);
	pitchWidget->setGeometry(QRect(QPoint(40, 40), QSize(200, 180)));

	// buttons
	ui.tabButton1->setIcon(QIcon("images/map_icon.png"));
	ui.tabButton1->setIconSize(QSize(64, 64));

	ui.tabButton2->setIcon(QIcon("images/plane_icon.png"));
	ui.tabButton2->setIconSize(QSize(75, 75));

	ui.tabButton3->setIcon(QIcon("images/settings.png"));
	ui.tabButton3->setIconSize(QSize(75, 75));

	ui.landingButton->setIcon(QIcon("images/landing.png"));
	ui.landingButton->setIconSize(QSize(80, 80));

	ui.takeoffButton->setIcon(QIcon("images/takeoff.png"));
	ui.takeoffButton->setIconSize(QSize(64, 64));

	ui.vtolButton->setIcon(QIcon("images/drone.png"));
	ui.vtolButton->setIconSize(QSize(57, 57));

	ui.shutdownBtn->setIcon(QIcon("images/shutdown-icon.png"));
	ui.shutdownBtn->setIconSize(QSize(53, 53));
	ui.shutdownBtn->setToolTip("Shutdown system");

	ui.rebootBtn->setIcon(QIcon("images/reboot-icon.png"));
	ui.rebootBtn->setIconSize(QSize(53, 53));
	ui.rebootBtn->setToolTip("Reboot System");

	ui.holdButton->setIcon(QIcon("images/hold-icon.png"));
	ui.holdButton->setIconSize(QSize(53, 53));
	ui.holdButton->setToolTip("Hold");

	ui.rtlButton->setIcon(QIcon("images/home-icon.png"));
	ui.rtlButton->setIconSize(QSize(53, 53));
	ui.rtlButton->setToolTip("Return to Launch");

	ui.aiButton->setIcon(QIcon("images/microphone-icon.png"));
	ui.aiButton->setIconSize(QSize(50, 50));

	static Graphic* mission_plane_graphic = CreatePictureMarkerSymbolGraphic("images/plane_icon_green.png", 50, 50, 0, Point(), m_mapView2);
	map_graphicsOverlay2->graphics()->append(mission_plane_graphic);

	static bool init = false;
	auto timer = new QTimer(parent);

	// main loop, runs on every frame
	connect(timer, &QTimer::timeout, [this, pitchWidget]
		{
			for (auto system : systems)
			{
				if (!system.second->IsConnected())
				{
					ui.systemsComboBox->removeItem(
						ui.systemsComboBox->findText(
							QString::fromStdString(
								system.second->GetConnectionUrl()
							)
						)
					);

					system.second->DisconnectForce();
					systems.erase(system.second->GetConnectionUrl());
				}
				else
				{
					if (ui.systemsComboBox->findText(QString::fromStdString(
						system.second->GetConnectionUrl()
					)) == -1)
					{
						ui.systemsComboBox->addItem(
							QString::fromStdString(
								system.second->GetConnectionUrl()));

						ui.systemsComboBox->setCurrentIndex(
							ui.systemsComboBox->findText(
								QString::fromStdString(
									system.second->GetConnectionUrl())));
					}
				}
			}

			if (systems.size() > 0)
			{
				int nosystem_item = ui.systemsComboBox->findText("No System");
				if (nosystem_item != -1)
					ui.systemsComboBox->removeItem(nosystem_item);

				QuadraInterface = systems[ui.systemsComboBox->currentText().toStdString()];

				if (systems.size() != uav_graphics.size() || refresh_map)
				{
					refresh_map = false;
					uav_graphics.clear();
					map_graphicsOverlay->graphics()->clear();
					for (auto system : systems)
					{
						if (!system.second->IsConnected())
							continue;

						Graphic* waypointGraphic = CreatePictureMarkerSymbolGraphic("images/way-point.png", 30, 30, 0, Point(), m_mapView);
						Graphic* lineGraphic = CreatePolylineSymbolGraphic(Point(), Point(), new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(Qt::white), 3, this));
						Graphic* planeGraphic = nullptr;

						switch (system.second->GetVtolState())
						{
						case Telemetry::VtolState::Fw:
							if (system.second == QuadraInterface)
								planeGraphic = CreatePictureMarkerSymbolGraphic("images/plane_icon_green.png", 50, 50, 0, Point(), m_mapView);
							else
								planeGraphic = CreatePictureMarkerSymbolGraphic("images/plane_icon.png", 50, 50, 0, Point(), m_mapView);
							break;
						case Telemetry::VtolState::TransitionToFw:
							if (system.second == QuadraInterface)
								planeGraphic = CreatePictureMarkerSymbolGraphic("images/plane_icon_green.png", 50, 50, 0, Point(), m_mapView);
							else
								planeGraphic = CreatePictureMarkerSymbolGraphic("images/plane_icon.png", 50, 50, 0, Point(), m_mapView);
							break;
						case Telemetry::VtolState::Mc:
							if (system.second == QuadraInterface)
								planeGraphic = CreatePictureMarkerSymbolGraphic("images/drone_green.png", 30, 30, 0, Point(), m_mapView);
							else
								planeGraphic = CreatePictureMarkerSymbolGraphic("images/drone.png", 30, 30, 0, Point(), m_mapView);
							break;
						case Telemetry::VtolState::TransitionToMc:
							if (system.second == QuadraInterface)
								planeGraphic = CreatePictureMarkerSymbolGraphic("images/drone_green.png", 30, 30, 0, Point(), m_mapView);
							else
								planeGraphic = CreatePictureMarkerSymbolGraphic("images/drone.png", 30, 30, 0, Point(), m_mapView);
							break;
						default:
							planeGraphic = CreatePictureMarkerSymbolGraphic("images/plane_icon.png", 50, 50, 0, Point(), m_mapView);
							break;
						}

						UavGraphics uav_graphic(system.second->GetConnectionUrl(),
							waypointGraphic,
							planeGraphic,
							lineGraphic);

						uav_graphics.push_back(uav_graphic);

						map_graphicsOverlay->graphics()->append(planeGraphic);
						map_graphicsOverlay->graphics()->append(lineGraphic);
						map_graphicsOverlay->graphics()->append(waypointGraphic);
					}
				}

				for (auto uav_graphic : uav_graphics)
				{
					auto system = systems[uav_graphic.connection_url];
					if (!system->IsConnected())
						continue;

					Telemetry::Position temp_position = system->GetPosition();
					Telemetry::Position temp_targetPosition = system->GetTargetPosition();
					Telemetry::EulerAngle temp_angles = system->GetAngles();

					Graphic* planeGraphic = uav_graphic.planeGraphic;
					Graphic* waypointGraphic = uav_graphic.waypointGraphic;
					Graphic* targetLineGraphic = uav_graphic.targetLineGraphic;

					PictureMarkerSymbol* newPlaneSymbol = reinterpret_cast<PictureMarkerSymbol*>(planeGraphic->symbol());
					newPlaneSymbol->setAngle(temp_angles.yaw_deg);
					planeGraphic->setGeometry(Point(temp_position.longitude_deg, temp_position.latitude_deg, SpatialReference::wgs84()));
					planeGraphic->setSymbol(newPlaneSymbol);

					PolylineBuilder* lineSymbol_builder = new PolylineBuilder(SpatialReference::wgs84(), this);
					lineSymbol_builder->addPoint(temp_position.longitude_deg, temp_position.latitude_deg);
					lineSymbol_builder->addPoint(temp_targetPosition.longitude_deg, temp_targetPosition.latitude_deg);

					PictureMarkerSymbol* newWaypointSymbol = reinterpret_cast<PictureMarkerSymbol*>(waypointGraphic->symbol());
					waypointGraphic->setSymbol(newWaypointSymbol);

					double proximityLimit = 0.0002000;
					if (fabs(temp_position.latitude_deg - temp_targetPosition.latitude_deg) > proximityLimit ||
						fabs(temp_position.longitude_deg - temp_targetPosition.longitude_deg) > proximityLimit)
					{
						waypointGraphic->setGeometry(Point(temp_targetPosition.longitude_deg, temp_targetPosition.latitude_deg, SpatialReference::wgs84()));
						targetLineGraphic->setGeometry(lineSymbol_builder->toGeometry());
					}
					else
					{
						targetLineGraphic->setGeometry(Point());
						waypointGraphic->setGeometry(Point());
					}

					delete newWaypointSymbol;
					delete lineSymbol_builder;
					delete newPlaneSymbol;
				}

				if (true)
				{
					if (QuadraInterface && QuadraInterface->IsConnected())
					{
						HandleIndicators();

						if (refresh_mission)
						{
							refresh_mission = false;
							RefreshMission();
						}

						if (refresh_params)
						{
							RefreshParams();
							refresh_params = false;
						}

						// update mission map plane graphic
						PictureMarkerSymbol* newPlaneSymbol = reinterpret_cast<PictureMarkerSymbol*>(mission_plane_graphic->symbol());
						newPlaneSymbol->setAngle(QuadraInterface->GetAngles().yaw_deg);
						mission_plane_graphic->setGeometry(Point(QuadraInterface->GetPosition().longitude_deg, QuadraInterface->GetPosition().latitude_deg,
							SpatialReference::wgs84()));
						mission_plane_graphic->setSymbol(newPlaneSymbol);
						delete newPlaneSymbol;

						// update mission map and waypoints
						if (refresh_mission_map)
						{
							refresh_mission_map = false;
							for (auto item : mission_graphics)
							{
								for (int i = map_graphicsOverlay2->graphics()->size() - 1; i >= 0; i--)
								{
									if (map_graphicsOverlay2->graphics()->at(i) != nullptr &&
										item.line_graphic == map_graphicsOverlay2->graphics()->at(i) ||
										item.circle_graphic == map_graphicsOverlay2->graphics()->at(i) ||
										item.text_graphic == map_graphicsOverlay2->graphics()->at(i))
									{
										map_graphicsOverlay2->graphics()->removeAt(i);
									}
								}
							}
							mission_graphics.clear();

							auto current_position = QuadraInterface->GetHome();
							auto local_mission = ParseMission(GetLocalMission());
							for (int i = 0; i < local_mission.size(); i++)
							{
								auto item = local_mission[i];
								int index = i + 1;

								RenderWaypoint(item, current_position, index);
							}

							for (auto item : mission_graphics)
							{
								if (item.line_graphic == nullptr || item.circle_graphic == nullptr)
									continue;

								map_graphicsOverlay2->graphics()->append(item.line_graphic);
								map_graphicsOverlay2->graphics()->append(item.circle_graphic);
								map_graphicsOverlay2->graphics()->append(item.text_graphic);
							}
						}

						ui.connectButton->setIcon(QIcon("images/connected.png"));
						ui.connectButton->setIconSize(QSize(70, 70));

						// check arm state and update ui
						if (QuadraInterface->IsArmed())
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
						Telemetry::VtolState vtolState = QuadraInterface->GetVtolState();
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

						// get data
						Telemetry::Position position = QuadraInterface->GetPosition();
						Telemetry::Position targetPosition = QuadraInterface->GetTargetPosition();
						Telemetry::EulerAngle angles = QuadraInterface->GetAngles();

						// handle gyroscope
						pitchWidget->setPitchValue(angles.pitch_deg);
						pitchWidget->setRollValue(angles.roll_deg);
						ui.gyroscopeIndicator->resetTransform();
						ui.gyroscopeIndicator->rotate(angles.roll_deg);

						// handle compass
						ui.compassIndicator->resetTransform();
						ui.compassIndicator->rotate(-angles.yaw_deg);

						// track the plane on map
						static auto lastExecutionTime = steady_clock::now();
						if (bTrackPlane && !std::isnan(position.longitude_deg) && !std::isnan(position.latitude_deg))
						{
							auto now = steady_clock::now();
							auto duration = duration_cast<milliseconds>(now - lastExecutionTime);

							if (duration.count() >= 50) // in every 50 milliseconds
							{
								const Viewpoint viewpoint(Point(position.longitude_deg, position.latitude_deg, SpatialReference::wgs84()), 5000);
								m_mapView->setViewpointAsync(viewpoint, 0);
								lastExecutionTime = now;
							}
						}
					}
				}
			}
			else
			{
				QuadraInterface = nullptr;

				uav_graphics.clear();
				map_graphicsOverlay->graphics()->clear();

				ui.connectButton->setIcon(QIcon("images/disconnected.png"));
				ui.connectButton->setIconSize(QSize(70, 70));

				ui.armButton->setText("DISARMED");
				ui.armButton->setStyleSheet("QPushButton{background-color: rgb(50, 0, 0);border-radius: 5px; border: 2px solid red;color: white;} QPushButton:pressed{ background-color: rgb(30, 0, 0); }");

				if (ui.systemsComboBox->findText("No System") == -1)
					ui.systemsComboBox->addItem("No System");
			}

		});
	timer->start();
}

quadrasoftware::~quadrasoftware()
{}

void quadrasoftware::on_connectButton_clicked()
{
	if (QuadraInterface && QuadraInterface->IsConnected())
	{
		ui.systemsComboBox->removeItem(
			ui.systemsComboBox->findText(
				QString::fromStdString(
					QuadraInterface->GetConnectionUrl())));

		QuadraInterface->Disconnect();
		systems.erase(QuadraInterface->GetConnectionUrl());
	}
	else
	{
		QuadraMavInterface* new_system = new QuadraMavInterface([]() {
			QApplication::processEvents();
			});

		connectDialog conDialog;
		int result = conDialog.exec();

		if (result != QDialog::Accepted)
			return;

		if (!new_system->Connect(conDialog.getConnectionURL().toStdString()))
		{
			MessageBox(NULL, L"Connection Failed", L"Failed", MB_OK);
			delete new_system;
			return;
		}

		systems[new_system->GetConnectionUrl()] = new_system;

		ui.systemsComboBox->addItem(
			QString::fromStdString(
				new_system->GetConnectionUrl()));

		ui.systemsComboBox->setCurrentIndex(
			ui.systemsComboBox->findText(
				QString::fromStdString(
					new_system->GetConnectionUrl())));

		QuadraInterface = new_system;
		RefreshMission();
	}
}

void quadrasoftware::on_connectButton_2_clicked()
{
	QuadraMavInterface* new_system = new QuadraMavInterface([]() {
		QApplication::processEvents();
		});

	connectDialog conDialog;
	int result = conDialog.exec();

	if (result != QDialog::Accepted)
		return;

	if (!new_system->Connect(conDialog.getConnectionURL().toStdString()))
	{
		MessageBox(NULL, L"Connection Failed", L"Failed", MB_OK);
		delete new_system;
		return;
	}

	systems[new_system->GetConnectionUrl()] = new_system;

	ui.systemsComboBox->addItem(
		QString::fromStdString(
			new_system->GetConnectionUrl()));

	ui.systemsComboBox->setCurrentIndex(
		ui.systemsComboBox->findText(
			QString::fromStdString(
				new_system->GetConnectionUrl())));
}

void quadrasoftware::on_armButton_clicked()
{
	if (!QuadraInterface->IsConnected())
	{
		MessageBox(NULL, L"Connect to PX4 first!", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	if (QuadraInterface->IsArmed())
	{
		if (!QuadraInterface->Disarm())
		{
			MessageBox(NULL, L"Error while disarm", L"Error", MB_ICONERROR);
			return;
		}
	}
	else
	{
		if (!QuadraInterface->Arm())
		{
			MessageBox(NULL, L"Error while arm", L"Error", MB_ICONERROR);
			return;
		}
	}
}

void quadrasoftware::on_takeoffButton_clicked()
{
	if (!QuadraInterface->IsConnected())
	{
		MessageBox(NULL, L"Connect to PX4 first!", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	if (!QuadraInterface->IsArmed())
	{
		MessageBox(NULL, L"Arm your vehicle first!", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	// open input dialog here
	takeoffInputDialog inputDlg;
	auto result = inputDlg.exec();

	if (result != QDialog::Accepted)
		return;

	if (!QuadraInterface->Takeoff(inputDlg.GetAltitude()))
	{
		MessageBox(NULL, L"Error while takeoff", L"Error", MB_OK | MB_ICONERROR);
	}
}

void quadrasoftware::on_landingButton_clicked()
{
	if (!QuadraInterface->IsConnected())
	{
		MessageBox(NULL, L"Connect to PX4 first!", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	if (!QuadraInterface->Land())
	{
		MessageBox(NULL, L"Error while landing", L"Error", MB_OK | MB_ICONERROR);
	}
}

void quadrasoftware::on_startMissionBtn_clicked()
{
	bool result = QuadraInterface->StartMission();
}

void quadrasoftware::on_deleteMissionBtn_clicked()
{
	QModelIndexList selectedIndexes = ui.waypointTable_2->selectionModel()->selectedRows();
	for (const QModelIndex& index : selectedIndexes) {
		int row = index.row();
		ui.waypointTable_2->removeRow(row);
	}
	refresh_mission_map = true;
}

void quadrasoftware::on_refreshMissionBtn_clicked()
{
	RefreshMission();
}

void quadrasoftware::on_clearMissionBtn_clicked()
{
	ClearMission();
}

void quadrasoftware::on_uploadMissionBtn_clicked()
{
	UploadMission();
}

void quadrasoftware::on_documentsBtn_clicked()
{
	system("start https://mavlink.io/en/messages/common.html");
}

void quadrasoftware::on_vtolButton_clicked()
{
	if (!QuadraInterface->IsConnected())
	{
		MessageBox(NULL, CONNECTION_ERROR_MSG, L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	Telemetry::VtolState vtolState = QuadraInterface->GetVtolState();
	if (vtolState == Telemetry::VtolState::Mc || vtolState == Telemetry::VtolState::TransitionToMc)
	{
		if (!QuadraInterface->TransitionToFixedwing())
			MessageBox(NULL, TRANSITION_FW_ERROR_MSG, L"Error", MB_ICONERROR);
	}
	else if (vtolState == Telemetry::VtolState::Fw || vtolState == Telemetry::VtolState::TransitionToFw)
	{
		if (!QuadraInterface->TransitionToDrone())
			MessageBox(NULL, TRANSITION_MC_ERROR_MSG, L"Error", MB_ICONERROR);
	}
}

void quadrasoftware::on_holdButton_clicked()
{
	if (!QuadraInterface->IsConnected())
	{
		MessageBox(NULL, CONNECTION_ERROR_MSG, L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	if (!QuadraInterface->Hold())
		MessageBox(NULL, HOLD_ERROR_MSG, L"Error", MB_OK | MB_ICONERROR);
}

void quadrasoftware::on_rtlButton_clicked()
{
	if (!QuadraInterface->IsConnected())
	{
		MessageBox(NULL, CONNECTION_ERROR_MSG, L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	if (!QuadraInterface->RTL())
		MessageBox(NULL, RTL_ERROR_MSG, L"Error", MB_OK | MB_ICONERROR);
}

void quadrasoftware::on_shutdownBtn_clicked()
{
	if (!QuadraInterface->IsConnected())
	{
		MessageBox(NULL, CONNECTION_ERROR_MSG, L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	if (!QuadraInterface->Shutdown())
		MessageBox(NULL, SHUTDOWN_ERROR_MSG, L"Error", MB_OK | MB_ICONERROR);
}

void quadrasoftware::on_rebootBtn_clicked()
{
	if (!QuadraInterface->IsConnected())
	{
		MessageBox(NULL, CONNECTION_ERROR_MSG, L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	if (!QuadraInterface->Reboot())
		MessageBox(NULL, REBOOT_ERROR_MSG, L"Error", MB_OK | MB_ICONERROR);
}

void quadrasoftware::on_aiButton_clicked()
{
	if (QuadraInterface && QuadraInterface->IsConnected())
		AiAssistant();
}

void quadrasoftware::on_logFrame_textChanged()
{
	ui.logFrame->verticalScrollBar()->setValue(ui.logFrame->verticalScrollBar()->maximum());
}

void quadrasoftware::on_paramFilter_textChanged()
{
	for (int row = 0; row < ui.paramTable->rowCount(); ++row)
	{
		bool rowMatches = false;

		QTableWidgetItem* item = ui.paramTable->item(row, 0);
		if (item && item->text().contains(ui.paramFilter->toPlainText(), Qt::CaseInsensitive))
			ui.paramTable->setRowHidden(row, false);
		else
			ui.paramTable->setRowHidden(row, true);
	}
}

void quadrasoftware::on_settingsTable_itemChanged(QTableWidgetItem* item)
{
	if (refresh_settings)
		return;

	int row = item->row();
	std::string setting_name = ui.settingsTable->item(row, 0)->text().toStdString();
	std::string setting_value = ui.settingsTable->item(row, 1)->text().toStdString();

	for (auto& setting : settings)
		if (setting->get_name() == setting_name)
			setting->set_value(setting_value);
}

void quadrasoftware::on_paramTable_itemChanged(QTableWidgetItem* item)
{
	if (refresh_params)
		return;

	auto param_type = ui.paramTable->item(item->row(), 2);
	if (param_type->text() == "Float")
	{
		if (!QuadraInterface->SetParamFloat(
			ui.paramTable->item(item->row(), 0)->text().toStdString(),
			ui.paramTable->item(item->row(), 1)->text().toFloat()))
		{
			MessageBox(NULL, L"Cannot set param", L"Error", MB_ICONERROR);
			return;
		}
	}
	else if (param_type->text() == "Int")
	{
		if (!QuadraInterface->SetParamInt(
			ui.paramTable->item(item->row(), 0)->text().toStdString(),
			ui.paramTable->item(item->row(), 1)->text().toInt()))
		{
			MessageBox(NULL, L"Cannot set param", L"Error", MB_ICONERROR);
			return;
		}
	}
}

void quadrasoftware::on_systemsComboBox_currentIndexChanged(int index)
{
	refresh_map = true;
	if (ui.systemsComboBox->itemText(index) != "No System")
	{
		refresh_mission = true;
		refresh_params = true;
	}
}

void quadrasoftware::on_waypointTable_2_itemChanged(QTableWidgetItem* item)
{
	refresh_mission_map = true;
}

void quadrasoftware::map2ContextMenuRequest(QPoint pos)
{
	QMenu* menu = new QMenu(m_mapView2);

	Point location = Point(GeometryEngine::project(m_mapView2->screenToLocation(pos.x(), pos.y()),
		Esri::ArcGISRuntime::SpatialReference::wgs84()));

	QAction* reset_action = new QAction("Go to UAV", menu);
	connect(reset_action, &QAction::triggered, [this]()
		{
			if (QuadraInterface->IsConnected())
			{
				auto position = QuadraInterface->GetPosition();
				const Viewpoint viewpoint(Point(position.longitude_deg, position.latitude_deg, SpatialReference::wgs84()), 5000);
				m_mapView2->setViewpointAsync(viewpoint, 0);
			}
		});
	menu->addAction(reset_action);

	QAction* mission_vtol_takeoff_item = new QAction("VTOL Takeoff", menu);
	connect(mission_vtol_takeoff_item, &QAction::triggered, [this, location_ = location]()
		{
			if (QuadraInterface->IsConnected())
			{
				AddMissionItem(QuadraMissionItem("VTOL_TAKEOFF", location_.y(), location_.x(), 50.0f, 0,
					VTOL_TRANSITION_HEADING_NEXT_WAYPOINT));
			}
		});
	menu->addAction(mission_vtol_takeoff_item);

	QAction* mission_takeoff_item = new QAction("Takeoff", menu);
	connect(mission_takeoff_item, &QAction::triggered, [this, location_ = location]()
		{
			if (QuadraInterface->IsConnected())
			{
				AddMissionItem(QuadraMissionItem("TAKEOFF", location_.y(), location_.x(), 50.0f, 0,
					VTOL_TRANSITION_HEADING_NEXT_WAYPOINT));
			}
		});
	menu->addAction(mission_takeoff_item);

	QAction* mission_waypoint_item = new QAction("Waypoint", menu);
	connect(mission_waypoint_item, &QAction::triggered, [this, location_ = location]()
		{
			if (QuadraInterface->IsConnected())
			{
				AddMissionItem(QuadraMissionItem("WAYPOINT", location_.y(), location_.x(), 50.0f));
			}
		});
	menu->addAction(mission_waypoint_item);

	QAction* mission_landing_item = new QAction("Land", menu);
	connect(mission_landing_item, &QAction::triggered, [this, location_ = location]()
		{
			if (QuadraInterface->IsConnected())
			{
				AddMissionItem(QuadraMissionItem("LAND", location_.y(), location_.x()));
			}
		});
	menu->addAction(mission_landing_item);

	menu->popup(m_mapView2->mapToGlobal(pos));
}

void quadrasoftware::mapContextMenuRequest(QPoint pos)
{
	QMenu* menu = new QMenu(m_mapView);
	// go to location sub menu
	QMenu* goToLocation_menu = new QMenu("Go To Here", m_mapView);

	Point location = Point(GeometryEngine::project(m_mapView->screenToLocation(pos.x(), pos.y()),
		Esri::ArcGISRuntime::SpatialReference::wgs84()));

	// in drone mode
	QAction* goToLocationDrone_action = new QAction("Drone", goToLocation_menu);
	connect(goToLocationDrone_action, &QAction::triggered, [this, location]() {
		std::thread thr(
			[this, location, system = QuadraInterface]() {
				HANDLE_ERRORS(system->TransitionToDrone(), TRANSITION_MC_ERROR_MSG);
				HANDLE_ERRORS(system->GoToLocation(location.y(), location.x(), system->GetPosition().absolute_altitude_m),
					GOTO_LOCATION_ERROR_MSG);
			});
		thr.detach();
		});
	goToLocation_menu->addAction(goToLocationDrone_action);

	//in vtol mode
	QAction* goToLocationVtol_action = new QAction("VTOL", goToLocation_menu);
	connect(goToLocationVtol_action, &QAction::triggered, [this, location_ = location]() {
		std::thread thr(
			[this, location__ = location_, system = QuadraInterface]() {
				HANDLE_ERRORS(system->TransitionToFixedwing(), TRANSITION_FW_ERROR_MSG);
				HANDLE_ERRORS(system->GoToLocation(location__.y(), location__.x(), system->GetPosition().absolute_altitude_m),
					GOTO_LOCATION_ERROR_MSG);
				HANDLE_ERRORS(system->TransitionToDrone(), TRANSITION_MC_ERROR_MSG);
			});
		thr.detach();
		});
	goToLocation_menu->addAction(goToLocationVtol_action);
	menu->addMenu(goToLocation_menu);


	// IMPORTANT: still in developemnt, dont use it !!! 
	// needs better and proper steering and positioning algorithm
	// for now, it uses missions as a method. but it doesnt meet the requirements...
	QAction* fleet_flight_action = new QAction("Formation Flight", menu);
	connect(fleet_flight_action, &QAction::triggered, [this, location_ = location]() {
		std::thread thr(
			[this, location__ = location_]()
			{
				QuadraInterface->Arm();
				Sleep(3000);
				QuadraInterface->Takeoff(5);

				auto position = QuadraInterface->GetPosition();
				auto angles = QuadraInterface->GetAngles();
				float new_yaw = CalcYaw(position.latitude_deg, position.longitude_deg, location__.y(), location__.x());
				double yaw_radians = toRadians(new_yaw);

				QuadraInterface->GoToLocation(position.latitude_deg, position.longitude_deg, position.absolute_altitude_m, 0.001, new_yaw);

				std::vector<QuadraMissionItem> leader_mission;

				auto [leader_vtol_takeoff_lat, leader_vtol_takeoff_lon] = FormationPosCalculation(
					position.latitude_deg,
					position.longitude_deg,
					yaw_radians,
					-0.00005,
					0,
					0
				);

				auto [leader_waypoint_lat, leader_waypoint_lon] = FormationPosCalculation(
					location__.y(),
					location__.x(),
					yaw_radians,
					0.00005, // forward calculation
					0,
					0
				);

				leader_mission.push_back(QuadraMissionItem("VTOL_TAKEOFF", leader_vtol_takeoff_lat, leader_vtol_takeoff_lon, 40.0f, 0,
					VTOL_TRANSITION_HEADING_NEXT_WAYPOINT, 0, NAN, false));
				leader_mission.push_back(QuadraMissionItem("WAYPOINT", leader_waypoint_lat, leader_waypoint_lon, 40.0f));
				leader_mission.push_back(QuadraMissionItem("LAND", location__.y(), location__.x(), 0));

				QuadraInterface->ClearMission();
				QuadraInterface->UploadMission(ParseMission(leader_mission));

				double base_distance = 0.00003;
				double spread_distance = 0.00003;

				int i = 0;
				for (auto& system : systems)
				{
					if (system.second == QuadraInterface)
						continue;

					system.second->Arm();
					Sleep(3000);
					system.second->Takeoff(position.relative_altitude_m);

					auto [new_lat, new_lon] = FormationPosCalculation(
						position.latitude_deg,
						position.longitude_deg,
						yaw_radians,
						base_distance,
						spread_distance,
						i
					);

					system.second->GoToLocation(new_lat, new_lon, position.absolute_altitude_m, 0.00002, new_yaw);

					auto [target_lat, target_lon] = FormationPosCalculation(
						location__.y(),
						location__.x(),
						yaw_radians,
						base_distance,
						spread_distance,
						i
					);

					auto [vtol_takeoff_lat, vtol_takeoff_lon] = FormationPosCalculation(
						new_lat,
						new_lon,
						yaw_radians,
						-0.00005,
						0,
						0
					);

					auto [waypoint_lat, waypoint_lon] = FormationPosCalculation(
						target_lat,
						target_lon,
						yaw_radians,
						0.00005, // forward calculation
						0,
						0
					);

					std::vector<QuadraMissionItem> fleet_mission;
					fleet_mission.push_back(QuadraMissionItem("VTOL_TAKEOFF", vtol_takeoff_lat, vtol_takeoff_lon, 40.0f, 0,
						VTOL_TRANSITION_HEADING_NEXT_WAYPOINT, 0, NAN, false));
					fleet_mission.push_back(QuadraMissionItem("WAYPOINT", waypoint_lat, waypoint_lon, 40.0f));
					fleet_mission.push_back(QuadraMissionItem("LAND", target_lat, target_lon, 0));

					system.second->ClearMission();
					system.second->UploadMission(ParseMission(fleet_mission));

					i++;
				}

				Sleep(3000);

				QuadraInterface->StartMission();

				for (auto& system : systems)
				{
					if (system.second != QuadraInterface)
						system.second->StartMission();
				}

				while (QuadraInterface->GetLandedState() != Telemetry::LandedState::Landing)
				{
					for (auto& system : systems)
					{
						system.second->test();
					}
				}
			});
		thr.detach();
		});
	menu->addAction(fleet_flight_action);

	// create checkbox object
	QAction* trackPlane_action = new QAction("Follow the plane", menu);
	trackPlane_action->setCheckable(true);
	trackPlane_action->setChecked(bTrackPlane);
	// set action value to our bool variable
	connect(trackPlane_action, &QAction::triggered, [this, trackPlane_action]() {
		bTrackPlane = trackPlane_action->isChecked();
		});
	menu->addAction(trackPlane_action);

	QAction* copyCoordinates_action = new QAction("Copy Coordinates", menu);
	connect(copyCoordinates_action, &QAction::triggered, [this, copyCoordinates_action, pos1 = pos]() {
		Point location = Point(GeometryEngine::project(m_mapView->screenToLocation(pos1.x(), pos1.y()),
			Esri::ArcGISRuntime::SpatialReference::wgs84()));

		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString::number(location.y()) + ", " + QString::number(location.x()));
		});
	menu->addAction(copyCoordinates_action);

	menu->popup(m_mapView->mapToGlobal(pos));
}

void quadrasoftware::setCursorViewpoint(const QPoint& pos)
{
	Point location = m_mapView->screenToLocation(pos.x(), pos.y());

	// create map way point
	const Viewpoint viewpoint(location, 50000);
	m_mapView->setViewpointAsync(viewpoint);
}

// this function is responsible from panel buttons
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

Graphic* quadrasoftware::CreatePictureMarkerSymbolGraphic(std::string imagePath, int width, int height, int angle, Point location = Point(0, 0, SpatialReference::wgs84()), MapGraphicsView* map = nullptr)
{
	PictureMarkerSymbol* waypoint = new PictureMarkerSymbol(QUrl::fromLocalFile(imagePath.c_str()), map);
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

void quadrasoftware::RefreshParams()
{
	ui.paramTable->clearContents();
	while (ui.paramTable->rowCount() > 0)
		ui.paramTable->removeRow(0);

	if (QuadraInterface && QuadraInterface->IsConnected())
	{
		auto params = QuadraInterface->GetAllParams();

		for (int i = 0; i < params.float_params.size(); i++)
		{
			int row = ui.paramTable->rowCount();
			ui.paramTable->insertRow(row);

			QTableWidgetItem* itemName = new QTableWidgetItem(QString::fromStdString(params.float_params[i].name));
			itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
			ui.paramTable->setItem(row, 0, itemName);

			QTableWidgetItem* itemValue = new QTableWidgetItem(QString::number(params.float_params[i].value));
			ui.paramTable->setItem(row, 1, itemValue); 

			QTableWidgetItem* itemType = new QTableWidgetItem("Float");
			itemType->setFlags(itemType->flags() & ~Qt::ItemIsEditable);
			ui.paramTable->setItem(row, 2, itemType);
		}

		for (int i = 0; i < params.int_params.size(); i++)
		{
			int row = ui.paramTable->rowCount();
			ui.paramTable->insertRow(row);

			QTableWidgetItem* itemName = new QTableWidgetItem(QString::fromStdString(params.int_params[i].name));
			itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
			ui.paramTable->setItem(row, 0, itemName);

			QTableWidgetItem* itemValue = new QTableWidgetItem(QString::number(params.int_params[i].value));
			ui.paramTable->setItem(row, 1, itemValue);

			QTableWidgetItem* itemType = new QTableWidgetItem("Int");
			itemType->setFlags(itemType->flags() & ~Qt::ItemIsEditable);
			ui.paramTable->setItem(row, 2, itemType);
		}
	}

}

void quadrasoftware::RenderWaypoint(MissionRaw::MissionItem mission_item, Telemetry::Position& current_position, int mission_index)
{
	MissionGraphics graphic;
	double target_lat = mission_item.y / 1e7;
	double target_lon = mission_item.x / 1e7;

	graphic.line_graphic =
		CreatePolylineSymbolGraphic(Point(), Point(), new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(Qt::darkYellow), 3, this));
	PolylineBuilder* lineSymbol_builder = new PolylineBuilder(SpatialReference::wgs84(), this);
	lineSymbol_builder->addPoint(current_position.longitude_deg, current_position.latitude_deg);
	lineSymbol_builder->addPoint(target_lat, target_lon);
	graphic.line_graphic->setGeometry(lineSymbol_builder->toGeometry());
	graphic.line_graphic->setZIndex(0);

	Point point(target_lat, target_lon, SpatialReference::wgs84());
	SimpleMarkerSymbol* circle_symbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Circle, QColor("green"), 15, m_mapView2);
	graphic.circle_graphic = new Graphic(point, circle_symbol, m_mapView2);
	graphic.circle_graphic->setZIndex(1);

	QString waypoint_number = QString::number(mission_index);
	TextSymbol* text_symbol = new TextSymbol(waypoint_number, QColor("white"), 12, HorizontalAlignment::Center, VerticalAlignment::Middle, this);
	graphic.text_graphic = new Graphic(point, text_symbol, this);
	graphic.text_graphic->setZIndex(2);

	current_position.latitude_deg = mission_item.x / 1e7;
	current_position.longitude_deg = mission_item.y / 1e7;
	mission_graphics.push_back(graphic);

	delete circle_symbol;
	delete lineSymbol_builder;
	delete text_symbol;
}

void quadrasoftware::closeEvent(QCloseEvent* event)
{
	// fixme: crash when window closed
	QuadraInterface->Disconnect();
	exit(0);
}

void quadrasoftware::on_tabButton1_clicked()
{
	// set panel to map screen
	ui.stackedWidget->setCurrentIndex(0);
	HandleTabButtons(ui);
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