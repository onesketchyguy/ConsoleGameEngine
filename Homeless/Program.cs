using System;

namespace Homeless
{
    internal class GameExample : ConsoleEngine
    {
        internal override bool Start()
        {
            for (int i = 0; i < buffer.Length; i++)
            {
                int x = i % bufferWidth;
                int y = i / bufferWidth;

                SetBackColor((ConsoleColor)(random.Next(0, int.MaxValue) % 15), x, y);
            }

            DrawString("Hello world!", 10, 10);
            for (int i = 0; i < 13; i++)
            {
                SetBackColor(ConsoleColor.Black, 10 + i, 10);
                SetFrontColor(ConsoleColor.White, 10 + i, 10);
            }

            bufferChanged = true;

            return base.Start();
        }

        internal override bool Update(float elapsedTime)
        {
            base.Update(elapsedTime);

            if (GetKey(ConsoleKey.Enter))
            {
                for (int i = 0; i < buffer.Length; i++)
                {
                    int x = i % bufferWidth;
                    int y = i / bufferWidth;

                    SetBackColor((ConsoleColor)(random.Next(0, int.MaxValue) % 15), x, y);
                }

                for (int i = 0; i < 13; i++)
                {
                    SetBackColor(ConsoleColor.Black, 10 + i, 10);
                    SetFrontColor(ConsoleColor.White, 10 + i, 10);
                }

                bufferChanged = true;
            }

            if (GetKey(ConsoleKey.Escape))
            {
                return false;
            }

            return true;
        }
    }

    internal class Program
    {
        public static bool APPLICATION_RUNNING = true;

        private static void Main(string[] args)
        {
            // Start the engine
            GameExample console = new GameExample();
            console.ConstructConsole(80, 30, 10);
        }
    }
}