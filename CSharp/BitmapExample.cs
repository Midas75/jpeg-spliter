using System;
using System.Drawing;
using System.Drawing.Imaging;

class BitmapExample
{
    static void Main(string[] args)
    {
        // 读取图像
        string relativePath = "../../../../";
        byte[] data = File.ReadAllBytes(relativePath + "unity_texture2d_big.jpg");
        int testTime = 10;
        double cost = 0;
        int rows = 5;
        int cols = 4;
        int pieceWidth = 1280;
        int pieceHeight = 768;
        MemoryStream[] memoryStreams = new MemoryStream[rows * cols];
        for (int i = 0; i < testTime; i++)
        {
            var startTime = DateTime.Now;
            Bitmap originalImage = new Bitmap(new MemoryStream(data));

            // 计算每个小块的宽度和高度
            for (int row = 0; row < rows; row++)
            {
                for (int col = 0; col < cols; col++)
                {
                    Rectangle pieceRect = new Rectangle(col * pieceWidth, row * pieceHeight, pieceWidth, pieceHeight);
                    memoryStreams[row * cols + col] = new();
                    var pieceImage = originalImage.Clone(pieceRect, originalImage.PixelFormat);
                    pieceImage.Save(memoryStreams[row * cols + col], ImageFormat.Jpeg);
                    pieceImage.Dispose();
                }
                cost += (DateTime.Now - startTime).TotalSeconds;
            }
        }
        for (int i = 0; i < memoryStreams.Length; i++)
        {
            File.WriteAllBytes($"{relativePath}sub/{i}.jpg", memoryStreams[i].ToArray());
        }
        Console.WriteLine($"avg {testTime} cost: {cost / testTime}, fps:{testTime / cost}");
    }
}
