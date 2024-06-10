# Testing Byte by Byte Access Time
import time

test_time = 200


def byte_enum(data):
    start_time = time.time()
    for i in range(len(data)):
        if data[i] == 0xFF:
            pass
    return time.time()-start_time


f = open("unity_texture2d_big.jpg", "rb")
data = f.read()
f.close()
cost = 0
for i in range(200):
    cost += byte_enum(data)

print(f"avg {test_time} cost:{cost/test_time}, fps: {test_time/cost}")
