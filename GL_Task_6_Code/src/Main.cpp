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
//#include "physics/PhysXInitializer.h"
#include "Geometry.h"

#include <filesystem>


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
    const GLchar* message,
    const GLvoid* userParam
);
static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void setPerFrameUniforms(Shader* shader, ArcCamera& camera, DirectionalLight& dirL, PointLight& pointL);
void renderQuad();
void renderScene(const std::shared_ptr<Shader> shader);
void renderCube();
unsigned int loadTexture(const char* path);
void initPhysics();

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

ArcCamera camera;
bool firstMouse = true;
static float PI = 3.14159265358979;




PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation* gFoundation = nullptr;
PxPhysics* gPhysics = nullptr;

PxDefaultCpuDispatcher* gDispatcher = nullptr;
PxScene* gScene = nullptr;

PxMaterial* gMaterial = nullptr;
PxPvd* gPvd = nullptr;





//PxPhysics* physics = PhysXInitializer::initializePhysX();
//PxScene* scene = PhysXInitializer::createPhysXScene(physics);
// meshes
unsigned int planeVAO;


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

    initPhysics();

    /* --------------------------------------------- */
    // Initialize scene and render loop
    /* --------------------------------------------- */
    {
        // Load shader(s)
        std::shared_ptr<Shader> depthShader = std::make_shared<Shader>("assets/shaders/depthShader.vert", "assets/shaders/depthShader.frag");
        std::shared_ptr<Shader> textureShader = std::make_shared<Shader>("assets/shaders/texture.vert", "assets/shaders/texture.frag");
        std::shared_ptr<Shader> modelShader = std::make_shared<Shader>("assets/shaders/model.vert", "assets/shaders/model.frag");
        std::shared_ptr<Shader> sky = std::make_shared<Shader>("assets/shaders/sky.vert", "assets/shaders/sky.frag");
        std::shared_ptr<Shader> debugDepthQuad = std::make_shared<Shader>("assets/shaders/debugDepthQuad.vert", "assets/shaders/debugDepthQuad.frag");

        // Create textures
        std::shared_ptr<Texture> fireTexture = std::make_shared<Texture>("assets/textures/fire.dds");
        std::shared_ptr<Texture> torchTexture = std::make_shared<Texture>("assets/textures/torch.dds");
        DDSImage img = loadDDS(gcgFindTextureFile("assets/textures/wood_texture.dds").c_str());
        GLuint texture3;
        glGenTextures(1, &texture3);
        glBindTexture(GL_TEXTURE_2D, texture3);

        // Transfer image data to GPU
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, img.width, img.height, 0, img.size, img.data);

        // Generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Create materials
        std::shared_ptr<Material> fireTextureMaterial = std::make_shared<TextureMaterial>(textureShader, glm::vec3(0.1f, 0.7f, 0.1f), 2.0f, fireTexture);
        std::shared_ptr<Material> torchTextureMaterial = std::make_shared<TextureMaterial>(textureShader, glm::vec3(0.1f, 0.7f, 0.3f), 8.0f, torchTexture);
        std::shared_ptr<Material> fireShadow = std::make_shared<TextureMaterial>(depthShader, glm::vec3(0.1f, 0.7f, 0.1f), 2.0f, fireTexture);
        std::shared_ptr<Material> torchShadow = std::make_shared<TextureMaterial>(depthShader, glm::vec3(0.1f, 0.7f, 0.3f), 8.0f, torchTexture);

        // Create geometry
        Geometry fire = Geometry(
            glm::translate(glm::mat4(1), glm::vec3(0, 2.5, 0)),
            Geometry::createCubeGeometry(0.34f, 0.34f, 0.34f),
            fireTextureMaterial
        );

        Geometry torch = Geometry(
            glm::scale(glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)), glm::vec3(1.0f, 3.0f, 1.0f)),
            Geometry::createCubeGeometry(0.34f, 0.34f, 0.34f),
            torchTextureMaterial
        );

        Geometry fireShad = Geometry(
            glm::translate(glm::mat4(1), glm::vec3(0, 2.5, 0)),
            Geometry::createCubeGeometry(0.34f, 0.34f, 0.34f),
            fireShadow
        );

        Geometry torchShad = Geometry(
            glm::scale(glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)), glm::vec3(1.0f, 3.0f, 1.0f)),
            Geometry::createCubeGeometry(0.34f, 0.34f, 0.34f),
            torchShadow
        );

        string path = gcgFindTextureFile("assets/geometry/maze/maze.obj");
        Model map(&path[0], gPhysics, gScene, false);
        Skybox skybox;

        string path1 = gcgFindTextureFile("assets/geometry/podest/podest.obj");
        Model podest(&path1[0], gPhysics, gScene, false);

        string path2 = gcgFindTextureFile("assets/geometry/floor/floor.obj");
        Model floor(&path2[0], gPhysics, gScene, false);

        string path3 = gcgFindTextureFile("assets/geometry/diamond/diamond.obj");
        Model diamond(&path3[0], gPhysics, gScene, false);

        string path4 = gcgFindTextureFile("assets/geometry/adventurer/adventurer.obj");
        Model adventurer(&path4[0], gPhysics, gScene, true);

        //diamond.printNormals();


        //Physics simulation;
        Player player1 = Player(adventurer, 0.0f, 0.0f, 0.0f, 1.0f, adventurer.getPlayerModel());

        player1.set(adventurer, 0, 0, 0, 1);

        // Initialize camera
        camera.setCamParameters(fov, float(window_width) / float(window_height), nearZ, farZ, camera_yaw, camera_pitch);

        // Initialize lights
        DirectionalLight dirL(glm::vec3(2.0f), glm::vec3(-2.0f, -4.0f, -1.0f));
        PointLight pointL(glm::vec3(4.0f), glm::vec3(0, 2.5, 0), glm::vec3(1.0f, 0.4f, 0.1f));

        // Render loop
        float t = float(glfwGetTime());
        float t_sum = 0.0f;
        float dt = 0.0f;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(6.0f, 6.0f, 6.0f));

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
        float alpha = 1.0f;
        float prevAngle = 0.0f;

        // configure depth map FBO
        // -----------------------
        const unsigned int SHADOW_WIDTH = window_width, SHADOW_HEIGHT = window_height;
        unsigned int depthMapFBO;
        glGenFramebuffers(1, &depthMapFBO);
        // create depth texture
        unsigned int depthMap;
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = {1.0, 1.0, 1.0, 1.0};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // shader configuration
        // --------------------
        textureShader->use();
        textureShader->setUniform("shadowMap", 2);
        modelShader->use();
        modelShader->setUniform("texture_diffuse", 0);
        modelShader->setUniform("skybox", 1);
        modelShader->setUniform("shadowMap", 2);
        debugDepthQuad->use();
        debugDepthQuad->setUniform("depthMap", 0);

        // lighting info
        // -------------
        glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

        lightPos *= 10;

        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 lightProjection, lightView;
            glm::mat4 lightSpaceMatrix;
            float near_plane = 0.1f, far_plane = 75.0f;
            lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
            lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightSpaceMatrix = lightProjection * lightView;
            // render scene from light's point of view
            depthShader->use();
            depthShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);

            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);

            fireShad.draw();
            torchShad.draw();
            
            player1.Draw(depthShader);
            depthShader->setUniform("modelMatrix", model);
            floor.Draw(depthShader);
            map.Draw(depthShader);
            depthShader->setUniform("modelMatrix", podestModel);
            podest.Draw(depthShader);
            modelDiamiond = glm::rotate(modelDiamiond, glm::radians(0.1f), glm::vec3(0.0f, 1.0f, 0.0f));
            depthShader->setUniform("modelMatrix", modelDiamiond);
            diamond.Draw(depthShader);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // reset viewport
            glViewport(0, 0, window_width, window_height);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            modelShader->use();

            glfwPollEvents();

            viewMatrix = camera.calculateMatrix(camera.getRadius(), camera.getPitch(), camera.getYaw(), player1);
            camDir = camera.extractCameraDirection(viewMatrix);
            viewProjectionMatrix = projection * viewMatrix;

            modelShader->setUniform("viewProjMatrix", viewProjectionMatrix);
            modelShader->setUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(play))));
            modelShader->setUniform("materialCoefficients", materialCoefficients);
            modelShader->setUniform("specularAlpha", alpha);
            // Set per-frame uniforms
            setPerFrameUniforms(modelShader.get(), camera, dirL, pointL);
            modelShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, depthMap);

            player1.checkInputs(window, dt, camDir);
            //player1.updateRotation(camDir);
            if (camDir != prevCamDir) {
                angle = player1.getRotY() * 0.005f;
                //play = glm::rotate(play, float(angle), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            play = glm::translate(play, player1.getPosition());

            prevCamDir = camDir;

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture3);

            player1.Draw(modelShader);

            modelShader->setUniform("modelMatrix", model);

            floor.Draw(modelShader);
            map.Draw(modelShader);

            modelShader->setUniform("modelMatrix", podestModel);

            podest.Draw(modelShader);

            modelDiamiond = glm::rotate(modelDiamiond, glm::radians(0.1f), glm::vec3(0.0f, 1.0f, 0.0f));

            modelShader->setUniform("modelMatrix", modelDiamiond);
            diamond.Draw(modelShader);
            
            setPerFrameUniforms(textureShader.get(), camera, dirL, pointL);
            textureShader->setUniform("viewProjMatrix", viewProjectionMatrix);
            textureShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);

            fire.draw();
            torch.draw();

            glm::vec3 firePosition = player1.getPosition() + glm::vec3(0.4f, 1.5f, 0.0f);
            glm::vec3 torchPosition = player1.getPosition() + glm::vec3(0.4f, 1.41f, 0.0f);
            fire.updateModelMatrix(glm::scale(glm::translate(play, firePosition), glm::vec3(0.1f, 0.1f, 0.1f)));
            torch.updateModelMatrix(glm::scale(glm::translate(play, torchPosition), glm::vec3(0.1f, 0.4f, 0.1f)));

            fireShad.updateModelMatrix(glm::scale(glm::translate(play, firePosition), glm::vec3(0.1f, 0.1f, 0.1f)));
            torchShad.updateModelMatrix(glm::scale(glm::translate(play, torchPosition), glm::vec3(0.1f, 0.4f, 0.1f)));

            pointL.position = player1.getPos() + glm::vec3(0.4f, 1.5f, 0.0f);
            
            gScene->simulate(dt);
            gScene->fetchResults(true);
            
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


// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
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
    camera.zoom(yOffset / 2);
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
    const GLchar* message,
    const GLvoid* userParam
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

void initPhysics()
{
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    if (!gFoundation) {
        std::cerr << "Failed to create PhysX foundation." << std::endl;
        return;
    }

    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
    if (!gPhysics) {
        std::cerr << "Failed to create PhysX physics." << std::endl;
        gPvd->disconnect();
        transport->release();
        gPvd->release();
        gFoundation->release();
        return;
    }

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    if (!gDispatcher) {
        std::cerr << "Failed to create dispatcher." << std::endl;
        return;
    }
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    gScene = gPhysics->createScene(sceneDesc);

    if (!gScene) {
        std::cerr << "Failed to create the scene." << std::endl;
        return;
    }

    PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
    if (pvdClient) {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }
    else {
        std::cout << "PVD client is null, check PVD connection stability." << std::endl;
    }

    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    if (!gMaterial) {
        std::cerr << "Failed to create material." << std::endl;
        return;
    }

    PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
    if (!groundPlane) {
        std::cerr << "Failed to create ground plane." << std::endl;
        return;
    }
    gScene->addActor(*groundPlane);
}
