# Sphero Robot

## Overview
The Sphero Robot Project is designed to enable real-time interaction and control over a Sphero robot via a networked interface.
The system utilizes a combination of PC-based control software and Raspberry Pi-based server scripts to manage communications, sensor data processing, and motor control.
I takes advantage of the Sphero SDK to enable real-time control of the robot, and the OpenCV library to process video data from the Raspberry Pi camera. With the use of TOF distance sensors, the robot is able to detect obstacles and stop automatically. It is also able to track a specified color and follow it.

## System Requirements 
- Windows with SDL (Simple DirectMedia Layer), boost and opencv library (installed through vcpkg).
- Raspberry Pi Zero with Python 3.9, and necessary libraries (cv2, imutils, numpy, etc.).

## Installation
- Clone the repository 
- Install required dependencies on both client and RPi (listed in vcpkg.json and Python script imports).
- Change the IP addresses for the UDP and TCP handler in the client.cpp and main.cpp file to the IP address of the RPi.
- Change the IP address in the server.py file to the IP address of the PC.

## Usage
- Execute server.py locally on the Rpi connected to the RVR to initiate server functionalities and video streaming.
- Run main.cpp on the PC to start the control interface.
- Control the robot using the both the GUI interface and the SDL interface.
- Change between manual and autonomous steering through the GUI interface

## Features
- Real-time Control: Control the Sphero robot in real-time using keyboard or joystick control.
- Video Streaming: Stream video data from Raspberry Pi to the GUI interface.
- Sensor Data Processing: Process and respond to sensor data from the Sphero robot.
- Automatic brake function utilizing range data from a sensor
- Color tracking algorithm using the camera to follow a specified color


## Code structure and summary 

### Communication module
- client.hpp: Defines a TCP/UDP server class for handling TCP/UDP network connections.
- client.cpp: Contains the implementation of the TCP client logic, interacting with the motor controller.
- websocket_client.hpp: Defines a WebSocket session and server class for handling WebSocket network connections.

### Testing module
- mock_server.hpp: Implements a mock server for testing, capable of handling both TCP and UDP connections.
- client_tester.cpp: Implements a test suite for the client module, including tests for TCP and UDP connections.
- color_tracking_tests.cpp: Implements a test suite for the color tracking algorithm.
- motorcontroller_tests.cpp: Implements a test suite for the motor controller module.
- JSON_tester.cpp: Implements a test suite for the JSON parser module.

### Control module
- motorcontroller.hpp: Defines a class for controlling the motor movements of the robot, including functions for normalizing belt speeds and handling user input.
- motorcontroller.cpp: Implements the motor control logic, with functions for normalizing motor speeds and handling user commands.

### Sensors module
- sensor_processing.hpp: Contains the definitions and structures for sensor data processing.
- sensor_processing.cpp: Implements functions for processing sensor data, such as color tracking in images.

### Video module
- thread_safe_queue.hpp: Defines a thread-safe queue for storing the undecoded base64 video frames received from the server.

### GUI module
- index.html: Contains the HTML code for the GUI interface.
- style.css: Contains the CSS code for the GUI interface.
- script.js: Contains the JavaScript code for the GUI interface.

### Server module
- server.py: Implements the server logic for handling network communications and sensor data processing (TCP for data and UDP for video).

### Summary 
Each module (communications, control, sensors etc) plays a specific role in the overall functionality of the Sphero Robot project. The communications module handles the different networks interactions, the control module manages motor movements and user inputs, sensors module processes sensor data for navigation and interaction purposes, video module handles the stream from the UDP server. In combination, these are presented at the GUI interface and partially controlled by both the GUI and the SDL interface.