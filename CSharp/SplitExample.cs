SpliterParam param = new SpliterParam
{
    SingleHeight = 768,
    SingleWidth = 1280,
    Col = 4,
    Row = 5,
    Dri = 8,
    Trim = false
};
string relativePath = "../../../../";
byte[] data = File.ReadAllBytes(relativePath+"unity_texture2d_big.jpg");
int length = data.Length;
List<byte>[] output = new List<byte>[param.Col*param.Row];
int testTime = 200;
double cost = 0;
for (int i = 0; i < testTime; i++)
{
    cost += Spliter.Split(data, length, param, output);
}
for(int i = 0; i < output.Length; i++)
{
    File.WriteAllBytes($"{relativePath}sub/{i}.jpg", output[i].ToArray());
}
Console.WriteLine($"avg {testTime} cost: {cost/testTime}, fps:{testTime/cost}");