#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>

class ImGuiHUD {
public:
    ImGuiHUD(GLFWwindow* window) {
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        io = &ImGui::GetIO(); (void)io;
        io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
        

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();
    }

    ~ImGuiHUD() {
       
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void NewFrame() {

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("HUD");
        ImGui::Text("Health: %d", 100); // Placeholder for health
        ImGui::Text("Score: %d", 50);   // Placeholder for score
        if (ImGui::Button("Action")) {
            std::cout << "Action button pressed!" << std::endl;
        }
        ImGui::End();

        ImGui::Begin("Debugger");
        ImGui::Text("FPS: %.1f", io->Framerate); // Display frame rate
        static bool show_debug_info = false;
        ImGui::Checkbox("Show Debug Info", &show_debug_info); // Placeholder checkbox
        ImGui::End();
    }

    void Render() {

        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void SetShowDebugInfo(bool show) {
        showDebugInfo = show;
    }

    bool GetShowDebugInfo() const {
        return showDebugInfo;
    }

private:
    ImGuiIO* io;
    bool showDebugInfo = false;
};
