import sys
import asyncio
import socket
import threading
import cv2
import imutils
import base64
import time
import qwiic
import json
import signal

sys.path.append('/home/pi/sphero-sdk-raspberrypi-python')
from sphero_sdk import SpheroRvrAsync, SerialAsyncDal, RvrStreamingServices


# Constants
HOST_IP = '10.25.46.49'  # The IP address of the RPi
VIDEO_PORT = 6001
VIDEO_BUFF_SIZE = 65536
COMMAND_SERVER_PORT = 6000
DATA_BROADCAST_PORT = 6003
FRAME_WIDTH = 320
FRAME_HEIGHT = 240
JPEG_QUALITY = 25
BATTERY_UPDATE_INTERVAL = 30  # seconds

# Global variables
left_velocity = 0
right_velocity = 0
running = True  # Flag to control the main loop
speed_data = {}  # Global variable for the velocity data from the RVR
encoder_data = {}  # Global variable for encoder data from the RVR
battery_percentage = 0  # Global variable for battery percentage from the RVR
distance_mm = 0  # Global variable for distance in millimeters from the sparkfun TOF
rvr_controller_thread_handle = None
video_stream_thread_handle = None
distance_thread_handle = None
data_thread_handle = None
threads_initialized = False # Global flag for thread initialization

# AsyncIO loop and SpheroRvrAsync instance
loop = asyncio.get_event_loop()
rvr = SpheroRvrAsync(dal=SerialAsyncDal(loop))


def stop():
    global running
    running = False
    print("Stop signal received, setting running to False.")


def process_command(command):
    """Processes a given driving command."""
    global left_velocity, right_velocity
    try:
        left_velocity, right_velocity = map(int, command.split(','))
        left_velocity = int((left_velocity / 255.0) * 127)
        right_velocity = int((right_velocity / 255.0) * 127)
    except ValueError as e:
        print(f"Failed to parse command: {command}. Error: {e}")


def rvr_controller_thread():
    """Thread for handling incoming driving commands via TCP."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(('0.0.0.0', COMMAND_SERVER_PORT))
        server_socket.listen(1)
        print("Command server is running and waiting for a connection...")

        server_socket.settimeout(1)  # Set timeout

        # Command server loop
        while running:
            client_socket, address = None, None
            try:
                client_socket, address = server_socket.accept()
                print(f"Accepted connection from {address}")

                while running:
                    data = client_socket.recv(1024).decode('utf-8')
                    if not data:
                        print("Connection closed by the client.")
                        break
                    commands = data.split('\n')
                    for command in commands:
                        if command:
                            process_command(command)
            except socket.timeout:
                continue  # Check the running flag every 1 second
            except Exception as e:
                print(f"An error occurred with the client {address}: {e}")
            finally:
                if client_socket:
                    client_socket.close()
                    print(f"Connection with {address} has been closed.")


def video_stream_thread():
    """Thread for handling video streaming via UDP."""
    vid = cv2.VideoCapture(0, cv2.CAP_V4L2)
    if not vid.isOpened():
        print("Cannot open camera. Exiting video thread.")
        return  # Exit the thread if the camera cannot be opened

    vid.set(cv2.CAP_PROP_FRAME_WIDTH, FRAME_WIDTH)
    vid.set(cv2.CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as server_socket:
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, VIDEO_BUFF_SIZE)

        try:
            server_socket.bind((HOST_IP, VIDEO_PORT))
        except Exception as e:
            print(f"Cannot bind to address {HOST_IP}:{VIDEO_PORT}, error: {e}")
            vid.release()
            return

        print(f'Video server listening at: {HOST_IP}:{VIDEO_PORT}')

        while running:
            try:
                print('Waiting for client to connect for video stream...')
                _, client_addr = server_socket.recvfrom(VIDEO_BUFF_SIZE)
                print(f'Video connection from {client_addr}')

                while running:
                    ret, frame = vid.read()
                    if not ret:
                        print("Failed to grab frame.")
                        break

                    frame = imutils.resize(frame, width=FRAME_WIDTH)
                    _, buffer = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, JPEG_QUALITY])
                    message = base64.b64encode(buffer)
                    server_socket.sendto(message, client_addr)

            except Exception as e:
                print(f"An error occurred in video_thread: {e}")

    vid.release()
    print("Video server closed.")



def data_broadcast_thread():
    """Thread for broadcasting sensor data via TCP."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(('0.0.0.0', DATA_BROADCAST_PORT))
        server_socket.listen(1)
        print("Data broadcast server running...")

        server_socket.settimeout(1)  # Set timeout for accept operation

        while running:
            try:
                client_socket, _ = server_socket.accept()
                with client_socket:
                    print("Data broadcast client connected")

                    while running:
                        if speed_data or encoder_data or battery_percentage or distance_mm:
                            try:
                                # Combine data from the different sensors
                                combined_data = {
                                    'timestamp': time.time(),
                                    'Speed': speed_data,
                                    'Encoder': encoder_data,
                                    'Battery': battery_percentage,
                                    'Distance': distance_mm
                                }
                                data_str = json.dumps(combined_data) + '\n'
                                client_socket.sendall(data_str.encode('utf-8'))
                            except Exception as e:
                                print(f"Error sending data: {e}")
                                break
            except socket.timeout:
                continue  # Check the running flag every 1 second
            except Exception as e:
                print(f"Data broadcast thread error: {e}")


async def speed_handler(data):
    """Async handler for updating speed data."""
    global speed_data
    speed_data = data
    # print("New speed data received:", speed_data) # Debugging print


async def update_battery_percentage():
    """Async task for updating battery percentage."""
    global battery_percentage
    while running:
        battery_percentage = await rvr.get_battery_percentage()
        # print(f"Battery percentage: {battery_percentage}")  # Debugging print
        await asyncio.sleep(BATTERY_UPDATE_INTERVAL)


def measure_distance():
    """Measures distance using the VL53L1X sensor."""
    global distance_mm
    ToF = qwiic.QwiicVL53L1X()
    if ToF.sensor_init() is None:
        print("Distance sensor online!\n")

    while running:
        try:
            ToF.start_ranging()
            time.sleep(0.005)
            distance_mm = ToF.get_distance()
            time.sleep(0.005)
            ToF.stop_ranging()
            # print(f"Distance(mm): {distance_mm}")  # Debugging print
        except Exception as e:
            print(e)
        time.sleep(0.1)  # Adjust the frequency of measurements as needed

def signal_handler(sig, frame):
    print('\nSignal received, stopping...')
    stop()

signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

async def main():
    """Main async function to set up and run the RVR and its servers."""
    global rvr_controller_thread_handle, video_stream_thread_handle, distance_thread_handle, data_thread_handle, threads_initialized

    await rvr.wake()
    await asyncio.sleep(2)
    await rvr.reset_yaw()

    # Set up speed streaming
    await rvr.sensor_control.add_sensor_data_handler(
        service=RvrStreamingServices.velocity,
        handler=speed_handler
    )
    await rvr.sensor_control.start(interval=250)

    # Set up battery percentage update
    battery_percentage_task = asyncio.create_task(update_battery_percentage())

    # Starting threads for server, video, distance and data broadcast
    rvr_controller_thread_handle = threading.Thread(target=rvr_controller_thread, daemon=True, name="rvr_controller_thread"
    rvr_controller_thread_handle.start()
    video_stream_thread_handle = threading.Thread(target=video_stream_thread, daemon=True, name="video_stream_thread")
    video_stream_thread_handle.start()
    distance_thread_handle = threading.Thread(target=measure_distance, daemon=True, name="distance_thread")
    distance_thread_handle.start()
    data_thread_handle = threading.Thread(target=data_broadcast_thread, daemon=True, name="data_thread"
    data_thread_handle.start()

    threads_initialized = True

    # Main loop for driving the RVR
    while running:
        await rvr.drive_tank_normalized(left_velocity=left_velocity, right_velocity=right_velocity)
        encoder_counts = await rvr.get_encoder_counts()
        encoder_data = encoder_counts
        # print(encoder_data)  # Debugging print
        await asyncio.sleep(0.1)

    # Cleanup
    battery_percentage_task.cancel()

    await rvr.close()


if __name__ == '__main__':
    try:
        loop.run_until_complete(main())
    except KeyboardInterrupt:
        print('\nProgram terminated with keyboard interrupt.')
        stop()
    finally:
        # Cancel all running tasks and close the loop
        for task in asyncio.all_tasks(loop):
            task.cancel()
        loop.run_until_complete(asyncio.gather(*asyncio.all_tasks(loop), return_exceptions=True))
        loop.close()

        # Now join the threads after the event loop is closed
        if threads_initialized:
            if rvr_controller_thread_handle is not None:
                rvr_controller_thread_handle.join()
            if video_stream_thread_handle is not None:
                video_stream_thread_handle.join()
            if distance_thread_handle is not None:
                distance_thread_handle.join()
            if data_thread_handle is not None:
                data_thread_handle.join()

        print("All threads have been successfully stopped.")
