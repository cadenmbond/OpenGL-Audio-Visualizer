#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/ImGuiFileDialog.h"

#include <glm/glm.hpp>

#include <SFML/Audio.hpp>
#include <SFML/System.hpp>

#include "Square.h"

#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <GLFW/glfw3.h>

constexpr int windowWidth{ 1080 };
constexpr int windowHeight{ 800 };

constexpr int buttonWidth{ 100 };
constexpr int buttonHeight{ 30 };

constexpr int fileExtensionLength{ 4 };

constexpr int minAmplitudeThreshold{ 23000 };
constexpr int maxAmplitudeThreshold{ 41000 };
unsigned int sampleRate;
unsigned int channelCount;
int amplitudeThreshold{ 35000 };

float currentTime;

struct GL_COLORS {
    static constexpr glm::vec3 Blue{ 0.0f, 0.0f, 1.0f };
    static constexpr glm::vec3 Yellow{ 1.0f, 1.0f, 0.0f };
    static constexpr glm::vec3 Red{ 1.0f, 0.0f, 0.0f };
    static constexpr glm::vec3 Orange{ 1.0f, 0.5f, 0.0f };
    static constexpr glm::vec3 Green{ 0.0f, 1.0f, 0.0f };
};

struct GL_ORTHO {
    static constexpr GLdouble left{ 0.0 };
    static constexpr GLdouble right{ windowWidth };
    static constexpr GLdouble bottom{ windowHeight };
    static constexpr GLdouble top{ 0.0 };
    static constexpr GLdouble zNear{ -1.0 };
    static constexpr GLdouble zFar{ 1.0 };
};

constexpr int squareRandomPosOffset{ 50 };
std::vector<Square> Squares;

int main(void)
{
    GLFWwindow* window;

    sf::SoundBuffer SoundBuffer;
    sf::Sound Music(SoundBuffer);
    std::string SongName;

    size_t prevSampleIndex{ 0 };

    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Randomized seed for squares spawning

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(windowWidth, windowHeight, "Music Visualizer", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(GL_ORTHO::left, GL_ORTHO::right, GL_ORTHO::bottom, GL_ORTHO::top, GL_ORTHO::zNear, GL_ORTHO::zFar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Visualizer Tweaks");
        ImGui::Text("Add a song:");

        if (ImGui::Button("Search for Song")) {
            IGFD::FileDialogConfig config;
            config.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".wav", config);
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (SoundBuffer.loadFromFile(filePathName)) {
                    Music.stop();

                    Music.setBuffer(SoundBuffer);

                    SongName = filePathName.substr(filePath.length() + 1); // Extract Just File Name
                    SongName.resize(SongName.length() - fileExtensionLength);

                    sampleRate = SoundBuffer.getSampleRate();
                    channelCount = SoundBuffer.getChannelCount();

                    // Clear old squares when loading a new song
                    Squares.clear();

                    Music.play();
                }
                else {
                    std::cerr << "Failed to load audio file: " << filePathName << std::endl;
                }

            }

            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::SliderInt("Amplitude Threshold", &amplitudeThreshold, minAmplitudeThreshold, maxAmplitudeThreshold);

        ImGui::Text("Current Song: %s", SongName.c_str());
        ImGui::Text("Amplitude Threshold: %d", amplitudeThreshold);

        ImGui::End();

        currentTime = Music.getPlayingOffset().asSeconds();
        size_t sampleIndex = static_cast<size_t>(currentTime * sampleRate * channelCount);

        const std::int16_t* AudioSamples = SoundBuffer.getSamples();
        std::size_t SampleCount = SoundBuffer.getSampleCount();

        if (sampleIndex > SampleCount) sampleIndex = SampleCount;

        for (size_t i = prevSampleIndex; i < sampleIndex; ++i) {
            int16_t amplitude = AudioSamples[i];

            if (amplitude > amplitudeThreshold || amplitude < -amplitudeThreshold) {
                float x = static_cast<float>(std::rand() % (windowWidth - squareRandomPosOffset));
                float y = static_cast<float>(std::rand() % (windowHeight - squareRandomPosOffset));

                Square sq;
                sq.position = glm::vec2(x, y);
                sq.transparency = 1.0f;
                if (abs(amplitude) < amplitudeThreshold * 1.02f) {
                    sq.color = GL_COLORS::Blue;
                }
                else if (abs(amplitude) < amplitudeThreshold * 1.04f) {
                    sq.color = GL_COLORS::Green;
                }
                else if (abs(amplitude) < amplitudeThreshold * 1.06f) {
                    sq.color = GL_COLORS::Yellow;
                }
                else if (abs(amplitude) < amplitudeThreshold * 1.08f) {
                    sq.color = GL_COLORS::Orange;
                }
                else {
                    sq.color = GL_COLORS::Red;
                }


                Squares.push_back(sq);
            }
        }

        prevSampleIndex = sampleIndex - 1;


        for (int i = 0; i < Squares.size(); ++i) {
            Squares[i].update(0.1f);

            if (Squares[i].transparency <= 0.0f) {
                Squares.erase(Squares.begin() + i);
                i--;
            }
        }

        for (const auto& sq : Squares) {
            sq.draw();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
