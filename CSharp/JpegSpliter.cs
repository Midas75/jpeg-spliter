public class SpliterParam
{
    public int SingleWidth { get; set; }  // 1280
    public int SingleHeight { get; set; } // 768
    public int Col { get; set; }          // 4
    public int Row { get; set; }          // 5
    public int Dri { get; set; }          // 8
    public bool Trim { get; set; }        // true
}

public class Spliter
{
    const int MCU_LENGTH = 8;

    private static int GetMcuSub(int counter, SpliterParam param)
    {
        int c = param.SingleWidth * param.Col / param.Dri / MCU_LENGTH;
        int x = counter % c * param.Dri * MCU_LENGTH / param.SingleWidth;
        int y = counter * param.Dri / c / param.SingleHeight;
        return x + y * param.Col;
    }

    public static double Split(byte[] data, int length, SpliterParam param, List<byte>[] output)
    {
        var startTime = DateTime.Now;

        int count = param.Col * param.Row;
        for (int i = 0; i < count; i++)
        {
            if (output[i] != null)
            {
                output[i].Clear();
            }
            else
            {
                output[i] = new List<byte>((length / count) << 2);
            }
        }

        int[] ffda = new int[4];
        int ffdaCount = 0;

        for (int i = 0; i < length - 1; ++i)
        {
            if (data[i] == 0xFF && data[i + 1] == 0xDA)
            {
                ffda[ffdaCount++] = i;
                break;
            }
        }

        for (int i = 0; i < ffda[0]; i += 2)
        {
            if (data[i] == 0xFF && data[i + 1] == 0xC0)
            {
                int heightOffset = 2 + 2 + 1;
                data[i + heightOffset] = (byte)((param.SingleHeight >> 8) & 0xFF);
                data[i + heightOffset + 1] = (byte)(param.SingleHeight & 0xFF);
                data[i + heightOffset + 2] = (byte)((param.SingleWidth >> 8) & 0xFF);
                data[i + heightOffset + 3] = (byte)(param.SingleWidth & 0xFF);
                break;
            }
        }

        for (int i = 0; i < count; ++i)
        {
            output[i].AddRange(data[..ffda[0]]);
        }

        int ffdaIndex = 0;

        while (ffdaIndex < ffdaCount)
        {
            int ffdaPos = ffda[ffdaIndex];
            int startMcu = ffdaPos + 8 + 2;
            byte[] subRsti = new byte[20];
            int counter = 0;
            int lastI = startMcu;

            for (int i = 0; i < count; ++i)
            {
                output[i].AddRange(data[ffdaPos..startMcu]);
            }

            for (int i = startMcu; i < length; ++i)
            {
                if (data[i] == 0xFF)
                {
                    int sub = GetMcuSub(counter, param);
                    if (data[i + 1] >= 0xD0 && data[i + 1] <= 0xD7)
                    {
                        data[i + 1] = (byte)(0xD0 + (subRsti[sub]++));
                        subRsti[sub] %= 8;
                        output[sub].AddRange(data[lastI..(i + 2)]);
                        counter++;
                        lastI = i + 2;
                    }
                    else if (data[i + 1] == 0xDA)
                    {
                        output[sub].AddRange(data[lastI..i]);
                        ffda[ffdaCount++] = i;
                        break;
                    }
                    else if (data[i + 1] == 0xD9)
                    {
                        output[sub].AddRange(data[lastI..i]);
                        break;
                    }
                }
            }
            ffdaIndex++;
        }

        for (int i = 0; i < count; ++i)
        {
            output[i].Add(0xFF);
            output[i].Add(0xD9);
            if (param.Trim)
            {
                output[i].TrimExcess();
            }
        }

        double result = (DateTime.Now - startTime).TotalSeconds;
        return result;
    }
}
