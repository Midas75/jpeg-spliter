import time
class SpliterParam:
    singleWidth: int
    singleHeight: int
    col: int
    row: int
    dri: int
    mcuLength: int = 8

    def __init__(self, singleWidth: int, singleHeight: int, col: int, row: int, dri: int):
        self.singleHeight = singleHeight
        self.singleWidth = singleWidth
        self.col = col
        self.row = row
        self.dri = dri


def getMcuSub(counter: int, param: SpliterParam) -> int:
    c = param.singleWidth*param.col//param.dri//param.mcuLength
    x = counter % c*param.dri*param.mcuLength//param.singleWidth
    y = counter * param.dri // c//param.singleHeight
    return x+y*param.col

# data will be edit (set jpeg header's w&h, cannot use bytes instead)
def split(data: bytearray, param: SpliterParam) -> tuple[float, list[bytearray]]:
    start_time = time.time()
    count = param.col*param.row
    out = [bytearray() for _ in range(count)]
    ffda = []
    length = len(data)
    for i in range(0, length-1):
        if data[i] == 0xFF and data[i+1] == 0xDA:
            ffda.append(i)
            break
    for i in range(0, ffda[0], 2):
        if data[i] == 0xFF and data[i+1] == 0xC0:
            height_offset = 2+2+1
            data[i+height_offset] = param.singleHeight // 256
            data[i+height_offset+1] = param.singleHeight % 256
            data[i+height_offset+2] = param.singleWidth // 256
            data[i+height_offset+3] = param.singleWidth % 256
            break
    for ba in out:
        ba += data[0:ffda[0]]
    ffda_index = 0
    while ffda_index < len(ffda):
        ffda_pos = ffda[ffda_index]
        start_mcu = -1
        sub_rsti = [0 for _ in range(count)]
        sos_scan_head = 8
        start_mcu = ffda_pos+sos_scan_head+2
        for ba in out:
            ba += data[ffda_pos:start_mcu-ffda_pos]
        counter = 0
        lasti = start_mcu

        for i in range(start_mcu, length):
            if data[i] == 0xFF:
                sub = getMcuSub(counter, param)
                if data[i+1] >= 0xD0 and data[i+1] <= 0xD7:
                    data[i+1] = 0xD0+sub_rsti[sub]
                    sub_rsti[sub] += 1
                    sub_rsti[sub] %= 8
                    out[sub] += data[lasti:i-lasti+2]
                    counter += 1
                    lasti = i+2
                elif data[i+1] == 0xDA:
                    out[sub] += data[lasti:i-lasti]
                    ffda.append(i)
                    break
                elif data[i+1] == 0xD9:
                    out[sub] += data[lasti:i-lasti]
                    break
        ffda_index += 1
    for ba in out:
        ba += b"\xFF\xD9"
    result = time.time()-start_time
    return result, out
