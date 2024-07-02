#include "../jpeg_spliter.c"
static spliter_param __sp;
static byte_array **__ba = 0;
static int ba_size = 0;
extern __declspec(dllexport) void __stdcall ClearSplitContext()
{
    if (__ba != 0)
    {
        for (int i = 0; i < ba_size; i++)
        {
            ba_free(__ba[i]);
        }
        free(__ba);
        ba_size = 0;
    }
}
extern __declspec(dllexport) void __stdcall InitSplitContext(
    int single_width,
    int single_height,
    int col,
    int row,
    int dri)
{
    __sp.single_height = single_height;
    __sp.single_width = single_width;
    __sp.col = col;
    __sp.row = row;
    __sp.dri = dri;
    __sp.trim = false;
    ClearSplitContext();
    ba_size = col * row;
    __ba = (byte_array **)malloc(ba_size * sizeof(byte_array *));
    for (int i = 0; i < ba_size; i++)
    {
        __ba[i] = ba_new();
    }
}
extern __declspec(dllexport) double __stdcall DoSplit(uint8_t *jpegData, size_t length)
{
    return split(jpegData, length, &__sp, __ba);
}
extern __declspec(dllexport) uint8_t *__stdcall GetJpegData(int index, size_t *length)
{
    if (index >= ba_size)
    {
        *length = 0;
        return 0;
    }
    *length = __ba[index]->length;
    return __ba[index]->data;
}
