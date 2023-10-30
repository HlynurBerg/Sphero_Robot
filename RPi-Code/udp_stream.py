# This is server code to send video frames over UDP - Inspiration from https://pyshine.com/Send-video-over-UDP-socket-in-Python/

import cv2, imutils, socket
import numpy as np
import time
import base64

def main():
    # Server configuration
    BUFF_SIZE = 32768
    host_ip = '10.25.46.49'
    port = 6123
    socket_address = (host_ip, port)

    # Create and bind the server socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, BUFF_SIZE)
    server_socket.bind(socket_address)
    print(f'Listening at: {socket_address}')

    # Video capture initialization
    vid = cv2.VideoCapture(0, cv2.CAP_V4L2)
    vid.set(cv2.CAP_PROP_FRAME_WIDTH, 320)
    vid.set(cv2.CAP_PROP_FRAME_HEIGHT, 240)

    while True:
        # Listen for a connection
        msg, client_addr = server_socket.recvfrom(BUFF_SIZE)
        print(f'GOT connection from {client_addr}')

        while vid.isOpened():
            ret, frame = vid.read()
            if not ret:
                print("Failed to grab frame")
                break

            # Resize and encode frame
            frame = imutils.resize(frame)
            _, buffer = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 30])

            # Send encoded frame
            message = base64.b64encode(buffer)
            server_socket.sendto(message, client_addr)

            # Exit on 'q' key press
            if cv2.waitKey(1) & 0xFF == ord('q'):
                server_socket.close()
                break

if __name__ == "__main__":
    main()