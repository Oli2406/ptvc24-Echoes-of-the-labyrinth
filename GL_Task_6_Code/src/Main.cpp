/*
 * Copyright 2023 Vienna University of Technology.
 * Institute of Computer Graphics and Algorithms.
 * This file is part of the GCG Lab Framework and must not be redistributed.
 */
// Badie:Hallo
// TestTestTest
// es funktioniert beim anderen nicht :(

#include "Utils.h"
#include <sstream>
#include "Camera.h"
#include "Shader.h"
#include "Geometry.h"
#include "Material.h"
#include "Light.h"
#include "Texture.h"
#include "Model.h"


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
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void setPerFrameUniforms(Shader* shader, Camera& camera, DirectionalLight& dirL, PointLight& pointL);

/* --------------------------------------------- */
// Global variables
/* --------------------------------------------- */

static bool _wireframe = false;
static bool _culling = false;

static bool _draw_normals = false;
static bool _draw_texcoords = false;

static bool _dragging = false;
static bool _strafing = false;
static float _zoom = 5.0f;

/* --------------------------------------------- */
// Main
/* --------------------------------------------- */

int main(int argc, char** argv) {
    std::cout << ":::::: WELCOME TO GCG 2023 ::::::" << std::endl;

    CMDLineArgs cmdline_args;
    gcgParseArgs(cmdline_args, argc, argv);

    /* --------------------------------------------- */
    // Load settings.ini
    /* --------------------------------------------- */

    INIReader window_reader("assets/settings/window.ini");

    int window_width = window_reader.GetInteger("window", "width", 800);
    int window_height = window_reader.GetInteger("window", "height", 800);
    int refresh_rate = window_reader.GetInteger("window", "refresh_rate", 60);
    bool fullscreen = window_reader.GetBoolean("window", "fullscreen", false);
    std::string window_title = window_reader.Get("window", "title", "GCG 2023");
    std::string init_camera_filepath = "assets/settings/camera_front.ini";
    if (cmdline_args.init_camera) {
        init_camera_filepath = cmdline_args.init_camera_filepath;
    }
    INIReader camera_reader(init_camera_filepath);

    float fov = float(camera_reader.GetReal("camera", "fov", 60.0f));
    float nearZ = float(camera_reader.GetReal("camera", "near", 0.1f));
    float farZ = float(camera_reader.GetReal("camera", "far", 100.0f));
    float camera_yaw = static_cast<float>(camera_reader.GetReal("camera", "yaw", 0.0f));
    float camera_pitch = static_cast<float>(camera_reader.GetReal("camera", "pitch", 0.0f));

    std::string init_renderer_filepath = "assets/settings/renderer_standard.ini";
    if (cmdline_args.init_renderer) {
        init_renderer_filepath = cmdline_args.init_renderer_filepath;
    }
    INIReader renderer_reader(init_renderer_filepath);

    _wireframe = renderer_reader.GetBoolean("renderer", "wireframe", false);
    _culling = renderer_reader.GetBoolean("renderer", "backface_culling", false);
    _draw_normals = renderer_reader.GetBoolean("renderer", "normals", false);
    _draw_texcoords = renderer_reader.GetBoolean("renderer", "texcoords", false);
    bool _depthtest = renderer_reader.GetBoolean("renderer", "depthtest", true);

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
    glfwSetMouseButtonCallback(window, mouse_button_callback);
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

        // Create textures
        std::shared_ptr<Texture> woodTexture = std::make_shared<Texture>("assets/textures/wood_texture.dds");
        std::shared_ptr<Texture> tileTexture = std::make_shared<Texture>("assets/textures/tiles_diffuse.dds");

        // Create materials
        std::shared_ptr<Material> cornellMaterial = std::make_shared<Material>(cornellShader, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.9f, 0.3f), 10.0f);
        std::shared_ptr<Material> woodTextureMaterial = std::make_shared<TextureMaterial>(textureShader, glm::vec3(0.1f, 0.7f, 0.1f), 2.0f, woodTexture);
        std::shared_ptr<Material> tileTextureMaterial = std::make_shared<TextureMaterial>(textureShader, glm::vec3(0.1f, 0.7f, 0.3f), 8.0f, tileTexture);

        // Create Model

        Model map("assets/geometry/maze.obj");

        // Create geometry
        std::vector<glm::vec3> controlPoints = {
            glm::vec3(-0.3f, 0.6f, 0.0f),
            glm::vec3(0.0f, 1.6f, 0.0f),
            glm::vec3(1.4f, 0.3f, 0.0f),
            glm::vec3(0.0f, 0.3f, 0.0f),
            glm::vec3(0.0f, -0.5f, 0.0f),
        };
        int numSegments = 42;
        Geometry cornellBox = Geometry(glm::mat4(1), Geometry::createCornellBoxGeometry(3, 3, 3), cornellMaterial);
        Geometry cube = Geometry(
            glm::rotate(glm::translate(glm::mat4(1), glm::vec3(-0.5f, -0.8f, 0)), glm::radians(45.0f), glm::vec3(0, 1, 0)),
            Geometry::createCubeGeometry(0.34f, 0.34f, 0.34f),
            woodTextureMaterial
        );
        Geometry sphere = Geometry(
            glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.8f, 0.0f)), Geometry::createSphereGeometry(18, 8, 0.24f), tileTextureMaterial
        );
        Geometry cylinderBezier = Geometry(
            glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f)),
            Geometry::createBezierCylinderGeometry(18, controlPoints, numSegments, 0.2f),
            tileTextureMaterial
        );
        Geometry cylinder = Geometry(
            glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.3f, 0.0f)),
            Geometry::createCylinderGeometry(18, 1.5f, 0.2f),
            woodTextureMaterial
        );

        // Initialize camera
        Camera camera(fov, float(window_width) / float(window_height), nearZ, farZ);
        camera.setYaw(camera_yaw);
        camera.setPitch(camera_pitch);

        // Initialize lights
        DirectionalLight dirL(glm::vec3(0.8f), glm::vec3(0.0f, -1.0f, -1.0f));
        PointLight pointL(glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(1.0f, 0.4f, 0.1f));

        // Render loop
        float t = float(glfwGetTime());
        float dt = 0.0f;
        float t_sum = 0.0f;
        double mouse_x, mouse_y;

        while (!glfwWindowShouldClose(window)) {
            // Clear backbuffer
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            textureShader->use();

            glm::mat4 viewProjctionMat = camera.getViewProjectionMatrix();
            glUniformMatrix4fv(glGetUniformLocation(textureShader->getHandle(), "viewProjMatrix"), 1, GL_FALSE, glm::value_ptr(viewProjctionMat));

            // Poll events
            glfwPollEvents();

            // Update camera
            glfwGetCursorPos(window, &mouse_x, &mouse_y);
            camera.update(int(mouse_x), int(mouse_y), _zoom, _dragging, _strafing);

            // Set per-frame uniforms
            setPerFrameUniforms(cornellShader.get(), camera, dirL, pointL);
            setPerFrameUniforms(textureShader.get(), camera, dirL, pointL);

            // Render
            /*cornellBox.draw();
            cube.draw();
            cylinder.draw();
            sphere.draw();*/
            cylinderBezier.draw();

            // Draw the loaded model
            /*glm::mat4 model;
            model = glm::translate( model, glm::vec3( 0.0f, -1.75f, 0.0f ) ); // Translate it down a bit so it's at the center of the scene
            model = glm::scale( model, glm::vec3( 0.2f, 0.2f, 0.2f ) );	// It's a bit too big for our scene, so scale it down
            glUniformMatrix4fv( glGetUniformLocation(textureShader->getHandle(), "modelMatrix" ), 1, GL_FALSE, glm::value_ptr(model) );
            map.Draw(textureShader);*/

            // Compute frame time
            dt = t;
            t = float(glfwGetTime());
            dt = t - dt;
            t_sum += dt;

            // Swap buffers
            glfwSwapBuffers(window);

            if (cmdline_args.run_headless) {
                std::string screenshot_filename = "screenshot";
                if (cmdline_args.set_filename) {
                    screenshot_filename = cmdline_args.filename;
                }
                saveScreenshot(screenshot_filename, window_width, window_height);
                break;
            }
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


void setPerFrameUniforms(Shader* shader, Camera& camera, DirectionalLight& dirL, PointLight& pointL) {
    shader->use();
    shader->setUniform("viewProjMatrix", camera.getViewProjectionMatrix());
    shader->setUniform("camera_world", camera.getPosition());

    shader->setUniform("dirL.color", dirL.color);
    shader->setUniform("dirL.direction", dirL.direction);
    shader->setUniform("pointL.color", pointL.color);
    shader->setUniform("pointL.position", pointL.position);
    shader->setUniform("pointL.attenuation", pointL.attenuation);
    shader->setUniform("draw_normals", _draw_normals);
    shader->setUniform("draw_texcoords", _draw_texcoords);
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        _dragging = true;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        _dragging = false;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        _strafing = true;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        _strafing = false;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { _zoom -= float(yoffset) * 0.5f; }

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
