#include "quadra_ui.h"

void quadrasoftware::InitSettings()
{
	// add settings here
	refresh_settings = true;
	settings.push_back(std::make_unique<Setting<int>>("Max RPM Value", &max_rpm));

	for (auto& setting : settings)
	{
		int row = ui.settingsTable->rowCount();
		ui.settingsTable->insertRow(row);

		ui.settingsTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(setting->get_name())));
		ui.settingsTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(setting->get_value())));
	}

	refresh_settings = false;
}