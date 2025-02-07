import pyrealsense2 as rs
import numpy as np
import socket
import cv2

# Configure RealSense camera pipeline
pipeline = rs.pipeline()
config = rs.config()

# Enable depth and color streams
config.enable_stream(rs.stream.depth, 640, 480, rs.format.z16, 30)
config.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 30)

# Start streaming
pipeline.start(config)

# Set up socket communication (e.g., with localhost and specific port)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('localhost', 65432))
sock.listen(1)

print("Waiting for connection from MATLAB...")
conn, addr = sock.accept()
print(f"Connected to {addr}")

try:
    while True:
        # Wait for a coherent pair of frames: depth and color
        frames = pipeline.wait_for_frames()
        depth_frame = frames.get_depth_frame()
        color_frame = frames.get_color_frame()

        if not depth_frame or not color_frame:
            continue

        # Convert depth and color frames to numpy arrays
        depth_image = np.asanyarray(depth_frame.get_data())
        color_image = np.asanyarray(color_frame.get_data())

        # Serialize color and depth data and send to MATLAB
        data = {
            'depth': depth_image.tobytes(),
            'color': color_image.tobytes(),
            'shape_depth': depth_image.shape,
            'shape_color': color_image.shape
        }

        # Send depth and color data as serialized data
        conn.sendall(b"START")
        conn.sendall(data['depth'])
        conn.sendall(b"BREAK")
        conn.sendall(data['color'])
        conn.sendall(b"END")

except Exception as e:
    print(f"Error: {e}")

finally:
    # Stop the pipeline and close the socket
    pipeline.stop()
    conn.close()
    sock.close()
