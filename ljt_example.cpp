// gcc ljt_example.cpp -o ljt_example -ljpeg -O3
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include <sys/time.h>

unsigned char *read_jpeg_from_memory(const unsigned char *data, long unsigned int data_size, int *width, int *height, int *channels)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, data_size);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    *width = cinfo.output_width;
    *height = cinfo.output_height;
    *channels = cinfo.output_components;

    int row_stride = cinfo.output_width * cinfo.output_components;
    unsigned char *image_buffer = (unsigned char *)malloc(cinfo.output_width * cinfo.output_height * cinfo.output_components);
    unsigned char *row_pointer[1];
    while (cinfo.output_scanline < cinfo.output_height)
    {
        row_pointer[0] = &image_buffer[cinfo.output_scanline * row_stride];
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return image_buffer;
}
unsigned long write_jpeg_to_file(unsigned char *image_buffer, int image_width, int image_height, int channels, int id)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    char filename[50];
    sprintf(filename,"sub/%d.jpg",id);
    FILE *outfile = fopen(filename, "wb");
    if (!outfile)
    {
        fprintf(stderr, "Error opening output jpeg file %s\n", filename);
        return 0;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = image_width;
    cinfo.image_height = image_height;
    cinfo.input_components = channels;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);

    unsigned char *row_pointer[1];
    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &image_buffer[cinfo.next_scanline * image_width * channels];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fclose(outfile);

    return cinfo.image_height * cinfo.image_width * cinfo.input_components; // 返回图像数据大小
}
unsigned long write_jpeg_to_memory(unsigned char *image_buffer, int image_width, int image_height, int channels, unsigned char **outbuffer)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    unsigned long outsize = 0;
    jpeg_mem_dest(&cinfo, outbuffer, &outsize);

    cinfo.image_width = image_width;
    cinfo.image_height = image_height;
    cinfo.input_components = channels;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);

    unsigned char *row_pointer[1];
    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &image_buffer[cinfo.next_scanline * image_width * channels];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return outsize;
}

void split_image(const unsigned char *image_buffer, int image_width, int image_height, int channels, int num_rows, int num_cols)
{
    int piece_width = image_width / num_cols;
    int piece_height = image_height / num_rows;

    // 预先分配足够的内存
    unsigned char *piece_buffer = (unsigned char *)malloc(piece_width * piece_height * channels);
    if (piece_buffer == NULL)
    {
        // 处理内存分配失败的情况
        fprintf(stderr, "Failed to allocate memory for piece_buffer\n");
        return;
    }

    for (int row = 0; row < num_rows; ++row)
    {
        for (int col = 0; col < num_cols; ++col)
        {
            // 计算当前子图像的起始位置
            int start_y = row * piece_height;
            int start_x = col * piece_width;

            // 复制子图像数据
            for (int y = 0; y < piece_height; ++y)
            {
                memcpy(piece_buffer + y * piece_width * channels,
                       image_buffer + ((start_y + y) * image_width + start_x) * channels,
                       piece_width * channels);
            }

            unsigned char *outbuffer = NULL;
            unsigned long outsize = write_jpeg_to_memory(piece_buffer, piece_width, piece_height, channels, &outbuffer);
            // unsigned long outsize = write_jpeg_to_file(piece_buffer, piece_width, piece_height, channels, row * num_cols + col);

            // 处理outbuffer
            if (outbuffer != NULL)
            {
                // 处理outbuffer数据
                free(outbuffer);
            }
        }
    }

    // 释放预先分配的内存
    free(piece_buffer);
}

long get_time_in_microseconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

int main(int argc, char *argv[])
{
    const char *input_filename = "unity_texture2d_big.jpg";

    FILE *infile = fopen(input_filename, "rb");
    if (!infile)
    {
        fprintf(stderr, "Error opening input file %s\n", input_filename);
        return 1;
    }
    fseek(infile, 0, SEEK_END);
    long unsigned int data_size = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    unsigned char *data = (unsigned char *)malloc(data_size);
    fread(data, 1, data_size, infile);
    fclose(infile);

    int width, height, channels;
    int test_time = 2;
    long long costll = 0;
    for (int i = 0; i < test_time; i++)
    {
        long start_time = get_time_in_microseconds();
        unsigned char *image_buffer = read_jpeg_from_memory(data, data_size, &width, &height, &channels);
        split_image(image_buffer, width, height, channels, 5, 4);
        long end_time = get_time_in_microseconds();
        costll += (end_time - start_time);
        free(image_buffer);
    }
    double cost = costll / 1000000.0;
    free(data);
    printf("avg %d cost: %lf, fps:%lf\n", test_time, cost / test_time, test_time / cost);

    return 0;
}
