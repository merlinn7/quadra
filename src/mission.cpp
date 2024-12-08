#include "mission.h"

std::vector<MissionRaw::MissionItem> quadrasoftware::ParseMission(std::vector<QuadraMissionItem> mission)
{
	std::vector<MissionRaw::MissionItem> parsed_items;

	for (int i = 0; i < mission.size(); i++)
	{
		QuadraMissionItem item = mission[i];
		MissionRaw::MissionItem parsed_item;

		for (auto defined_mission_item : defined_mission_items)
		{
			if (item.cmd == defined_mission_item.quadra_mission)
				parsed_item.command = defined_mission_item.mavlink_mission;
		}

		parsed_item.x = static_cast<int32_t>(item.lat * 1e7);
		parsed_item.y = static_cast<int32_t>(item.lon * 1e7);
		parsed_item.z = item.alt;

		parsed_item.param1 = item.p1;
		parsed_item.param2 = item.p2;
		parsed_item.param3 = item.p3;
		parsed_item.param4 = item.p4;

		parsed_item.autocontinue = item.auto_continue;
		parsed_item.seq = i;
		parsed_item.frame = 3;

		if (i == 0)
			parsed_item.current = true;

		parsed_items.push_back(parsed_item);
	}

	return parsed_items;
}

std::vector<QuadraMissionItem> quadrasoftware::ParseMission(std::vector<MissionRaw::MissionItem> mission)
{
	std::vector<QuadraMissionItem> parsed_items;
	for (int i = 0; i < mission.size(); i++)
	{
		MissionRaw::MissionItem item = mission[i];
		QuadraMissionItem parsed_item;

		for (auto defined_mission_item : defined_mission_items)
		{
			if (item.command == defined_mission_item.mavlink_mission)
				parsed_item.cmd = defined_mission_item.quadra_mission;
		}

		// parse coordinates
		parsed_item.lat = static_cast<double>(item.x / 1e7);
		parsed_item.lon = static_cast<double>(item.y / 1e7);
		parsed_item.alt = item.z;

		parsed_item.p1 = item.param1;
		parsed_item.p2 = item.param2;
		parsed_item.p3 = item.param3;
		parsed_item.p4 = item.param4;

		parsed_item.auto_continue = item.autocontinue;

		parsed_items.push_back(parsed_item);
	}

	return parsed_items;
}

std::vector<QuadraMissionItem> quadrasoftware::GetLocalMission()
{
	std::vector<QuadraMissionItem> mission;

	for (int i = 0; i < ui.waypointTable_2->rowCount(); i++)
	{
		QuadraMissionItem item;
		item.cmd = ui.waypointTable_2->item(i, 0)->text().toStdString();
		item.lat = ui.waypointTable_2->item(i, 1)->text().toDouble();
		item.lon = ui.waypointTable_2->item(i, 2)->text().toDouble();
		item.alt = ui.waypointTable_2->item(i, 3)->text().toFloat();
		item.p1 = ui.waypointTable_2->item(i, 4)->text().toFloat();
		item.p2 = ui.waypointTable_2->item(i, 5)->text().toFloat();
		item.p3 = ui.waypointTable_2->item(i, 6)->text().toFloat();
		item.p4 = ui.waypointTable_2->item(i, 7)->text().toFloat();

		mission.push_back(item);
	}

	return mission;
}

void quadrasoftware::AddMissionItem(QuadraMissionItem item)
{
	int currentRow = ui.waypointTable_2->rowCount();
	ui.waypointTable_2->insertRow(currentRow);
	ui.waypointTable_2->setItem(currentRow, 0, new QTableWidgetItem(QString::fromStdString(item.cmd)));
	ui.waypointTable_2->setItem(currentRow, 1, new QTableWidgetItem(QString::number(item.lat, 'f', 6)));
	ui.waypointTable_2->setItem(currentRow, 2, new QTableWidgetItem(QString::number(item.lon, 'f', 6)));
	ui.waypointTable_2->setItem(currentRow, 3, new QTableWidgetItem(QString::number(item.alt, 'f', 1)));
	ui.waypointTable_2->setItem(currentRow, 4, new QTableWidgetItem(QString::number(item.p1, 'f', 1)));
	ui.waypointTable_2->setItem(currentRow, 5, new QTableWidgetItem(QString::number(item.p2, 'f', 1)));
	ui.waypointTable_2->setItem(currentRow, 6, new QTableWidgetItem(QString::number(item.p3, 'f', 1)));
	ui.waypointTable_2->setItem(currentRow, 7, new QTableWidgetItem(QString::number(item.p4, 'f', 1)));
}

// brain-dead compiler breaks this function, i had to disable optimizations..
#pragma optimize("", off)
void quadrasoftware::RefreshMission()
{
	if (!QuadraInterface->IsConnected())
	{
		return;
	}

	auto downloaded_mission = QuadraInterface->DownloadMission();
	if (!downloaded_mission.first)
	{
		MessageBox(NULL, DOWNLOAD_MISSION_ERROR_MSG, L"Error", MB_OK);
		return;
	}

	ui.waypointTable_2->clearContents();

	while (ui.waypointTable_2->rowCount() > 0)
		ui.waypointTable_2->removeRow(0);

	auto parsed_mission = ParseMission(downloaded_mission.second);

	for (int i = 0; i < parsed_mission.size(); i++)
	{
		auto item = parsed_mission[i];
		AddMissionItem(item);
	}

	refresh_mission_map = true;
}
#pragma optimize("", on)

void quadrasoftware::ClearMission()
{
	if (!QuadraInterface->IsConnected())
	{
		MessageBox(NULL, CONNECTION_ERROR_MSG, L"Error", MB_OK);
		return;
	}

	if (!QuadraInterface->ClearMission())
	{
		MessageBox(NULL, CLEAR_MISSION_ERROR_MSG, L"Error", MB_OK);
		return;
	}

	ui.waypointTable_2->clearContents();

	RefreshMission();
}

void quadrasoftware::UploadMission()
{
	if (!QuadraInterface->IsConnected())
	{
		MessageBox(NULL, CONNECTION_ERROR_MSG, L"Error", MB_OK);
		return;
	}

	auto local_mission = GetLocalMission();
	auto mission_to_upload = ParseMission(local_mission);

	if (!QuadraInterface->UploadMission(mission_to_upload))
	{
		MessageBox(NULL, UPLOAD_MISSION_ERROR_MSG, L"Error", MB_OK);
		return;
	}

	MessageBox(NULL, L"Uploaded mission succesfully", L"Success", MB_ICONINFORMATION);
}