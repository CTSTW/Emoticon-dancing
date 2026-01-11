#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif

class SimpleEmojiDancer {
private:
    struct Dancer {
        int x, y;
        int dx, dy;
        std::string emoji;
        int color;
        
        Dancer(int w, int h) {
            static std::vector<std::string> emojis = {
                "(=^･ω･^=)", "ʕ•ᴥ•ʔ", "／(≧ x ≦)＼", 
                "☆*:.｡.o(≧▽≦)o.｡.:*☆", "٩(◕‿◕｡)۶", "꒰ᵕ༚ᵕ꒱˖♡"
            };
            
            x = rand() % (w - 10);
            y = rand() % (h - 2);
            dx = (rand() % 3) - 1;
            dy = (rand() % 3) - 1;
            emoji = emojis[rand() % emojis.size()];
            color = 1 + rand() % 6; // 1-6 基本颜色
        }
        
        void update(int width, int height) {
            x += dx;
            y += dy;
            
            if (x <= 0 || x >= width - 10) dx = -dx;
            if (y <= 0 || y >= height - 1) dy = -dy;
            
            // 偶尔改变方向
            if (rand() % 100 < 5) {
                dx = (rand() % 3) - 1;
                dy = (rand() % 3) - 1;
            }
        }
    };
    
    std::vector<Dancer> dancers;
    bool running = true;
    int width = 80;
    int height = 24;
    
public:
    SimpleEmojiDancer(int count = 30) {
        #ifdef _WIN32
        system("cls");
        #else
        system("clear");
        #endif
        
        // 获取终端大小
        getTerminalSize();
        
        for (int i = 0; i < count; ++i) {
            dancers.emplace_back(width, height);
        }
    }
    
    void getTerminalSize() {
        #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        #else
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        width = w.ws_col;
        height = w.ws_row;
        #endif
    }
    
    void run() {
        while (running) {
            // 更新
            for (auto& dancer : dancers) {
                dancer.update(width, height);
            }
            
            // 渲染
            #ifdef _WIN32
            system("cls");
            #else
            system("clear");
            #endif
            
            // 清屏并重新绘制
            for (const auto& dancer : dancers) {
                #ifdef _WIN32
                SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
                                         {static_cast<SHORT>(dancer.x), 
                                          static_cast<SHORT>(dancer.y)});
                #else
                std::cout << "\033[" << dancer.y << ";" << dancer.x << "H";
                #endif
                
                // 设置颜色
                #ifdef _WIN32
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), dancer.color);
                #else
                std::cout << "\033[3" << dancer.color << "m";
                #endif
                
                std::cout << dancer.emoji;
                
                #ifndef _WIN32
                std::cout << "\033[0m";
                #endif
            }
            
            // 显示信息
            #ifdef _WIN32
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
            #else
            std::cout << "\033[1;1H\033[33m";
            #endif
            
            std::cout << "颜文字跳舞 (" << dancers.size() << "个) - 按Q退出";
            
            #ifndef _WIN32
            std::cout << "\033[0m";
            #endif
            
            // 检查输入
            #ifdef _WIN32
            if (_kbhit()) {
                char c = _getch();
                if (c == 'q' || c == 'Q') running = false;
            }
            #endif
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};

int main() {
    SimpleEmojiDancer dancer(30);
    dancer.run();
    return 0;
}