CC = cl
CFLAGS = /O2 /LD
RM = del /Q
TARGET = jpeg_spliter_wrapper.dll
SRC = jpeg_spliter_wrapper.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) $(CFLAGS) /link /out:$(TARGET)

clean:
	$(RM) $(TARGET) 
