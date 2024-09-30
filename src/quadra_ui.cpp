#include "quadra_ui.h"
#include "QuadraMavInterface.h"
#include <windows.h>
#include "connectDialog.h"
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

	ui.connectButton->setIcon(QIcon("images/disconnected.png"));
	ui.connectButton->setIconSize(QSize(70, 70));

	static bool init = false;
	auto timer = new QTimer(parent);

	// main loop, runs on every frame
	connect(timer, &QTimer::timeout, [this]
		{
			if (QuadraInterface.IsConnected())
			{
				ui.connectButton->setIcon(QIcon("images/connected.png"));
				ui.connectButton->setIconSize(QSize(70, 70));
			}
			else
			{
				ui.connectButton->setIcon(QIcon("images/disconnected.png"));
				ui.connectButton->setIconSize(QSize(70, 70));
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
			MessageBox(NULL, L"Connection Success", L"Success", MB_OK);
		}
		else
		{
			MessageBox(NULL, L"Connection Failed", L"Failed", MB_OK);
		}
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
		break;
	case 1: // when data page is active
		ui.tabButton1->setStyleSheet(deactiveStyleSheet);
		ui.tabButton2->setStyleSheet(activeStyleSheet);
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
	// fixed: crash when window closed
	QuadraInterface.Disconnect();
	exit(0);
	//QApplication::quit();
}

void quadrasoftware::on_tabButton2_clicked()
{
	// set panel to map screen
	ui.stackedWidget->setCurrentIndex(1);
	HandleTabButtons(ui);
}