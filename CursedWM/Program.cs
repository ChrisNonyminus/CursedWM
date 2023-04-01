using CursedWM;

class Program
{
    static void Main(string[] args)
    {
        var WM = new WindowManager(SimpleLogger.LogLevel.Debug);
        WM.Run();
    }
}
