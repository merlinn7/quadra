#pragma once
#include <pybind11/embed.h>
namespace py = pybind11;

#include <iostream>
#include "quadra_ui.h"


// cr: https://stackoverflow.com/a/75161457 thank you for saving me
class PythonWrapper
{
public:
	PythonWrapper() : m_interpreter()
	{
		py::object obj = py::module::import("ai_assistant").attr("main_flow");
		mp_gil_release = std::make_unique<py::gil_scoped_release>();
	}

	void Run()
	{
		py::gil_scoped_acquire acquire;
		py::tuple result = py::module::import("ai_assistant").attr("main_flow")().cast<py::tuple>();

		std::string command = result[0].cast<std::string>();
		int distance = result[1].cast<int>();

		std::cout << command << std::endl;
		if (distance != 0)
			std::cout << distance << std::endl;

		QuadraInterface->Arm();
		Sleep(1000);

		if (command == "takeoff")
		{
			QuadraInterface->Takeoff(distance);
		}
		else if (command == "landing")
		{
			QuadraInterface->Land();
		}
		else if (command == "transition")
		{
			auto state = QuadraInterface->GetVtolState();
			state == Telemetry::VtolState::Fw || state == Telemetry::VtolState::TransitionToFw ?
				QuadraInterface->TransitionToDrone() : QuadraInterface->TransitionToFixedwing();
		}

		mp_gil_release = std::make_unique<py::gil_scoped_release>();
	}
private:
	py::scoped_interpreter m_interpreter;

	std::unique_ptr<py::gil_scoped_release> mp_gil_release;
};
std::unique_ptr<PythonWrapper> ai_assistant;