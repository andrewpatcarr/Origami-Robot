import torch
import cv2
import pyrealsense2 as rs
import numpy as np
from ultralytics import YOLO

# Testing without freezing
model = YOLO('yolov8s.pt')
model.to('cuda')

if __name__ == '__main__':
    model.train(data="data.yaml", epochs=50, imgsz=640)



freeze = [f'model.{x}.' for x in range(freeze)]  # layers to freeze
for k, v in model.named_parameters():
 v.requires_grad = True  # train all layers
 if any(x in k for x in freeze):
     print(f'freezing {k}')
     v.requires_grad = False