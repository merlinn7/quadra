# Quadra Ground Control Software

**Quadra** is an open-source UAV ground control software developed in **C++** to serve as a **free guide, resource, and awareness tool** for UAV enthusiasts, students, and professionals. It aims to make advanced UAV control technology more accessible and transparent.

Quadra offers a modern and intuitive control experience for drones and VTOLs. It features AI-powered voice assistance and multi-UAV management capabilities, enabling users to experiment, learn, and innovate in the field of UAV technology.

Originally developed for the **Teknofest High School UAV Competition**, the project has since been reimagined as a community-focused, open-source initiative. The software has been **rewritten two times** to ensure better performance, maintainability, and scalability.

## About Me

I'm a student and **independent developer** focused on **UAV systems**, **low-level programming**, **reverse engineering**, and **cybersecurity**. I started working on Quadra for the **Teknofest High School UAV Competition**, but it soon evolved into a great passion.

My goal is to create an **open-source UAV control system** that can become a global standard. I believe in learning by doing, and Quadra is a result of that belief.

While developing this software, I struggled to find sufficient resources, so I developed Quadra — not just to overcome this challenge but to help others progress faster and further. My aim is to raise awareness about UAV technology and make it more accessible to everyone.

## Mission & Vision

The primary goal of Quadra is to provide:  
- **A Free Guide**: Empower users to learn about UAV control systems and development of ground control stations.
- **An Open Resource**: Provide an open-source tool for students, hobbyists, and developers.  
- **Awareness & Accessibility**: Increase awareness of UAV technology and make it more accessible to a wider audience.

By sharing this project, I hope to support innovation, experimentation, and education in UAV systems.

## Features

- **Mission Planning for VTOL & Drones**: Create, edit, and execute missions for both drones and VTOLs.  
- **One-Click Action Buttons**: Trigger critical commands instantly with simple, accessible buttons.  
- **AI-Powered Voice Assistant**: Control UAVs with natural voice commands for an enhanced user experience.  
- **Parameter Monitoring & Configuration**: View and edit UAV parameters in real-time.  
- **Multi-UAV Connection & Management**: Connect and control multiple UAVs simultaneously.  
- **Live Map**: Plan and adjust missions or manage connected systems directly on an interactive map.

*(If you have suggestions for features, feel free to contribute!)*

## Powered By

- **MAVSDK** – For seamless communication with UAVs.  
- **Qt6** – For a sleek and responsive user interface.  
- **ArcGIS Map SDK** – For viewing and managing connected systems on a live map.  
- **pybind11** – For bridging C++ and Python, enabling seamless communication with the AI assistant.  
- **skLearn**, **transformers**, **speech_recognition** – For the development of the AI-powered assistant.

---

## Screenshots

![Screenshot 1](https://i.imgur.com/cugY34p.png)  
![Screenshot 2](https://i.imgur.com/TQc6vZC.png)  
![Screenshot 3](https://i.imgur.com/BCOdIqy.png)  
![Screenshot 4](https://i.imgur.com/w9OENam.png)

---

## Installation

1. You must have **Python 3.12** installed on your system to use the AI assistant.
2. Download the latest binaries from the **Releases** section.
3. If you encounter any issues, feel free to open an issue on the repository.

## Compiling

1. Download the **x64-src** file from the **Releases** page.
2. Install **ArcGIS Map SDK 200.5.0**: [Download Link](https://developers.arcgis.com/qt/downloads/)
3. Install **Python 3.12**: [Download Link](https://www.python.org/downloads/release/python-3125/)
4. Ensure that the include and lib paths match those in the project settings.
5. Build the project in **Release | x64** configuration.

---

## License
GNU GPLv3
