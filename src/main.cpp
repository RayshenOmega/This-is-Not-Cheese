#include "main.hpp"
#include "config.hpp"
#include "feats/ability_failure.hpp"
#include "feats/anti_anti_cheat.hpp"
#include "feats/chain_logging.hpp"
#include "feats/display_poi.hpp"
#include "feats/early_init.hpp"
#include "feats/esp.hpp"
#include "feats/fov.hpp"
#include "feats/hotkey.hpp"
#include "feats/inf_dodge.hpp"
#include "feats/inf_jump.hpp"
#include "feats/jump_height.hpp"
#include "feats/login.hpp"
#include "feats/move_speed.hpp"
#include "feats/no_clip.hpp"
#include "feats/no_transparency.hpp"
#include "feats/quest.hpp"
#include "feats/rapid_attack.hpp"
#include "feats/resizer.hpp"
#include "feats/teleport_anywhere.hpp"
#include "feats/teleport_box.hpp"
#include "feats/teleport_nucleus.hpp"
#include "feats/uid_edit.hpp"
#include "globals.hpp"
#include "hooks.hpp"
#include "logger/logger.hpp"
#include "memory_manager.hpp"
#include "menu/menu.hpp"

std::vector<void *> registeredFeatures;

#define REGISTER_FEATURE(name)                                                                                         \
    name::init();                                                                                                      \
    registeredFeatures.push_back((void *)name::tick);

extern "C" __declspec(dllexport) void preMain(const wchar_t *dir) { Config::setDirectory(dir); }

int MainThread(HINSTANCE hInstDLL) {
    MemoryManager memory;
    const auto result = memory.PatternScan("40 56 57 48 83 EC 38 33 FF", 1);

    if (!result.empty()) {
        const auto addr = result[0];
        DWORD oldProtect;
        VirtualProtect(addr, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
        *(byte *)addr = 0xC3;
        VirtualProtect(addr, 1, oldProtect, &oldProtect);
    }

    bool hasWindow = false;
    while (hasWindow == false) {
        EnumWindows(
            [](HWND hWnd, LPARAM lParam) {
                DWORD pid;
                GetWindowThreadProcessId(hWnd, &pid);

                if (pid == GetCurrentProcessId()) {
                    *(bool *)lParam = true;
                    return FALSE;
                }

                return TRUE;
            },
            (long long)&hasWindow);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    const auto ntdll = GetModuleHandle(L"ntdll.dll");
    const auto dbgUiRemoteBreakin = (void **)GetProcAddress(ntdll, "DbgUiRemoteBreakin");
    DWORD oldProtect;
    VirtualProtect(dbgUiRemoteBreakin, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(dbgUiRemoteBreakin, "\x48\x83\xEC\x28\x65", 5);
    VirtualProtect(dbgUiRemoteBreakin, 5, oldProtect, &oldProtect);

    Logger::init();
    Logger::info("Initializing...");

    Config::init(hInstDLL);

    while (Globals::getInstance() == nullptr) {
        Logger::info("Waiting for game instance...");
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    Logger::success("Game instance found!");

    auto initEarly = Config::get<bool>(Feats::EarlyInit::confEnabled, false);

    if (*initEarly == false) {
        bool isAtLoginPage = false;
        while (true) {
            const auto world = Globals::getWorld();
            if (world->GetName() == "Login_P") {
                isAtLoginPage = true;
                Logger::info("Waiting for character to be loaded...");
            } else if (isAtLoginPage) {
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    Menu::init();
    Hooks::init();

    REGISTER_FEATURE(Feats::AntiAntiCheat);
    REGISTER_FEATURE(Feats::MoveSpeed);
    REGISTER_FEATURE(Feats::Fov);
    REGISTER_FEATURE(Feats::InfJump);
    REGISTER_FEATURE(Feats::TeleportNucleus);
    REGISTER_FEATURE(Feats::Quest);
    REGISTER_FEATURE(Feats::Login);
    REGISTER_FEATURE(Feats::TeleportAnywhere);
    REGISTER_FEATURE(Feats::ChainLogging);
    REGISTER_FEATURE(Feats::NoClip);
    REGISTER_FEATURE(Feats::UidEdit);
    REGISTER_FEATURE(Feats::Hotkey);
    REGISTER_FEATURE(Feats::TeleportBox);
    REGISTER_FEATURE(Feats::JumpHeight);
    REGISTER_FEATURE(Feats::DisplayPoi);
    REGISTER_FEATURE(Feats::Esp);
    REGISTER_FEATURE(Feats::RapidAttack);
    REGISTER_FEATURE(Feats::NoTransparency);
    REGISTER_FEATURE(Feats::Resizer);
    REGISTER_FEATURE(Feats::AbilityFailure);
    REGISTER_FEATURE(Feats::InfDodge);
    REGISTER_FEATURE(Feats::EarlyInit);

    while (true) {
        if (Feats::Hotkey::hotkeyPressed(confExit)) {
            break;
        }

        for (auto &feature : registeredFeatures) {
            ((void (*)())feature)();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Feats::Esp::shutdown();
    Hooks::shutdown();
    Menu::shutdown();
    Config::shutdown();
    Logger::shutdown();
    FreeLibraryAndExitThread(hInstDLL, 0);

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        HANDLE thread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hInstDLL, 0, nullptr);
        if (thread) {
            CloseHandle(thread);
        }
    }

    return true;
}
