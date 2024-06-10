#include <stdint.h>
#include<malloc.h>
#include <stdbool.h>
#include<string.h>
#include "jpeg_spliter.c"
#include <stdio.h>
int main() {
	printf("testing str\n");
	byte_array *ba = ba_new();
	char hello[] = "hello";
	ba_write(ba, hello, strlen(hello) + 1);
	printf("data in ba is: [%s], now its length is [%ld], max length is [%ld]\n", ba->data, ba->length, ba->max_length);
	char world[] = " world";
	ba_write_with_position(ba, strlen(hello), world, strlen(world) + 1);
	printf("data in ba is: [%s], now its length is [%ld], max length is [%ld]\n", ba->data, ba->length, ba->max_length);

	ba_free(ba);
	printf("ba freed\n");
	int data[1000] = {0};

	ba = ba_new();
	printf("a new ba is created with length [%d], max length [%d]\n", ba->length, ba->max_length);
	ba_write(ba, (uint8_t*)data, sizeof(int) * 1000);
	printf("written [%ld] bytes to ba, now its max length is [%ld]\n", sizeof(int) * 1000, ba->max_length);
	bool full_zero_check = true;
	for (int i = 0; i < sizeof(int) * 1000; i += 4) {
		if (ba->data[i] != 0) {
			full_zero_check = false;
			break;
		}
	}
	printf("full zero check for ba is :%u\n", full_zero_check);
	printf("testing trim\n");
	ba_clear(ba);
	ba_write(ba, hello, strlen(hello));
	ba_trim(ba);
	printf("ba trimmed, now length: [%ld], max length: [%ld]\n", ba->length, ba->max_length);
	ba_trim_append_zero(ba,true);
	printf("ba trimmed and appended zero, now length: [%ld], max length: [%ld]\n", ba->length, ba->max_length);
	printf("taz ba has content: [%s]\n",ba->data);
	
	

}
