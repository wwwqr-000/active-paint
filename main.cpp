#include <iostream>
#include <windows.h>
#include <vector>
#include <thread>
#include <fstream>
#include <sstream>

std::vector<std::thread> threads;

bool drawing = false;
bool active = true;
int colorR, colorG, colorB;
int colorIndex = -1;

HMODULE WDll = LoadLibraryExW(L"whiteavocado64.dll", nullptr, 0);

using DP = void (__cdecl*)(int, int, int, int, int, bool);
using KL = int (__cdecl*)(const char*, std::string&);
using GCP = void (__cdecl*)(int&, int&);
using K = void (__cdecl*)(const char*, int);
using MB = void (__cdecl*)(const char*, const char*, const char*, const char*, std::string&);
using QS = bool (__cdecl*)(std::string, std::string&);

DP const drawPixel = reinterpret_cast<DP>(GetProcAddress(WDll, "drawPixel"));
KL const keyListener = reinterpret_cast<KL>(GetProcAddress(WDll, "keyListener"));
GCP const getCursorPos = reinterpret_cast<GCP>(GetProcAddress(WDll, "getCursorPos"));
K const key = reinterpret_cast<K>(GetProcAddress(WDll, "key"));
MB const msgBox = reinterpret_cast<MB>(GetProcAddress(WDll, "msgBox"));
QS const quietShell = reinterpret_cast<QS>(GetProcAddress(WDll, "quietShell"));

void free() {
    active = false;
    Sleep(20);
    for (auto& thread : threads) {
        thread.join();
    }
    FreeLibrary(WDll);
}

void updateStat() {
    while (active) {
        std::cout << "";//Triggers thread closing listening inside this loop by executing random task
        if (!drawing) { continue; }
        int x, y;
        getCursorPos(x, y);
        drawPixel(x, y, colorR, colorG, colorB, false);
    }
}

int lineCount(std::string fileName) {
    std::string qsResult;
    quietShell(("find /v /c \"\" < " + fileName).c_str(), qsResult);
    return std::stoi(qsResult);
}

void toggleColor() {
    ++colorIndex;
    std::cout << "cIndex: " << colorIndex << "\n";
    if (colorIndex >= lineCount("color.list")) {
        colorIndex = 0;
    }
    std::ifstream colorFile("color.list");
    std::string line, num;
    std::vector<int> rgb;
    int lineIndex = -1;
    while (std::getline(colorFile, line)) {
        ++lineIndex;
        std::cout << "lIndex: " << lineIndex << "\n";
        if (lineIndex != colorIndex) { continue; }
        std::istringstream iss(line);
        while (std::getline(iss, num, '.')) {
            rgb.emplace_back(std::stoi(num));
        }
        colorR = rgb[0];
        colorG = rgb[1];
        colorB = rgb[2];
    }
}

int main() {
    threads.emplace_back([] { updateStat(); });
    std::string klResult, msgResult;
    toggleColor();//Load first color
    while (active) {
        int VKCode = keyListener("global", klResult);
        std::cout << VKCode << "\n";
        switch (VKCode) {
            case 162:
                drawing = !drawing;
            break;
            case 27:
                drawing = false;
                msgBox("Active Paint", "Are you sure you want to close Active Paint?", "yn", "q", msgResult);
                if (msgResult == "yes") { free(); }
            break;
            case 9:
                toggleColor();
            break;
        }
    }
    return 0;
}
