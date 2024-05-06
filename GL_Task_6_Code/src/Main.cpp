/*
 * Copyright 2023 Vienna University of Technology.
 * Institute of Computer Graphics and Algorithms.
 * This file is part of the GCG Lab Framework and must not be redistributed.
 */

#include "Utils.h"
#include <sstream>
#include "Shader.h"
#include "Material.h"
#include "Light.h"
#include "Texture.h"
#include "Model.h"
#include <filesystem>
#include "Skybox.h"
#include "Player.h"
#include "ArcCamera.h"
#include "physics/PhysXInitializer.h"

#undef min
#undef max

/* --------------------------------------------- */
// Prototypes
/* --------------------------------------------- */

static void APIENTRY DebugCallbackDefault(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const GLvoid *userParam
);
static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char *msg);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void setPerFrameUniforms(Shader* shader, ArcCamera& camera, DirectionalLight& dirL, PointLight& pointL);

/* --------------------------------------------- */
// Global variables
/* --------------------------------------------- */

static bool _wireframe = false;
static bool _culling = false;

static bool _draw_normals = false;
static bool _draw_texcoords = false;

static bool _dragging = true;
static bool _strafing = false;
static float _zoom = 5.0f;

Player player1;
ArcCamera camera;
bool firstMouse = true;
static float PI = 3.14159265358979;

PxPhysics* physics = PhysXInitializer::initializePhysX();
PxScene* scene = PhysXInitializer::createPhysXScene(physics);

/* --------------------------------------------- */
// Main
/* --------------------------------------------- */

int main(int argc, char** argv) {
    
    CMDLineArgs cmdline_args;
    gcgParseArgs(cmdline_args, argc, argv);

    /* --------------------------------------------- */
    // Load settings.ini
    /* --------------------------------------------- */

    INIReader window_reader("assets/settings/window.ini");

    int window_width = 1920;
    int window_height = 1080;
    int refresh_rate = window_reader.GetInteger("window", "refresh_rate", 60);
    bool fullscreen = window_reader.GetBoolean("window", "fullscreen", false);
    std::string window_title = "Echoes of the Labyrinth";
    std::string init_camera_filepath = "assets/settings/camera_front.ini";
    if (cmdline_args.init_camera) {
        init_camera_filepath = cmdline_args.init_camera_filepath;
    }
    INIReader camera_reader(init_camera_filepath);

    float fov = 60.0f;
    float nearZ = 0.1f;
    float farZ = 100.0f;
    float camera_yaw = 0.0f;
    float camera_pitch = 0.0f;

    std::string init_renderer_filepath = "assets/settings/renderer_standard.ini";
    INIReader renderer_reader(init_renderer_filepath);

    _wireframe = renderer_reader.GetBoolean("renderer", "wireframe", false);
    _culling = renderer_reader.GetBoolean("renderer", "backface_culling", false);
    _draw_normals = renderer_reader.GetBoolean("renderer", "normals", false);
    _draw_texcoords = renderer_reader.GetBoolean("renderer", "texcoords", false);
    bool _depthtest = renderer_reader.GetBoolean("renderer", "depthtest", true);

    glm::mat4 projection = glm::perspective(radians(fov), (float)window_width / (float)window_height, nearZ, farZ);
    glm::mat4 viewProjectionMatrix = mat4(1.0f);

    /* --------------------------------------------- */
    // Create context
    /* --------------------------------------------- */

    glfwSetErrorCallback([](int error, const char* description) { std::cout << "GLFW error " << error << ": " << description << std::endl; });

    if (!glfwInit()) {
        EXIT_WITH_ERROR("Failed to init GLFW");
    }
    std::cout << "GLFW was initialized." << std::endl;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // Request OpenGL version 4.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Request core profile
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);            // Create an OpenGL debug context
    glfwWindowHint(GLFW_REFRESH_RATE, refresh_rate);               // Set refresh rate
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Enable antialiasing (4xMSAA)
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Open window
    GLFWmonitor* monitor = nullptr;

    if (fullscreen)
        monitor = glfwGetPrimaryMonitor();

    GLFWwindow* window = glfwCreateWindow(window_width, window_height, window_title.c_str(), monitor, nullptr);

    if (!window)
        EXIT_WITH_ERROR("Failed to create window");

    // This function makes the context of the specified window current on the calling thread.
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true;
    GLenum err = glewInit();

    // If GLEW wasn't initialized
    if (err != GLEW_OK) {
        EXIT_WITH_ERROR("Failed to init GLEW: " << glewGetErrorString(err));
    }
    std::cout << "GLEW was initialized." << std::endl;

    // Debug callback
    if (glDebugMessageCallback != NULL) {
        // Register your callback function.

        glDebugMessageCallback(DebugCallbackDefault, NULL);
        // Enable synchronous callback. This ensures that your callback function is called
        // right after an error has occurred. This capability is not defined in the AMD
        // version.
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }

    


    /* --------------------------------------------- */
    // Init framework
    /* --------------------------------------------- */

    if (!initFramework()) {
        EXIT_WITH_ERROR("Failed to init framework");
    }
    std::cout << "Framework was initialized." << std::endl;

    // set callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // set GL defaults
    glClearColor(1.0, 0.8, 1.0, 1);
    if (_depthtest) {
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }
    if (_culling) {
        glEnable(GL_CULL_FACE);
    }
    if (_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    /* --------------------------------------------- */
    // Initialize scene and render loop
    /* --------------------------------------------- */
    {
        // Load shader(s)
        std::shared_ptr<Shader> cornellShader = std::make_shared<Shader>("assets/shaders/cornellGouraud.vert", "assets/shaders/cornellGouraud.frag");
        std::shared_ptr<Shader> textureShader = std::make_shared<Shader>("assets/shaders/texture.vert", "assets/shaders/texture.frag");
        std::shared_ptr<Shader> modelShader = std::make_shared<Shader>("assets/shaders/model.vert", "assets/shaders/model.frag");
        std::shared_ptr<Shader> sky = std::make_shared<Shader>("assets/shaders/sky.vert", "assets/shaders/sky.frag");

        string path = gcgFindTextureFile("assets/geometry/maze/maze.obj");
        Model map(&path[0], physics, scene);
        Skybox skybox;

        string path1 = gcgFindTextureFile("assets/geometry/podest/podest.obj");
        Model podest(&path1[0], physics, scene);

        string path2 = gcgFindTextureFile("assets/geometry/floor/floor.obj");
        Model floor(&path2[0], physics, scene);

        string path3 = gcgFindTextureFile("assets/geometry/diamond/diamond.obj");
        Model diamond(&path3[0], physics, scene);

        string path4 = gcgFindTextureFile("assets/geometry/adventurer/adventurer.obj");
        Model adventurer(&path4[0], physics, scene);

        //diamond.printNormals();


        //Physics simulation;

        player1.set(adventurer, glm::vec3(0.0f, 0.0f, 0.0f), 0, 0, 0, 1);

        // Initialize camera
        camera.setCamParameters(fov, float(window_width) / float(window_height), nearZ, farZ, camera_yaw, camera_pitch);

        // Initialize lights
        DirectionalLight dirL(glm::vec3(1.0f), glm::vec3(0.0f, -1.0f, -1.0f));
        PointLight pointL(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 10.4f, 0.0f));

        // Render loop
        float t = float(glfwGetTime());
        float t_sum = 0.0f;
        float dt = 0.0f;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(6.0f, 6.0f, 6.0f));
        GLint modelLoc = glGetUniformLocation(modelShader->getHandle(), "model");

        glm::mat4 modelDiamiond = glm::mat4(1.0f);
        modelDiamiond = glm::translate(modelDiamiond, glm::vec3(0.0f, 0.0f, 0.0f));
        modelDiamiond = glm::scale(modelDiamiond, glm::vec3(3.0f, 3.0f, 3.0f));

        glm::mat4 podestModel = glm::mat4(1.0f);
        podestModel = glm::translate(podestModel, glm::vec3(0.0f, 0.0f, 0.0f));
        podestModel = glm::scale(podestModel, glm::vec3(3.0f, 3.0f, 3.0f));

        mat4 viewMatrix = camera.calculateMatrix(camera.getRadius(), camera.getPitch(), camera.getYaw(), player1);
        glm::vec3 camDir = camera.getPos();

        glm::mat4 play = glm::mat4(1.0f);
        play = glm::translate(play, player1.getPosition());
        play = glm::scale(play, glm::vec3(player1.getScale(), player1.getScale(), player1.getScale()));

        glm::vec3 prevCamDir = glm::vec3(0.0f, 0.0f, 0.0f);
        double prevRotation = 0.0f;
        double angle = 0.0f;

        glm::vec3 materialCoefficients = glm::vec3(0.1f, 0.7f, 0.1f);
        float alpha = 8.0f;

        while (!glfwWindowShouldClose(window)) {
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            modelShader->use();

            glfwPollEvents();

            viewMatrix = camera.calculateMatrix(camera.getRadius(), camera.getPitch(), camera.getYaw(), player1);
            camDir = camera.extractCameraDirection(viewMatrix);
            viewProjectionMatrix = projection * viewMatrix;

            modelShader->setUniform("viewProjMatrix", viewProjectionMatrix);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(play));
            modelShader->setUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(play))));
            modelShader->setUniform("materialCoefficients", materialCoefficients);
            modelShader->setUniform("specularAlpha", alpha);

            player1.checkInputs(window, dt, camDir);
            //player1.updateRotation(camDir);
            /*if (camDir != prevCamDir) {
                angle = player1.getRotY() * 0.00005f;
                std::cout << angle << endl;
                play = glm::rotate(play, float(angle), glm::vec3(0.0f, 1.0f, 0.0f));
            }*/
            play = glm::translate(play, player1.getPosition());

            prevRotation = angle;
            prevCamDir = camDir;

            player1.Draw(modelShader);


            // Set per-frame uniforms
            setPerFrameUniforms(modelShader.get(), camera, dirL, pointL);
            setPerFrameUniforms(modelShader.get(), camera, dirL, pointL);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            floor.Draw(modelShader);
            map.Draw(modelShader);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(podestModel));
            podest.Draw(modelShader);

            modelDiamiond = glm::rotate(modelDiamiond, glm::radians(0.1f), glm::vec3(0.0f, 1.0f, 0.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDiamiond));
            diamond.Draw(modelShader);

            scene->simulate(dt);
            scene->fetchResults();

            sky->use();
            sky->setUniform("viewProjMatrix", viewProjectionMatrix);
            skybox.draw();

            // Compute frame time
            dt = t;
            t = float(glfwGetTime());
            dt = (t - dt);
            t_sum += dt;

            // Swap buffers
            glfwSwapBuffers(window);
        }
    }

    /* --------------------------------------------- */
    // Destroy framework
    /* --------------------------------------------- */

    destroyFramework();

    /* --------------------------------------------- */
    // Destroy context and exit
    /* --------------------------------------------- */

    glfwTerminate();

    return EXIT_SUCCESS;
}

void setPerFrameUniforms(Shader* shader, ArcCamera& camera, DirectionalLight& dirL, PointLight& pointL) {
    shader->use();
    //shader->setUniform("viewProjMatrix", camera.getViewProjectionMatrix());
    shader->setUniform("camera_world", camera.getPos());

    shader->setUniform("dirL.color", dirL.color);
    shader->setUniform("dirL.direction", dirL.direction);
    shader->setUniform("pointL.color", pointL.color);
    shader->setUniform("pointL.position", pointL.position);
    shader->setUniform("pointL.attenuation", pointL.attenuation);
    shader->setUniform("draw_normals", _draw_normals);
    shader->setUniform("draw_texcoords", _draw_texcoords);
}


void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
    static double lastX = 0.0;
    static double lastY = 0.0;

    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    float xOffset = xPos - lastX;
    float yOffset = yPos - lastY;
    lastX = xPos;
    lastY = yPos;
    camera.rotate(yOffset, xOffset);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
    camera.zoom(yOffset);
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // F1 - Wireframe
    // F2 - Culling
    // Esc - Exit

    if (action != GLFW_RELEASE)
        return;

    switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true); break;
        case GLFW_KEY_F1:
            _wireframe = !_wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);
            break;
        case GLFW_KEY_F2:
            _culling = !_culling;
            if (_culling)
                glEnable(GL_CULL_FACE);
            else
                glDisable(GL_CULL_FACE);
            break;
        case GLFW_KEY_N:
            _draw_normals = !_draw_normals;
            break;
        case GLFW_KEY_T:
            _draw_texcoords = !_draw_texcoords;
            break;
    }
}

static void APIENTRY DebugCallbackDefault(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const GLvoid *userParam
) {
    if (id == 131185 || id == 131218)
        return; // ignore performance warnings from nvidia
    std::string error = FormatDebugOutput(source, type, id, severity, message);
    std::cout << error << std::endl;
}

static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg) {
    std::stringstream stringStream;
    std::string sourceString;
    std::string typeString;
    std::string severityString;

    // The AMD variant of this extension provides a less detailed classification of the error,
    // which is why some arguments might be "Unknown".
    switch (source) {
        case GL_DEBUG_CATEGORY_API_ERROR_AMD:
        case GL_DEBUG_SOURCE_API: {
            sourceString = "API";
            break;
        }
        case GL_DEBUG_CATEGORY_APPLICATION_AMD:
        case GL_DEBUG_SOURCE_APPLICATION: {
            sourceString = "Application";
            break;
        }
        case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
            sourceString = "Window System";
            break;
        }
        case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
        case GL_DEBUG_SOURCE_SHADER_COMPILER: {
            sourceString = "Shader Compiler";
            break;
        }
        case GL_DEBUG_SOURCE_THIRD_PARTY: {
            sourceString = "Third Party";
            break;
        }
        case GL_DEBUG_CATEGORY_OTHER_AMD:
        case GL_DEBUG_SOURCE_OTHER: {
            sourceString = "Other";
            break;
        }
        default: {
            sourceString = "Unknown";
            break;
        }
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR: {
            typeString = "Error";
            break;
        }
        case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
            typeString = "Deprecated Behavior";
            break;
        }
        case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
            typeString = "Undefined Behavior";
            break;
        }
        case GL_DEBUG_TYPE_PORTABILITY_ARB: {
            typeString = "Portability";
            break;
        }
        case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
        case GL_DEBUG_TYPE_PERFORMANCE: {
            typeString = "Performance";
            break;
        }
        case GL_DEBUG_CATEGORY_OTHER_AMD:
        case GL_DEBUG_TYPE_OTHER: {
            typeString = "Other";
            break;
        }
        default: {
            typeString = "Unknown";
            break;
        }
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: {
            severityString = "High";
            break;
        }
        case GL_DEBUG_SEVERITY_MEDIUM: {
            severityString = "Medium";
            break;
        }
        case GL_DEBUG_SEVERITY_LOW: {
            severityString = "Low";
            break;
        }
        default: {
            severityString = "Unknown";
            break;
        }
    }

    stringStream << "OpenGL Error: " << msg;
    stringStream << " [Source = " << sourceString;
    stringStream << ", Type = " << typeString;
    stringStream << ", Severity = " << severityString;
    stringStream << ", ID = " << id << "]";

    return stringStream.str();
}