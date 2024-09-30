#include "QuadraMavInterface.h"

bool QuadraMavInterface::Connect(std::string url)
{
	// send connection request
	auto connectionResult = mavsdk.add_any_connection_with_handle(url);
	system = mavsdk.first_autopilot(3.0);

	// check connection success
	if (connectionResult.first != mavsdk::ConnectionResult::Success || 
		!system || !system.value().get() || !system.value().get()->is_connected())
	{
		system.reset();
		return false;
	}

	// save handle for disconnection
	handle = connectionResult.second;
	return true;
}

bool QuadraMavInterface::Disconnect()
{
	if (!IsConnected())
		return false;

	mavsdk.remove_connection(handle);
	system.reset(); // fix: reset system object otherwise crashes
	return true;
}

bool QuadraMavInterface::IsConnected()
{
	// check system object is valid and connected
	if (system && system.value() && system.value().get()->is_connected())
	{
		return true;
	}
	else
		return false;
}
