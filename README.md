# JPEG Spliter
此库用于对**具备特定格式**的JPEG图像，在单线程以**极高的性能**进行分割（$O(n)$，其中n为JPEG文件的bytes数），是[该任务](https://gitee.com/sun-zhongjie-0426/unity-native-rendering-plugin-d3-d11-cuda)的**下游补充**，包括下述内容：
- [x] 【高性能】c实现与示例
- [x] c的对比测试
- [x] 【易于理解】python实现与示例
- [x] python的对比测试
- [x] 基于c的bytearray实现以及基本功能测试
- [x] python对bytes进行迭代的性能测试
- [ ] 自动读取JPEG相关参数并进行验证
- [ ] 对JPEG各格式进行兼容性测试 
- [x] 更多实验数据
- [x] 更多语言版本的实现
## 开始使用
多种语言的实现均有示例，不依赖**基本库**外的任何内容，可直接使用。注意代码中**读写文件**不会影响耗时的计算。
- [./split_example.c]()
- [./split_example.py]()
- [./CSharp/SplitExample.cs]()
这些示例将根路径下的大图![](./unity_texture2d_big.jpg)拆分到sub文件夹下。
### c
c语言版本的最简单示例，这一示例与[./split_example.c]()基本相同，此外，c版本示例中有**大量的注释**：
```c
#include"jpeg_spliter.c"
//read jpeg data to it
uint8_t *data;
size_t data_length;

// for a jpeg with 5120x3840
spliter_param param = {
    .single_width = 1280, 
    .single_height = 768, 
    .col = 4, 
    .row = 5, 
    .dri = 8, 
    .trim = false
    };

// set 20 pointer
byte_array *ba[20] = {0};

double cost=split(data,data_length,&param,ba);

fwrite(ba[0]->data,1,ba[0]->length,f);
```
### python
python版本的最简单示例就是[./split_example.py]()，没有必要再展开阐述了，不妨将代码拷贝到chatGPT，让chatGPT帮忙讲讲
### C#
C#版本的最简单示例也很简单，见[./CSharp/SplitExample.cs]()，建议让chatGPT进行阐述
## 参数设置
此节对用到的参数（spliter_param）进行介绍，部分参数需要对JPEG有一定的理解：
| 参数          | 类型 | 作用                                                                                                              |
| ------------- | ---- | ----------------------------------------------------------------------------------------------------------------- |
| single_width  | int  | 整张大图中，单张子图的宽度                                                                                        |
| single_height | int  | 整张大图中，单张子图的高度                                                                                        |
| col           | int  | 整张大图中，每行有几张子图                                                                                        |
| row           | int  | 整张大图中，每列有几张子图                                                                                        |
| dri           | int  | 这个参数是JPEG中DRI帧定义的内容，表明了对于图像数据中，RST标志之间有多少个MCU                                     |
| trim          | bool | 对于split函数分割后输出的bytearray，是否需要进行收缩以节省内存，建议禁用，从而以较少的额外空间换取更好的性能(10%) |

## 约束
本节阐述此库的约束；
1. 本库仅确认了在dri=8时可以正常运行
2. 本库仅确认了在三通道（YCrCb）分离存储（JPEG中有三个SOS帧）且每个通道所使用的MCU均为8x8时可以正常运行
3. 对于JPEG图像尺寸的要求：
   1. 大图的长、宽均为8的倍数
   2. 子图的长（width）为64的倍数（8*dri），宽为8的倍数
4. 本库主要针对GPUJPEG库的编码图像进行切分，在使用GPUJPEG编码本库所使用的测试图片时，遵循下述参数：
```c
	gpujpeg_set_default_parameters(&param);
	gpujpeg_image_set_default_parameters(&param_image);
	param_image.color_space = GPUJPEG_RGB;
	param_image.pixel_format = GPUJPEG_4444_U8_P0123;
```
5. 本库不保证JPEG信息的不丢失，并未遵循JPEG标准支持了应支持的帧
## 性能测试
本节对该库的基本性能进行测试，同样地，也探讨由于部分参数指定带来的性能影响。
### 测试图像
测试图像选取了前文的[那张大图](./unity_texture2d_big.jpg)，此外，项目中还包含一张[小一些的图](./unity_texture2d.jpg)可供开展测试。

此图的相关参数：
- 宽度：4*1280像素
- 高度：5*768像素
- 3通道分别编码
- 尺寸：约3822KB

采用的平台中，可能相关的项列举如下：
| 配置              | 参数                                          |
| ----------------- | --------------------------------------------- |
| CPU               | i7-12700                                      |
| MEM               | DDR4 64GB 2400MHz                             |
| OS                | Windows10                                     |
| MinGW编译器       | MinGW GCC8.1.0 64-bit Release                 |
| C编译优化         | -O3                                           |
| C编译器（MSVC）   | cl 19.34.31937                                |
| C编译优化（MSVC） | /O2                                           |
| Python            | 3.11.7                                        |
| OS(Linux)         | Linux 5.4.0-177-generic(Ubuntu 20.04.6)       |
| C编译器(Linux)    | gcc 9.4.0                                     |
| C编译优化(Linux)  | gcc ljt_example.cpp -o ljt_example -ljpeg -O3 |
| C#IDE             | VisualStudio 2022                             |
| .Net              | .Net 6.0                                      |
| C#构建选项        | Release Any CPU                               |

分别运行下述的示例，这将会对一张内存中的jpeg执行200次切分操作，并且不包括文件读写时间，则测试结果如下：
| 实现                                | 平均处理时长(ms) | FPS    | 备注                                                                                                                           |
| ----------------------------------- | ---------------- | ------ | ------------------------------------------------------------------------------------------------------------------------------ |
| c                                   | 4.265            | 234.46 |                                                                                                                                |
| c                                   | 5.820            | 171.82 | `trim=true`                                                                                                                    |
| c                                   | 4.275            | 233.91 | 将byte_array的扩容因子从默认的1.5改为2                                                                                         |
| c                                   | 4.755            | 210.30 | 将split函数中，对byte_array的初始化空间从JPEG尺寸的1/10改为1/20                                                                |
| c                                   | 4.325            | 231.21 | 将split函数中，对byte_array的初始化空间从JPEG尺寸的1/10改为1（必定不会扩容）                                                   |
| c                                   | 5.925            | 168.77 | `trim=true`; </br>将split函数中，对byte_array的初始化空间从JPEG尺寸的1/10改为1（必定不会扩容）                                 |
| c                                   | 4.920            | 203.25 | 将byte_array的扩容因子从默认的1.5改为2; </br>将split函数中，对byte_array的初始化空间从JPEG尺寸的1/10改为1/20                   |
| c                                   | 6.075            | 164.60 | `trim=true`; </br>将byte_array的扩容因子从默认的1.5改为2; </br>将split函数中，对byte_array的初始化空间从JPEG尺寸的1/10改为1/20 |
| c                                   | 5.240            | 190.83 | 对byte_array的初始化空间采用默认的10                                                                                           |
| c(cl)                               | 4.775            | 209.42 | 采用cl.exe进行编译                                                                                                             |
| c(cl)                               | 5.098            | 194.49 | 采用cl.exe进行编译，采用/arch:AVX2                                                                                             |
| C#                                  | 7.17             | 139.36 |                                                                                                                                |
| [Bitmap](./CSharp/BitmapExample.cs) | 7110.57          | 0.14   | 需要在项目属性中将入口类改为BitmapExample;</br>由于该方法过慢，因此`testTime=10`                                               |
| python                              | 201.19           | 4.97   |                                                                                                                                |
| [opencv](./cv_example.py)           | 355.87           | 2.81   |                                                                                                                                |
| [pil](./pil_example.py)             | 422.7            | 2.36   |                                                                                                                                |
| [pil](./pil_example.py)             | 184. 81          | 5.41   | 不执行`tile.save(BytesIO())`                                                                                                   |
| [bet](./byte_enum_test.py)          | 113.07           | 8.84   | 不做任何处理，仅枚举并读取图片中的每个byte，以阐述python实现的主要性能瓶颈                                                     |

此外，考察在Linux平台下的性能（因为可以直接下载可用的[libjpeg-turbo](https://libjpeg-turbo.org/)：`apt install libjpeg-turbo-dev8`）。尽管调用的头与连接的动态库均名为libjpeg，但实际其源码是libjpeg-turbo的实现。
| 实现                     | 平均处理时长(ms) | FPS    | 备注         |
| ------------------------ | ---------------- | ------ | ------------ |
| c                        | 5.216            | 191.73 |              |
| c                        | 2.849            | 351.01 | 不开启O3优化 |
| [ljt](./ljt_example.cpp) | 147.66           | 6.77   |              |
| [ljt](./ljt_example.cpp) | 147.79           | 6.76   | 不开启O3优化 |
### 总结
1. 在c语言版本中，主要的性能浮动来自于byte_array实现中，基于memcpy的扩容。小尺寸的扩容并不会带来过多的性能影响，然而在最后对于约300KB的数据采用memcpy扩容/收缩时，会带来较多的性能损失，因此本项目会将每个byte_array的初始尺寸约定为1/10的最大尺寸（分割为20张图的场景），该数值可以尽可能地避免扩容行为的发生。
2. 经测试，扩容因子是1.5（Java ArrayList）还是2（C++ vector）没有显著的性能影响，Java选取1.5主要来自于对JVM中减少内存碎片的考量。此外，1.5的实现已经通过位运算以避免实数运算。
3. python实现相对c存在约40倍的性能差距，尽管在实现上，均尽可能地采用了最优的方法，但通过在python中对bytes进行枚举测试可以发现性能开销基本来自对其的枚举。
4. PIL的实现中，实际上在将`tile.save(0)`去掉后，可以得到一个比python jpeg spliter更好的性能表现，然而在实际业务场景中，读取分割数据的期望下一步是将图片基于网络发送，因此不能够发送RGB格式的原始数据。
5. 在Linux平台开展的的测试中，是否开启O3对此项目影响很大，且性能远超Windows（完全相同的硬件平台），可能是Windows会将业务线程调到其他核心导致的。此外，基于libjpeg-turbo的实现性能仅比PIL略高。
6. C#版本的性能大概是同平台C版本的2/3，目前尚不清楚原因。但可观察到相比C版本，C#版本运行时CPU会有更多的内核态时间。
7. MSVC编译结果性能略逊于MinGW编译结果，尚不清楚原因
## 原理
主要考虑了JPEG的结构，快速的将RST间的MCU分解到各个子图，并复用JPEG头信息，从而在不解码JPEG的情况下完成图片的分割。

*详细内容待补充……*

## byte_array
byte_array是基于c实现的一个简单的自动扩容的字节数组，主要是由于本人想用纯c实现但纯c里面没有std::vector导致的。

*详细内容待补充……*