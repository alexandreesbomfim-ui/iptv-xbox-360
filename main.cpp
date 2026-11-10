#include <xtl.h>
#include <winsock2.h>
#include <vector>
#include <string>
#include <fstream>

struct Channel {
    std::string name;
    std::string url;
};

std::vector<Channel> g_channelList;
int g_selectedIndex = 0;

void ParseM3U(const std::string& filePath) {
    std::ifstream file(filePath.c_str());
    if (!file.is_open()) return;

    std::string line;
    Channel currentChannel;

    while (std::getline(file, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }

        if (line.find("#EXTINF:") == 0) {
            size_t commaPos = line.find(',');
            if (commaPos != std::string::npos) {
                currentChannel.name = line.substr(commaPos + 1);
            }
        } else if (!line.empty() && line[0] != '#') {
            currentChannel.url = line;
            g_channelList.push_back(currentChannel);
            currentChannel.name = "";
            currentChannel.url = "";
        }
    }
    file.close();
}

void PlayStream(const std::string& streamUrl) {
    wchar_t wUrl[512];
    mbstowcs(wUrl, streamUrl.c_str(), 512);

    XMP_MEDIA_SOURCE mediaSource;
    ZeroMemory(&mediaSource, sizeof(XMP_MEDIA_SOURCE));
    mediaSource.dwStructSize = sizeof(XMP_MEDIA_SOURCE);
    mediaSource.pszPath = wUrl;

    XMPPlayMedia(&mediaSource);
}

void __cdecl main() {
    XNetStartup(NULL);
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    ParseM3U("game:\\playlist.m3u");

    XINPUT_STATE inputState;

    while (true) {
        ZeroMemory(&inputState, sizeof(XINPUT_STATE));
        XInputGetState(0, &inputState);

        if (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
            if (g_selectedIndex < (int)g_channelList.size() - 1) {
                g_selectedIndex++;
                Sleep(150);
            }
        }

        if (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) {
            if (g_selectedIndex > 0) {
                g_selectedIndex--;
                Sleep(150);
            }
        }

        if (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
            if (!g_channelList.empty()) {
                PlayStream(g_channelList[g_selectedIndex].url);
                Sleep(500);
            }
        }

        if (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) {
            XLaunchNewImage(NULL, 0);
        }
    }

    WSACleanup();
    XNetCleanup();
}
