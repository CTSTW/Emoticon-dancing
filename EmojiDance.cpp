#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <atomic>
#include <functional>
#include <algorithm>
#include <cmath>

// 跨平台终端控制
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    
    class Terminal {
    private:
        HANDLE hConsole;
        CONSOLE_CURSOR_INFO cursorInfo;
        COORD prevSize;
        
    public:
        Terminal() {
            hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            cursorInfo.dwSize = 1;
            cursorInfo.bVisible = FALSE;
            SetConsoleCursorInfo(hConsole, &cursorInfo);
            
            // 保存原始大小
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(hConsole, &csbi);
            prevSize = csbi.dwSize;
            
            // 设置窗口大小
            SMALL_RECT rect = {0, 0, 119, 39};  // 120x40
            SetConsoleWindowInfo(hConsole, TRUE, &rect);
            
            COORD size = {120, 1000};  // 设置缓冲区高度
            SetConsoleScreenBufferSize(hConsole, size);
        }
        
        ~Terminal() {
            cursorInfo.bVisible = TRUE;
            SetConsoleCursorInfo(hConsole, &cursorInfo);
            SetConsoleScreenBufferSize(hConsole, prevSize);
        }
        
        void clear() {
            system("cls");
        }
        
        void setCursor(int x, int y) {
            COORD coord = {(SHORT)x, (SHORT)y};
            SetConsoleCursorPosition(hConsole, coord);
        }
        
        void setColor(int color) {
            SetConsoleTextAttribute(hConsole, color);
        }
        
        void resetColor() {
            SetConsoleTextAttribute(hConsole, 7);
        }
        
        std::pair<int, int> getSize() {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(hConsole, &csbi);
            return {csbi.srWindow.Right - csbi.srWindow.Left + 1,
                    csbi.srWindow.Bottom - csbi.srWindow.Top + 1};
        }
        
        bool kbhit() {
            return _kbhit() != 0;
        }
        
        char getch() {
            return _getch();
        }
    };
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    
    class Terminal {
    private:
        struct termios oldt, newt;
        
    public:
        Terminal() {
            tcgetattr(STDIN_FILENO, &oldt);
            newt = oldt;
            newt.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
            
            std::cout << "\033[?25l";  // 隐藏光标
        }
        
        ~Terminal() {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            std::cout << "\033[?25h";  // 显示光标
            std::cout << "\033[0m";    // 重置颜色
        }
        
        void clear() {
            std::cout << "\033[2J\033[1;1H";
        }
        
        void setCursor(int x, int y) {
            std::cout << "\033[" << y + 1 << ";" << x + 1 << "H";
        }
        
        void setColor(int color) {
            const char* colors[] = {
                "\033[0m",   "\033[31m", "\033[32m", "\033[33m",
                "\033[34m",  "\033[35m", "\033[36m", "\033[37m",
                "\033[90m",  "\033[91m", "\033[92m", "\033[93m",
                "\033[94m",  "\033[95m", "\033[96m", "\033[97m"
            };
            if (color >= 0 && color < 16) {
                std::cout << colors[color];
            }
        }
        
        void resetColor() {
            std::cout << "\033[0m";
        }
        
        std::pair<int, int> getSize() {
            struct winsize w;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            return {w.ws_col, w.ws_row};
        }
        
        bool kbhit() {
            struct termios oldt, newt;
            int ch;
            int oldf;
            
            tcgetattr(STDIN_FILENO, &oldt);
            newt = oldt;
            newt.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
            oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
            
            ch = getchar();
            
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            fcntl(STDIN_FILENO, F_GETFL, oldf);
            
            if (ch != EOF) {
                ungetc(ch, stdin);
                return true;
            }
            
            return false;
        }
        
        char getch() {
            return getchar();
        }
    };
#endif

// ==================== 颜文字跳舞系统 ====================

class EmojiDancer {
private:
    struct Dancer {
        // 位置
        float x, y;
        float vx, vy;
        
        // 动画状态
        int frame;
        float animationSpeed;
        float size;
        
        // 外观
        std::vector<std::string> frames;
        int color;
        char type;  // 'c'猫, 'b'熊, 'r'兔子, 's'星星
        
        // 物理属性
        float mass;
        float bounce;
        
        // 边界框
        float width, height;
        
        Dancer(float startX, float startY, char t) 
            : x(startX), y(startY), vx(0), vy(0), 
              frame(0), animationSpeed(0.1f + (rand() % 10) * 0.05f),
              size(1.0f + (rand() % 5) * 0.2f), type(t), 
              mass(1.0f + (rand() % 10) * 0.1f), bounce(0.7f + (rand() % 4) * 0.1f) {
            
            color = 1 + rand() % 15;  // 1-15 的颜色
            
            initializeFrames();
            calculateBounds();
        }
        
        void initializeFrames() {
            switch(type) {
                case 'c': // 猫咪
                    frames = {
                        "(=^･ω･^=)",
                        "(=^･ｪ･^=)",
                        "(=^◕ᴥ◕^=)",
                        "(=^‥^=)",
                        "(=^..^=)",
                        "(=^● ⋏ ●^=)"
                    };
                    break;
                    
                case 'b': // 小熊
                    frames = {
                        "ʕ•ᴥ•ʔ",
                        "ʕ·ᴥ·ʔ",
                        "ʕ º ᴥ ºʔ",
                        "ʕっ•ᴥ•ʔっ",
                        "ʕ•ﻌ•ʔ",
                        "ʕ≧ᴥ≦ʔ"
                    };
                    break;
                    
                case 'r': // 兔子
                    frames = {
                        "／(≧ x ≦)＼",
                        "／(･ × ･)＼",
                        "／(^ x ^)＼",
                        "／(° × °)＼",
                        "／(˘･ᴗ･˘)＼",
                        "／(◕ ⋏ ◕)＼"
                    };
                    break;
                    
                case 's': // 星星/特殊
                    frames = {
                        "☆*:.｡.o(≧▽≦)o.｡.:*☆",
                        "★~(◠‿◕✿)",
                        "✧*。ヾ(｡>ω<｡)ﾉﾞ✧*。",
                        "♪♪＼(^ω^＼)( /^ω^)/♪♪",
                        "꒰ᵕ༚ᵕ꒱˖♡",
                        "٩(◕‿◕｡)۶"
                    };
                    break;
            }
            
            // 确保每帧宽度一致
            for (auto& frame : frames) {
                if (frame.length() < 10) {
                    frame = std::string(10 - frame.length(), ' ') + frame;
                }
            }
        }
        
        void calculateBounds() {
            if (!frames.empty()) {
                width = frames[0].length() * size;
                height = 1 * size;
            } else {
                width = 10;
                height = 1;
            }
        }
        
        void update(float deltaTime, float gravity, float wind, 
                   float terminalWidth, float terminalHeight) {
            // 应用重力
            vy += gravity * deltaTime;
            
            // 应用风力（随机变化）
            vx += (wind + ((rand() % 100) - 50) * 0.01f) * deltaTime;
            
            // 添加一些随机运动
            vx += ((rand() % 100) - 50) * 0.001f;
            vy += ((rand() % 100) - 50) * 0.001f;
            
            // 更新位置
            x += vx * deltaTime * 60.0f;
            y += vy * deltaTime * 30.0f;
            
            // 边界碰撞
            if (x < 0) {
                x = 0;
                vx = -vx * bounce;
            } else if (x + width > terminalWidth) {
                x = terminalWidth - width;
                vx = -vx * bounce;
            }
            
            if (y < 0) {
                y = 0;
                vy = -vy * bounce;
            } else if (y + height > terminalHeight) {
                y = terminalHeight - height;
                vy = -vy * bounce;
                // 地面摩擦
                vx *= 0.9f;
            }
            
            // 更新动画帧
            static float frameAccumulator = 0;
            frameAccumulator += deltaTime * animationSpeed * 10.0f;
            if (frameAccumulator >= 1.0f) {
                frame = (frame + 1) % frames.size();
                frameAccumulator -= 1.0f;
            }
            
            // 速度衰减
            vx *= 0.99f;
            vy *= 0.99f;
        }
        
        void draw(Terminal& term) {
            int screenX = static_cast<int>(x);
            int screenY = static_cast<int>(y);
            
            term.setCursor(screenX, screenY);
            term.setColor(color);
            
            // 绘制当前帧
            std::cout << frames[frame];
            
            term.resetColor();
        }
        
        bool collidesWith(const Dancer& other) {
            return !(x + width < other.x ||
                    other.x + other.width < x ||
                    y + height < other.y ||
                    other.y + other.height < y);
        }
        
        void resolveCollision(Dancer& other) {
            // 简单的弹性碰撞
            float dx = (other.x + other.width/2) - (x + width/2);
            float dy = (other.y + other.height/2) - (y + height/2);
            
            float distance = std::sqrt(dx*dx + dy*dy);
            if (distance == 0) return;
            
            dx /= distance;
            dy /= distance;
            
            // 分离
            float overlap = (width/2 + other.width/2) - distance;
            x -= dx * overlap * 0.5f;
            y -= dy * overlap * 0.5f;
            other.x += dx * overlap * 0.5f;
            other.y += dy * overlap * 0.5f;
            
            // 交换速度
            float tempVx = vx;
            float tempVy = vy;
            vx = other.vx * bounce;
            vy = other.vy * bounce;
            other.vx = tempVx * other.bounce;
            other.vy = tempVy * other.bounce;
        }
    };
    
    // 性能优化的舞蹈池
    class DancerPool {
    private:
        std::vector<Dancer> dancers;
        std::vector<std::vector<int>> grid;
        int gridWidth, gridHeight;
        float cellSize;
        
    public:
        DancerPool() : gridWidth(0), gridHeight(0), cellSize(20.0f) {}
        
        void initialize(int width, int height) {
            gridWidth = static_cast<int>(width / cellSize) + 1;
            gridHeight = static_cast<int>(height / cellSize) + 1;
            grid.resize(gridWidth * gridHeight);
        }
        
        void addDancer(const Dancer& dancer) {
            dancers.push_back(dancer);
        }
        
        void updateGrid(float terminalWidth, float terminalHeight) {
            // 清空网格
            for (auto& cell : grid) {
                cell.clear();
            }
            
            // 将舞者分配到网格
            for (int i = 0; i < dancers.size(); ++i) {
                int gx = static_cast<int>(dancers[i].x / cellSize);
                int gy = static_cast<int>(dancers[i].y / cellSize);
                
                if (gx >= 0 && gx < gridWidth && gy >= 0 && gy < gridHeight) {
                    grid[gy * gridWidth + gx].push_back(i);
                }
            }
        }
        
        void update(float deltaTime, float gravity, float wind, 
                   float terminalWidth, float terminalHeight) {
            // 更新所有舞者
            for (auto& dancer : dancers) {
                dancer.update(deltaTime, gravity, wind, terminalWidth, terminalHeight);
            }
            
            // 更新网格
            updateGrid(terminalWidth, terminalHeight);
            
            // 基于网格的碰撞检测（性能优化）
            #pragma omp parallel for schedule(dynamic)
            for (int i = 0; i < dancers.size(); ++i) {
                int gx = static_cast<int>(dancers[i].x / cellSize);
                int gy = static_cast<int>(dancers[i].y / cellSize);
                
                // 检查相邻网格
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int ngx = gx + dx;
                        int ngy = gy + dy;
                        
                        if (ngx >= 0 && ngx < gridWidth && ngy >= 0 && ngy < gridHeight) {
                            int cellIndex = ngy * gridWidth + ngx;
                            
                            for (int otherIndex : grid[cellIndex]) {
                                if (otherIndex > i && dancers[i].collidesWith(dancers[otherIndex])) {
                                    dancers[i].resolveCollision(dancers[otherIndex]);
                                }
                            }
                        }
                    }
                }
            }
        }
        
        void draw(Terminal& term) {
            // 按Y坐标排序，确保正确的绘制顺序
            std::vector<int> indices(dancers.size());
            for (int i = 0; i < dancers.size(); ++i) indices[i] = i;
            
            std::sort(indices.begin(), indices.end(), 
                [&](int a, int b) { return dancers[a].y < dancers[b].y; });
            
            for (int idx : indices) {
                dancers[idx].draw(term);
            }
        }
        
        size_t size() const { return dancers.size(); }
        
        void applyForce(float fx, float fy) {
            for (auto& dancer : dancers) {
                dancer.vx += fx / dancer.mass;
                dancer.vy += fy / dancer.mass;
            }
        }
        
        void clear() { dancers.clear(); }
    };
    
    // 系统变量
    Terminal term;
    DancerPool dancerPool;
    std::atomic<bool> running{true};
    
    // 环境参数
    float gravity = 0.1f;
    float wind = 0.0f;
    float windChangeTimer = 0.0f;
    
    // 性能统计
    int fps = 0;
    int frameCount = 0;
    auto lastFpsTime = std::chrono::steady_clock::now();
    
public:
    EmojiDancer() {
        srand(static_cast<unsigned>(time(nullptr)));
    }
    
    void initialize(int dancerCount = 50) {
        auto [width, height] = term.getSize();
        
        dancerPool.initialize(width, height);
        
        // 创建舞者
        char types[] = {'c', 'b', 'r', 's'};
        for (int i = 0; i < dancerCount; ++i) {
            float x = rand() % (width - 20);
            float y = rand() % (height - 5);
            char type = types[rand() % 4];
            
            dancerPool.addDancer(Dancer(x, y, type));
        }
        
        std::cout << "初始化完成: " << dancerCount << " 个颜文字舞者" << std::endl;
    }
    
    void run() {
        // 输入处理线程
        std::thread inputThread([this]() {
            while (running) {
                if (term.kbhit()) {
                    char c = term.getch();
                    handleInput(c);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        
        // 主渲染循环
        auto lastTime = std::chrono::steady_clock::now();
        
        while (running) {
            auto currentTime = std::chrono::steady_clock::now();
            auto deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            
            // 更新FPS计数
            updateFPS(deltaTime);
            
            // 更新风
            windChangeTimer += deltaTime;
            if (windChangeTimer > 3.0f) {
                wind = ((rand() % 100) - 50) * 0.02f;
                windChangeTimer = 0.0f;
            }
            
            // 更新舞者
            auto [width, height] = term.getSize();
            dancerPool.update(deltaTime, gravity, wind, width, height);
            
            // 渲染
            term.clear();
            drawBackground(width, height);
            dancerPool.draw(term);
            drawUI(width);
            
            // 控制帧率
            auto frameTime = std::chrono::steady_clock::now() - currentTime;
            auto targetFrameTime = std::chrono::milliseconds(16); // ~60 FPS
            
            if (frameTime < targetFrameTime) {
                std::this_thread::sleep_for(targetFrameTime - frameTime);
            }
        }
        
        inputThread.join();
    }
    
private:
    void handleInput(char c) {
        switch(c) {
            case 'q':
            case 'Q':
            case 27:  // ESC
                running = false;
                break;
                
            case ' ':
                // 随机推动所有舞者
                dancerPool.applyForce(((rand() % 100) - 50) * 0.5f, 
                                     ((rand() % 100) - 50) * 0.5f);
                break;
                
            case 'g':
                gravity = -gravity;
                break;
                
            case 'w':
                dancerPool.clear();
                initialize(50);
                break;
                
            case '+':
                // 添加更多舞者
                for (int i = 0; i < 10; ++i) {
                    auto [width, height] = term.getSize();
                    char types[] = {'c', 'b', 'r', 's'};
                    float x = rand() % (width - 20);
                    float y = rand() % (height - 5);
                    char type = types[rand() % 4];
                    dancerPool.addDancer(Dancer(x, y, type));
                }
                break;
                
            case '-':
                // 减少舞者（模拟）
                break;
        }
    }
    
    void updateFPS(float deltaTime) {
        frameCount++;
        
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<float>(now - lastFpsTime).count();
        
        if (elapsed >= 1.0f) {
            fps = static_cast<int>(frameCount / elapsed);
            frameCount = 0;
            lastFpsTime = now;
        }
    }
    
    void drawBackground(int width, int height) {
        // 绘制简单的背景
        term.setCursor(0, 0);
        term.setColor(8); // 灰色
        
        // 顶部边框
        std::cout << "╔";
        for (int i = 0; i < width - 2; ++i) std::cout << "═";
        std::cout << "╗\n";
        
        // 侧边框
        for (int i = 1; i < height - 1; ++i) {
            term.setCursor(0, i);
            std::cout << "║";
            term.setCursor(width - 1, i);
            std::cout << "║";
        }
        
        // 底部边框
        term.setCursor(0, height - 1);
        std::cout << "╚";
        for (int i = 0; i < width - 2; ++i) std::cout << "═";
        std::cout << "╝";
        
        term.resetColor();
    }
    
    void drawUI(int width) {
        // 显示控制信息
        term.setCursor(2, 0);
        term.setColor(14); // 黄色
        std::cout << "✨ 颜文字舞蹈派对 ✨";
        
        term.setCursor(width - 30, 0);
        term.setColor(10); // 绿色
        std::cout << "FPS: " << fps << " 舞者: " << dancerPool.size();
        
        // 控制说明
        term.setCursor(2, 1);
        term.setColor(8); // 灰色
        std::cout << "空格: 推动  G: 反转重力  +/-: 增减舞者  W: 重置  Q: 退出";
        
        term.resetColor();
    }
};

// ==================== 主程序 ====================

int main() {
    try {
        std::cout << "正在启动颜文字跳舞系统..." << std::endl;
        
        EmojiDancer app;
        app.initialize(100);  // 100个舞者
        
        std::cout << "按任意键开始，Q键退出..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        app.run();
        
        std::cout << "\n程序结束，感谢观看！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}