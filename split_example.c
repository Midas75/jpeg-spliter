// Linux: gcc split_example.c -o split_example -std=c11 -O3
// Windows: cl split_example.c /O2
// simd implementation: requires special compilation parameters
// #include "jpeg_spliter_simd.c"

#include "jpeg_spliter.c"
int main()
{
	// this example shows how to use jpeg_spliter
	// Just paste the code into chatGPT and it can interpret the entire process
	const char *path = "unity_texture2d_big.jpg";
	// When true, the sub jpeg image will be written to ./pic/
	bool write_to_file = true;
	double cost = 0;
	// Conduct multiple tests to obtain the average running time
	int test_time = 200;
	// open file
	FILE *f = fopen(path, "rb");
	// get the total length by fseek
	fseek(f, 0, SEEK_END);
	size_t file_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	// allocate the memory to read all the file data
	uint8_t *data = (uint8_t *)malloc(file_size);
	fread(data, 1, file_size, f);
	// no longer need to use the original file
	fclose(f);
	// initialize the split parameters
	// refer to readme for specific requirements
	// single_width&single_height: the subpic's width&height
	// col&row: how many columns&rows will the original image be divided into
	// dri: a jpeg parameter indicates how many MCUs(8x8) will be insterted after/before an RST
	// trim: whether to trim the byte array to a suitable space will save memory space, but it will be a little bit slower
	spliter_param param = {.single_width = 1280, .single_height = 768, .col = 4, .row = 5, .dri = 8, .trim = false};
	// Implementation of a simple byte array for carrying split jpeg data
	// 20 means param.col*param.row
	byte_array *ba[20] = {0};
	// many test
	for (int i = 0; i < test_time; i++)
	{
		// The splitting method will return its own time consumption
		// byte arrays will be created within this method, and users will need to release them themselves when they are no longer in use
		cost += split(data, file_size, &param, ba);
		// write the content to file
		if (write_to_file)
		{
			for (int j = 0; j < 20; j++)
			{
				char filename[50];
				snprintf(filename, 50, "sub/%d.jpg", j);
				FILE *pic = fopen(filename, "wb");
				// pay attention to the usage of byte_array
				fwrite(ba[j]->data, 1, ba[j]->length, pic);
				fclose(pic);
			}
		}
		// free it just like this, in split(), if ba[j] is not 0, it will reuse the byte_array
		for (int j = 0; j < 20; j++)
		{
			ba_free(ba[j]);
			// important to set it zero
			ba[j] = 0;
		}
	}

	printf("avg %d cost: %lf, fps:%lf", test_time, cost / test_time, test_time / cost);
	free(data);
	return 0;
}
