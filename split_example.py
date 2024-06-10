
import jpeg_spliter
path = "unity_texture2d_big.jpg"
write_to_file = False
test_time = 200
f = open(path, "rb")
data = bytearray(f.read())
f.close()

param = jpeg_spliter.SpliterParam(
    singleWidth=1280,
    singleHeight=768,
    col=4,
    row=5,
    dri=8
)

cost = 0

for i in range(test_time):
    icost, out = jpeg_spliter.split(data, param)
    cost += icost
    if write_to_file:
        for i in range(len(out)):
            f = open(f"sub/{i}.jpg", "wb")
            f.write(out[i])
            f.close()
print(f"avg {test_time} cost:{cost/test_time}, fps: {test_time/cost}")
