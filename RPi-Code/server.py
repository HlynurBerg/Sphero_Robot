#Used for controlling rvr with tcp and streaming video with udp

import os
import sys
import asyncio
import socket
import threading
import cv2
import imutils
import numpy as np
import base64
from sphero_sdk import SpheroRvrAsync
from sphero_sdk import SerialAsyncDal

loop = asyncio.get_event_loop()

rvr = SpheroRvrAsync(
    dal=SerialAsyncDal(
        loop
    )
)

left_velocity = 0
right_velocity = 0

# Process the command received from the C++ client
def process_command(command):
    global left_velocity, right_velocity
    try:
        print(f"Received command: {command}")
        left_velocity, right_velocity = map(int, command.split(','))
        left_velocity = int((left_velocity / 255.0) * 127)
        right_velocity = int((right_velocity / 255.0) * 127)
        print(f"Processed velocities: Left: {left_velocity}, Right: {right_velocity}")
    except ValueError:
        print(f"Failed to parse command: {command}")

# Thread to receive data from the C++ client
def server_thread():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('0.0.0.0', 6000))
    server_socket.listen(1)
    print("Server is running and waiting for a connection...")

    client_socket, address = server_socket.accept()
    print(f"Accepted connection from {address}")

    while True:
        try:
            data = client_socket.recv(1024).decode('utf-8')
            if not data:
                print("Connection closed.")
                break
            commands = data.split('\n')
            for command in commands:
                if command:
                    process_command(command)
        except Exception as e:
            print(f"An error occurred: {e}")
            break

    client_socket.close()
    server_socket.close()

# Thread to handle video streaming
def video_thread():
    BUFF_SIZE = 32768
    host_ip = '10.25.46.49'
    port = 6123
    socket_address = (host_ip, port)

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, BUFF_SIZE)
    server_socket.bind(socket_address)
    print(f'Video server listening at: {socket_address}')

    vid = cv2.VideoCapture(0, cv2.CAP_V4L2)
    vid.set(cv2.CAP_PROP_FRAME_WIDTH, 320)
    vid.set(cv2.CAP_PROP_FRAME_HEIGHT, 240)

    while True:
        msg, client_addr = server_socket.recvfrom(BUFF_SIZE)
        print(f'GOT connection from {client_addr}')

        while vid.isOpened():
            ret, frame = vid.read()
            if not ret:
                print("Failed to grab frame")
                break
            frame = imutils.resize(frame)
            _, buffer = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 30])
            message = base64.b64encode(buffer)
            server_socket.sendto(message, client_addr)

async def main():
    global left_velocity, right_velocity

    await rvr.wake()
    await asyncio.sleep(2)
    await rvr.reset_yaw()

    server_thread_handle = threading.Thread(target=server_thread)
    server_thread_handle.start()

    video_thread_handle = threading.Thread(target=video_thread)
    video_thread_handle.start()

    while True:
        print(f"Sending to Sphero: Left: {left_velocity}, Right: {right_velocity}")
        await rvr.drive_tank_normalized(
            left_velocity=left_velocity,
            right_velocity=right_velocity
        )
        await asyncio.sleep(0.1)

if __name__ == '__main__':
    try:
        loop.run_until_complete(
            main()
        )
    except KeyboardInterrupt:
        print('\nProgram terminated with keyboard interrupt.')
        loop.run_until_complete(
            rvr.close()
        )
    finally:
        if loop.is_running():
            loop.close()
