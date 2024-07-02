#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#define MCU_LENGTH 8
#define FACTOR2 false
/**
 * @brief a simple implementation for byte array
 *
 */
typedef struct
{
    uint8_t *data;
    size_t length, max_length;
} byte_array;
byte_array *ba_new_with_length(size_t init_length)
{
    byte_array *ba = (byte_array *)malloc(sizeof(byte_array));
    ba->data = (uint8_t *)malloc(init_length * sizeof(uint8_t));
    ba->length = 0;
    ba->max_length = init_length;
    return ba;
}
void ba_free(byte_array *ba)
{
    free(ba->data);
    free(ba);
}

byte_array *ba_new()
{
    return ba_new_with_length(10);
}
void ba_grow(byte_array *ba, size_t length)
{
    if (length < ba->max_length)
    {
        length = ba->max_length;
    }
    size_t new_length = 0;
#if FACTOR2
    new_length = length << 1;
#else
    new_length = length >> 1;
    if (new_length == 0)
        new_length += 1;
    new_length += length;
#endif

    uint8_t *new_data = (uint8_t *)malloc(sizeof(uint8_t) * new_length);
    memcpy(new_data, ba->data, ba->length);
    free(ba->data);
    ba->data = new_data;
    ba->max_length = new_length;
}
void ba_write_with_position(byte_array *ba, size_t position, uint8_t *src, size_t length)
{
    if (position + length > ba->max_length)
    {
        ba_grow(ba, position + length);
    }
    memcpy(ba->data + position, src, length);
    ba->length = position + length;
}
void ba_write(byte_array *ba, uint8_t *src, size_t length)
{
    ba_write_with_position(ba, ba->length, src, length);
}

void ba_trim_append_zero(byte_array *ba, bool append_zero)
{
    size_t new_length = ba->length + (append_zero & 1);
    if (new_length == ba->max_length)
    {
        return;
    }
    uint8_t *new_data = (uint8_t *)malloc(sizeof(uint8_t) * new_length);
    memcpy(new_data, ba->data, ba->length);
    if (append_zero)
    {
        new_data[ba->length] = 0;
    }
    free(ba->data);
    ba->data = new_data;
    ba->max_length = new_length;
}
void ba_trim(byte_array *ba)
{
    ba_trim_append_zero(ba, false);
}
void ba_clear(byte_array *ba)
{
    ba->length = 0;
}

typedef struct
{
    int single_width;  // 1280
    int single_height; // 768
    int col;           // 4
    int row;           // 5
    int dri;           // 8
    bool trim;         // true
} spliter_param;
/**
 * @brief get mcu should be put to which subblock
 *
 * @param counter
 * @param param
 * @return int
 */
int get_mcu_sub(int counter, const spliter_param *param)
{
    int c = param->single_width * param->col / param->dri / MCU_LENGTH;
    int x = counter % c * param->dri * MCU_LENGTH / param->single_width;
    int y = counter * param->dri / c / param->single_height;
    return x + y * param->col;
}
double split(uint8_t *data, size_t length, spliter_param *param, byte_array **out)
{
    struct timespec start_ns, end_ns;
    timespec_get(&start_ns, TIME_UTC);
    int count = param->col * param->row;
    for (int i = 0; i < count; i++)
    {
        if (out[i])
        {
            ba_clear(out[i]);
        }
        else
        {
            out[i] = ba_new_with_length((length / count) << 1);
        }
    }

    int ffda[4];
    int ffda_count = 0;
    for (size_t i = 0; i < length - 1; ++i)
    {
        if (data[i] == 0xFF && data[i + 1] == 0xDA)
        {
            ffda[ffda_count++] = i;
            break;
        }
    }
    for (int i = 0; i < ffda[0]; i += 2)
    {
        if (data[i] == 0xFF && data[i + 1] == 0xC0)
        {
            int height_offset = 2 + 2 + 1; // FFC0 FFFF FF wwww hhhh
            data[i + height_offset] = (param->single_height >> 8) & 0xFF;
            data[i + height_offset + 1] = param->single_height & 0xFF;
            data[i + height_offset + 2] = (param->single_width >> 8) & 0xFF;
            data[i + height_offset + 3] = param->single_width & 0xFF;
            break;
        }
    }
    for (int i = 0; i < count; ++i)
    {
        ba_write(out[i], data, ffda[0]);
    }
    int ffda_index = 0;
    while (ffda_index < ffda_count)
    {
        int ffda_pos = ffda[ffda_index];
        int start_mcu = -1;
        uint8_t sub_rsti[20] = {0};
        int sos_scan_head = 8;
        start_mcu = ffda_pos + sos_scan_head + 2; // sos frame has a fixed length header
        for (int i = 0; i < count; ++i)
        {
            ba_write(out[i], data + ffda_pos, start_mcu - ffda_pos);
        }
        int counter = 0;
        int lasti = start_mcu;

        for (size_t i = start_mcu; i < length; ++i)
        {
            if (data[i] == 0xFF)
            {
                int sub = get_mcu_sub(counter, param);
                if (data[i + 1] >= 0xD0 && data[i + 1] <= 0xD7) // 0xD0~0xD7
                {
                    data[i + 1] = 0xD0 + (sub_rsti[sub]++);
                    sub_rsti[sub] %= 8;
                    ba_write(out[sub], data + lasti, i - lasti + 2);
                    counter++;
                    lasti = i + 2;
                }
                else if (data[i + 1] == 0xDA)
                {
                    ba_write(out[sub], data + lasti, i - lasti);
                    ffda[ffda_count++] = i;
                    break;
                }
                else if (data[i + 1] == 0xD9)
                {
                    ba_write(out[sub], data + lasti, i - lasti);
                    break;
                }
            }
        }
        ffda_index++;
    }

    for (int i = 0; i < count; ++i)
    {
        ba_write(out[i], "\xFF\xD9", 2);
        if (param->trim)
        {
            ba_trim(out[i]);
        }
    }
    timespec_get(&end_ns, TIME_UTC);
    double result = (end_ns.tv_sec - start_ns.tv_sec) + (end_ns.tv_nsec - start_ns.tv_nsec) * 1.0 / 1e9;
    return result;
}
