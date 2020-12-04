using System;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;

namespace Homeless
{
    public enum PIXEL_TYPE
    {
        PIXEL_SOLID = (char)0x2588,
        PIXEL_THREEQUARTERS = (char)0x2593,
        PIXEL_HALF = (char)0x2592,
        PIXEL_QUARTER = (char)0x2591,
    };

    public struct Pixel
    {
        public char character;
        public ConsoleColor foreColor;
        public ConsoleColor backColor;
    }

    public class ConsoleEngine
    {
        public bool APPLICATION_RUNNING = true;

        public string AppTitle;
        public string AppBranding;

        public System.Random random;

        internal int bufferWidth, bufferHeight, bufferLength;

        private Stopwatch time;

        internal Pixel[] buffer;

        private TextWriter textWriter;

        internal bool bufferChanged = true;

        private readonly Queue<ConsoleKeyInfo> messageQueue = new Queue<ConsoleKeyInfo>();
        private readonly HashSet<ConsoleKey> keys = new HashSet<ConsoleKey>();

        /// <summary>
        /// Create a console to play around with.
        /// </summary>
        /// <param name="width">Max value 200</param>
        /// <param name="height">Max value 50</param>
        public void ConstructConsole(byte width, byte height, short fontSize = 10)
        {
            CreateWindow(width, height);

            ConsoleHelper.SetCurrentFont("Raster Fonts", fontSize);

            StartKeyboardListener();
            Console.Title = "Application Alpha";
            Console.CursorVisible = false;
            Console.TreatControlCAsInput = true;

            random = new Random();

            for (int i = 0; i < buffer.Length; i++)
            {
                buffer[i].foreColor = (ConsoleColor)(random.Next(0, int.MaxValue) % 15);
            }

            time = new Stopwatch();
            time.Start();

            textWriter = Console.Out;

            Start();

            var mainThread = new Thread(() =>
            {
                MessageLoop();
            });

            mainThread.IsBackground = false;
            mainThread.Start();
        }

        public void CreateWindow(short width, short height)
        {
            bufferLength = width * height;
            bufferWidth = width;
            bufferHeight = height;

            Console.SetWindowSize(1, 1);

            Console.SetBufferSize(width, height);
            Console.SetWindowSize(width, height);

            buffer = new Pixel[bufferLength];
        }

        #region Drawing

        public void ClearBuffer()
        {
            buffer = new Pixel[bufferLength];
        }

        public void DrawToBuffer(char val, int x, int y)
        {
            int index = y * bufferWidth + x;
            if (index >= buffer.Length || index < 0) return;

            buffer[index].character = val;
            bufferChanged = true;
        }

        public void SetBackColor(ConsoleColor col, int x, int y)
        {
            int index = y * bufferWidth + x;
            if (index >= buffer.Length || index < 0) return;

            buffer[index].backColor = col;
        }

        public void SetFrontColor(ConsoleColor col, int x, int y)
        {
            int index = y * bufferWidth + x;
            if (index >= buffer.Length || index < 0) return;

            buffer[index].foreColor = col;
        }

        public void DrawToBuffer(PIXEL_TYPE val, int x, int y)
        {
            int index = y * bufferWidth + x;
            if (index >= buffer.Length || index < 0) return;

            buffer[index].character = (char)val;
            bufferChanged = true;
        }

        public void DrawString(string val, int x, int y)
        {
            for (int i = 0; i < val.Length; i++)
            {
                int index = y * bufferWidth + (x + i);

                if (index >= buffer.Length || index < 0) break;

                char c = val[i];
                buffer[index].character = c;
            }

            bufferChanged = true;
        }

        private void DrawBuffer()
        {
            if (bufferChanged == false) return;

            Task drawBuffer = Task.Run(() =>
            {
                Console.Clear();

                for (int i = 0; i < buffer.Length; i++)
                {
                    Console.ForegroundColor = buffer[i].foreColor;
                    Console.BackgroundColor = buffer[i].backColor;

                    textWriter.WriteAsync(buffer[i].character);
                    //Console.Write(buffer[i].character);
                    Console.ResetColor();
                }
            });

            bufferChanged = false;
        }

        #endregion Drawing

        private void StartKeyboardListener()
        {
            var thread = new Thread(() =>
            {
                while (APPLICATION_RUNNING)
                {
                    ConsoleKeyInfo key = Console.ReadKey(true);
                    messageQueue.Enqueue(key);
                }
            });

            thread.IsBackground = true;
            thread.Start();
        }

        private void MessageLoop()
        {
            while (APPLICATION_RUNNING)
            {
                if (keys.Count > 0) keys.Clear();

                if (messageQueue.Count > 0)
                {
                    var message = messageQueue.Dequeue();

                    if (message != null)
                    {
                        if (message.Key != ConsoleKey.NoName)
                        {
                            if (keys.Contains(message.Key) == false)
                                keys.Add(message.Key);
                        }
                    }
                }

                time.Stop();
                DrawBuffer();

                Thread.Yield();

                APPLICATION_RUNNING = Update((float)time.Elapsed.TotalMilliseconds);

                time.Restart();

                Console.Title = $"{AppBranding} - {AppTitle}";
            }
        }

        internal bool GetKey(ConsoleKey key)
        {
            return keys.Contains(key);
        }

        internal virtual bool Start()
        {
            AppTitle = "Application Alpha";
            return true;
        }

        internal virtual bool Update(float elapsedTime)
        {
            AppBranding = (elapsedTime).ToString();

            return true;
        }
    }

    public static class ConsoleHelper
    {
        private const int FixedWidthTrueType = 54;
        private const int StandardOutputHandle = -11;

        [DllImport("kernel32.dll", SetLastError = true)]
        internal static extern IntPtr GetStdHandle(int nStdHandle);

        [return: MarshalAs(UnmanagedType.Bool)]
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        internal static extern bool SetCurrentConsoleFontEx(IntPtr hConsoleOutput, bool MaximumWindow, ref FontInfo ConsoleCurrentFontEx);

        [return: MarshalAs(UnmanagedType.Bool)]
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        internal static extern bool GetCurrentConsoleFontEx(IntPtr hConsoleOutput, bool MaximumWindow, ref FontInfo ConsoleCurrentFontEx);

        private static readonly IntPtr ConsoleOutputHandle = GetStdHandle(StandardOutputHandle);

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct FontInfo
        {
            internal int cbSize;
            internal int FontIndex;
            internal short FontWidth;
            public short FontSize;
            public int FontFamily;
            public int FontWeight;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string FontName;
        }

        public static FontInfo[] SetCurrentFont(string font, short fontSize = 0)
        {
            Console.WriteLine("Set Current Font: " + font);

            FontInfo before = new FontInfo
            {
                cbSize = Marshal.SizeOf<FontInfo>()
            };

            if (GetCurrentConsoleFontEx(ConsoleOutputHandle, false, ref before))
            {
                FontInfo set = new FontInfo
                {
                    cbSize = Marshal.SizeOf<FontInfo>(),
                    FontIndex = 0,
                    FontFamily = FixedWidthTrueType,
                    FontName = font,
                    FontWeight = 400,
                    FontSize = fontSize > 0 ? fontSize : before.FontSize
                };

                // Get some settings from current font.
                if (!SetCurrentConsoleFontEx(ConsoleOutputHandle, false, ref set))
                {
                    var ex = Marshal.GetLastWin32Error();
                    Console.WriteLine("Set error " + ex);
                    throw new System.ComponentModel.Win32Exception(ex);
                }

                FontInfo after = new FontInfo
                {
                    cbSize = Marshal.SizeOf<FontInfo>()
                };
                GetCurrentConsoleFontEx(ConsoleOutputHandle, false, ref after);

                return new[] { before, set, after };
            }
            else
            {
                var er = Marshal.GetLastWin32Error();
                Console.WriteLine("Get error " + er);
                throw new System.ComponentModel.Win32Exception(er);
            }
        }
    }
}