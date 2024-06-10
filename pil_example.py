from PIL import Image
import time
from io import BytesIO
f = open("unity_texture2d_big.jpg", "rb")
data = BytesIO(f.read())
f.close()
test_time = 200
cost = 0
for i in range(test_time):
    start_time = time.time()
    img = Image.open(data)
    img_width, img_height = img.size
    num_cols = 4
    num_rows = 5
    tile_width = img_width // num_cols
    tile_height = img_height // num_rows
    for row in range(num_rows):
        for col in range(num_cols):
            left = col*tile_width
            top = row*tile_height
            right = (col + 1) * tile_width
            bottom = (row + 1) * tile_height
            box = (left, top, right, bottom)
            tile = img.crop(box)
            tile.save(BytesIO(), format="JPEG")
    cost += time.time()-start_time
print(f"avg {test_time} cost:{cost/test_time}, fps: {test_time/cost}")
