# Sphero Robot

## Overview:
The Sphero Robot Project is designed to enable real-time interaction and control over a Sphero robot via a networked interface.
The system utilizes a combination of PC-based control software and Raspberry Pi-based server scripts to manage communications, sensor data processing, and motor control.

## System Requirements 
- PC with SDL (Simple DirectMedia Layer) library and vcpkg.
- Raspberry Pi with Python 3 and necessary libraries (cv2, imutils, numpy, etc.).

## Installation
- Clone the repository 
- Install required dependencies (listed in vcpkg.json and Python script requirements).

## Usage
- Run main.cpp on the PC to start the control interface.
- ~~x and z keybindings for either manual or autonomous steering~~
- Change between manual and autonomous steering through the GUI/HTML interface
- Execute server.py locally on the Rpi connected to the RVR to initiate server functionalities and video streaming.

## Features
- Real-time Control: Control the Sphero robot in real-time using keyboard or joystick control.
- Video Streaming: Stream video data from Raspberry Pi to the GUI interface.
- Sensor Data Processing: Process and respond to sensor data from the Sphero robot.
- Automatic brake function utilizing range data from a sensor
- Color tracking algorithm using the camera to follow a specified color

## Code structure and summary 

### Communication module
- client.hpp: Defines a TCP server class for handling TCP network connections.
- mock_server.hpp: Implements a mock server for testing, capable of handling both TCP and UDP connections.
- network_helper.hpp: Provides helper functions for network operations, such as converting integers to byte arrays.
- network_socket.hpp: Defines a socket handler class for network communications.
- tcp_client.cpp: Contains the implementation of the TCP client logic, interacting with the motor controller.

### Control module
- motorcontroller.hpp: Defines a class for controlling the motor movements of the robot, including functions for normalizing belt speeds and handling user input.
- motorcontroller.cpp: Implements the motor control logic, with functions for normalizing motor speeds and handling user commands.

### Sensors module
- sensordata.hpp: Contains the definitions and structures for sensor data processing.
- sensorreader.cpp: Implements functions for processing sensor data, such as color tracking in images.

### Video module
- thread_safe_queue.hpp: Defines a thread-safe queue for storing the undecoded base64 video frames received from the server.

### GUI module
- index.html: Contains the HTML code for the GUI interface.
- style.css: Contains the CSS code for the GUI interface.
- script.js: Contains the JavaScript code for the GUI interface.

### Server module
- server.py: Implements the server logic for handling network communications and sensor data processing (TCP for data and UDP for video).

### Summary 
Each module (communications, control, sensors) plays a specific role in the overall functionality of the Sphero Robot project. The communications module handles network interactions, the control module manages motor movements and user inputs, and the sensors module processes sensor data for navigation and interaction purposes.