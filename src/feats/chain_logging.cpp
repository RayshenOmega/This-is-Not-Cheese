#include "chain_logging.hpp"
#include "../hooks.hpp"
#include "../logger/chain/chain.hpp"

namespace Feats {
    namespace ChainLogging {
        bool enabled = false;
        int callStackSize = 3;
        bool showObjFullName = false;

        void init() {
            Hooks::registerHook(
                "*",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> uint8_t {
                    const auto functionName = pFunction->GetFullName().substr(9);
                    Logger::Chain::startCallLog(functionName, {{"objFullName", pObject->GetFullName()}});
                    return Hooks::ExecutionFlag::CONTINUE_EXECUTION;
                },
                Hooks::Type::PRE);

            Hooks::registerHook(
                "*",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> uint8_t {
                    const auto functionName = pFunction->GetFullName().substr(9);
                    Logger::Chain::endCallLog(functionName);
                    return Hooks::ExecutionFlag::CONTINUE_EXECUTION;
                },
                Hooks::Type::POST);
        }

        void tick() { return; }

        void menu() {
            if (ImGui::Checkbox("Chain Logging", &enabled)) {
                if (enabled) {
                    Logger::Chain::enable();
                } else {
                    Logger::Chain::disable();
                }
            }

            ImGui::SameLine();

            ImGui::PushItemWidth(100.0);
            if (ImGui::InputInt("Min Call Stack Size", &callStackSize)) {
                Logger::Chain::setMinCallStackSize((uint16_t)callStackSize);
            }
            ImGui::PopItemWidth();

            ImGui::SameLine();

            ImGui::Checkbox("Show Object Full Name", &showObjFullName);

            ImGui::SameLine();

            if (ImGui::Button("Clear Logs")) {
                Logger::Chain::clearLogs();
            }

            auto logs = Logger::Chain::getLogs();

            if (logs.empty()) {
                return;
            }

            for (uint64_t i = 1; auto &log : logs) {
                std::string header = "Log " + std::to_string(i);
                header += " (" + std::to_string(log.size()) + " calls)";

                if (ImGui::CollapsingHeader(header.c_str())) {
                    for (uint64_t i = 1; auto &call : log) {
                        ImGui::Indent();
                        auto entry = call.funcName;

                        if (showObjFullName) {
                            const auto index = call.attributes["objFullName"].find_first_of(" ");
                            const auto type = call.attributes["objFullName"].substr(0, index);

                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                            ImGui::Text(type.c_str());
                            ImGui::PopStyleColor();
                            ImGui::SameLine();

                            if (index != std::string::npos) {
                                const auto path = call.attributes["objFullName"].substr(index);

                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                                ImGui::Text(path.c_str());
                                ImGui::PopStyleColor();
                                ImGui::SameLine();
                            }
                        }

                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
                        ImGui::Text(call.funcName.c_str());
                        ImGui::PopStyleColor();
                    }

                    ImGui::Unindent(ImGui::GetStyle().IndentSpacing * log.size());
                }

                i++;
            }
        }
    } // namespace ChainLogging
} // namespace Feats
