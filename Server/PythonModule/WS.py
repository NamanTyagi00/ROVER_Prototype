import asyncio
import websockets
import socket
import cv2
import numpy as np
from ImageProcessingModule import OPTICALFLOW


async def listen():
    local_ip = socket.gethostname()
    uri = f"ws://{local_ip}:8888"
    try:

        async with websockets.connect(uri) as websocket:
            tracker = OPTICALFLOW()
            print('Connected to server')
            await websocket.send('{"role":"python"}')
            while True:
                try:

                    message = await websocket.recv()
                    frame = cv2.imdecode(np.frombuffer(message, np.uint8), cv2.IMREAD_COLOR)
                    if tracker.frame_number == 0:
                        tracker.detect_initial_features(frame)
                    else:
                        tracker.track_features(frame)
                    frame = tracker.draw_features(frame)
                    # Display the image (optional)
                    cv2.imshow('Image', frame)
                    cv2.waitKey(1)
                    tracker.frame_number += 1
                    print(tracker.frame_number)


                    
                except websockets.ConnectionClosedOK:
                    print("Connection closed normally")
                    break
                except websockets.ConnectionClosedError as e:
                    print(f"Connection closed with error: {e}")
                    break
    except Exception as e:
        print(f"Connection error: {e}")

async def main():
    while True:
        await listen()
        print("Reconnecting in 5 seconds...")
        await asyncio.sleep(5)

if __name__ == "__main__":
    asyncio.run(main())
