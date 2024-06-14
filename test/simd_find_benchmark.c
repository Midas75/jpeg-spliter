#include <immintrin.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
int main()
{
    const char *path = "../unity_texture2d_big.jpg";
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    int test_time = 2048;
    if (_mm_popcnt_u32(test_time) != 1)
    {
        printf("test_time must be a power of two!\n");
        return 1;
    }
    uint8_t *data = (uint8_t *)malloc(file_size * test_time);
    fread(data, 1, file_size, f);
    //    memset(data,0,file_size);
    //	memset(data,0xFF,file_size);
    printf("copying\n");
    for (size_t i = 1; i < test_time; i *= 2)
    {
        memcpy(data + i * file_size, data, i * file_size);
    }
    printf("copied\n");
    fclose(f);
    file_size *= test_time;
    bool *check_map[3] = {
        malloc(file_size),
        malloc(file_size),
        malloc(file_size)};

    {
        clock_t start_time = clock();
        for (size_t i = 0; i < file_size; i++)
        {
            if (data[i] == 0xFF)
            {
                check_map[0][i] = true;
            }
        }
        printf("Native Method Usage: %ld\n", clock() - start_time);
    }
    {
        const size_t chunk32_size = 32;
        const __m256i pattern32 = _mm256_set1_epi8(0xFF);
        const size_t cl32 = file_size - chunk32_size;
        clock_t start_time = clock();
        size_t i = 0;
        for (; i <= cl32; i += chunk32_size)
        {
            __m256i chunk = _mm256_loadu_si256((__m256i *)(data + i));
            __m256i cmp_result = _mm256_cmpeq_epi8(chunk, pattern32);
            int mask = _mm256_movemask_epi8(cmp_result);
            if (mask != 0)
            {
                for (size_t j = 0; j < chunk32_size; j++)
                {
                    if (mask & (1 << j))
                    {
                        check_map[1][i + j] = true;
                    }
                }
            }
        }
        for (; i < file_size; i++)
        {
            if (data[i] == 0xFF)
            {
                check_map[1][i] = true;
            }
        }
        printf("SIMD256 Method Usage: %ld\n", clock() - start_time);
    }
    {
        const size_t chunk16_size = 16;
        const __m128i pattern16 = _mm_set1_epi8(0xFF);
        const size_t cl16 = file_size - chunk16_size;
        clock_t start_time = clock();
        size_t i = 0;
        for (; i < cl16; i += chunk16_size)
        {
            __m128i chunk = _mm_loadu_si128((__m128i *)(data + i));
            __m128i cmp_result = _mm_cmpeq_epi8(chunk, pattern16);
            int mask = _mm_movemask_epi8(cmp_result);
            if (mask != 0)
            {
                for (size_t j = 0; j < chunk16_size; j++)
                {
                    if (mask & (1 << j))
                    {
                        check_map[2][i + j] = true;
                    }
                }
            }
        }
        for (; i < file_size; i++)
        {
            if (data[i] == 0xFF)
            {
                check_map[2][i] = true;
            }
        }
        printf("SIMD128 Method Usage: %ld\n", clock() - start_time);
    }
    {
        bool diff = false;
        printf("starting diff check\n");
        for (int i = 0; i < file_size; i++)
        {
            if (check_map[0][i] != check_map[1][i] ||
                check_map[0][i] != check_map[2][i])
            {
                printf("diff in pos [%d]: [%d] [%d] [%d]\n", i, check_map[0][i], check_map[1][i], check_map[2][i]);
                diff = true;
            }
        }
        if (!diff)
        {
            printf("diff check passed\n");
        }
    }
}
