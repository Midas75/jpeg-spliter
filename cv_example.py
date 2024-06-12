import cv2
import time
import numpy as np
from io import BytesIO

# 读取图片数据到内存
file_path = "unity_texture2d_big.jpg"
with open(file_path, "rb") as f:
    img_data = f.read()

# 将图像数据转换为 NumPy 数组
img_array = np.frombuffer(img_data, np.uint8)

test_time = 200
cost = 0
for i in range(test_time):
    start_time = time.time()
    
    # 从内存中读取图片
    img = cv2.imdecode(img_array, cv2.IMREAD_COLOR)
    if img is None:
        raise FileNotFoundError(f"File {file_path} could not be read from memory.")
    
    img_height, img_width, _ = img.shape
    num_cols = 4
    num_rows = 5
    tile_width = img_width // num_cols
    tile_height = img_height // num_rows
    
    for row in range(num_rows):
        for col in range(num_cols):
            left = col * tile_width
            top = row * tile_height
            right = (col + 1) * tile_width
            bottom = (row + 1) * tile_height
            tile = img[top:bottom, left:right]
            # 将图块保存到内存
            _, tile_encoded = cv2.imencode('.jpg', tile)
    
    cost += time.time() - start_time

print(f"avg {test_time} cost: {cost/test_time}, fps: {test_time/cost}")
