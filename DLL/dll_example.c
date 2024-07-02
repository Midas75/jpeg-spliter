#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
typedef void (*ClearSplitContext)();
typedef void (*InitSplitContext)(int single_width,
                                 int single_height,
                                 int col,
                                 int row,
                                 int dri);
typedef double (*DoSplit)(uint8_t *jpegData, size_t length);
typedef uint8_t *(*GetJpedData)(int index, size_t *length);
int main()
{
    HINSTANCE hDLL = LoadLibrary("jpeg_spliter_wrapper.dll");
    ClearSplitContext clearSplitContext = (ClearSplitContext)GetProcAddress(hDLL, "ClearSplitContext");
    InitSplitContext initSplitContext = (InitSplitContext)GetProcAddress(hDLL, "InitSplitContext");
    DoSplit doSplit = (DoSplit)GetProcAddress(hDLL, "DoSplit");
    GetJpedData getJpegData = (GetJpedData)GetProcAddress(hDLL, "GetJpegData");

    const char *path = "../unity_texture2d_big.jpg";
    bool write_to_file = true;
    double cost = 0;
    int test_time = 200;
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *data = (uint8_t *)malloc(file_size);
    fread(data, 1, file_size, f);
    fclose(f);
    initSplitContext(1280, 768, 4, 5, 8);
    for (int i = 0; i < test_time; i++)
    {
        cost += doSplit(data, file_size);
        if (write_to_file)
        {
            for (int j = 0; j < 20; j++)
            {
                char filename[50];
                snprintf(filename, 50, "../sub/%d.jpg", j);
                FILE *pic = fopen(filename, "wb");
                size_t len;
                uint8_t *sub_data = getJpegData(j, &len);
                fwrite(sub_data, 1, len, pic);
                fclose(pic);
            }
        }
    }
    clearSplitContext();
    printf("avg %d cost: %lf, fps:%lf", test_time, cost / test_time, test_time / cost);
    free(data);
    return 0;
}
