import torch
import cv2
import pyrealsense2 as rs
import numpy as np
from ultralytics import YOLO
import math
# Load the YOLOv5 model (pre-trained on COCO dataset)
#model = torch.hub.load('ultralytics/yolov5', 'yolov5s', pretrained=True)
model = YOLO('object_test.pt')
model.to('cuda')
# Set up the RealSense pipeline
pipeline = rs.pipeline()
config = rs.config()


# Configure RealSense to use both RGB and depth streams
config.enable_stream(rs.stream.depth, 640, 480, rs.format.z16, 30)
config.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 30)

# Start the pipeline
pipeline.start(config)
'''
COCO_classes = [
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat",
    "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog",
    "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella",
    "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball", "kite",
    "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle",
    "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich",
    "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "TV", "laptop", "mouse", "remote",
    "keyboard", "cell phone", "microwave", "oven", "toaster", "sink", "refrigerator", "book",
    "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"
]
'''
robot_test_classes = ['keyboard', 'monitor', 'mouse', 'robotic-arm']

while True:
    # Wait for frames from the RealSense camera
    frames = pipeline.wait_for_frames()

    # Get the color and depth frames
    color_frame = frames.get_color_frame()
    depth_frame = frames.get_depth_frame()

    if not color_frame or not depth_frame:
        continue

    # Convert the RGB frameexit to a numpy array
    color_image = np.asanyarray(color_frame.get_data())

    # Perform object detection using YOLOv5
    results = model.predict(source=color_image, show=False, device='cuda:0')

    # Extract detected data
    boxes = results[0].boxes.xyxy
    confidences = results[0].boxes.conf
    class_ids = results[0].boxes.cls


    # Draw bounding boxes on the detected objects
    for i, box in enumerate(boxes):
        # extract box coordinates
        x1,y1,x2,y2 = map(int,box)
        conf = confidences[i]
        class_id = int(class_ids[i])

        if class_id < len(robot_test_classes):
            name = robot_test_classes[class_id]
        else:
            name = '?'

        # Get depth at the center of the bounding box
        center_x = int((x1+x2)/2)
        center_y = int((y1+y2)/2)
        depth_value = depth_frame.get_distance(center_x, center_y)
        # box and text colors
        color_BGR = (0, int(conf*255), int((1-conf)*255))
        # Draw bounding box and label
        cv2.rectangle(color_image, (int(x1), int(y1)), (int(x2), int(y2)), color_BGR, 2)
        label = f"{name}, {conf:.2f}, {depth_value:.2f}m"
        cv2.putText(color_image, label, (int(x1), int(y1) - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, color_BGR, 2)

    # Display the RGB frame with detections
    cv2.imshow('YOLOv8 RealSense', color_image)

    # Break the loop on 'q' key press
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Stop the RealSense pipeline
pipeline.stop()
cv2.destroyAllWindows()
