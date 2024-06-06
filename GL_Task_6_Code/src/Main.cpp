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
#include "Geometry.h"
#include "Animator.h"

#include <filesystem>

#include <ft2build.h>
#include FT_FREETYPE_H  
#include "globals.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


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
void initPhysics();
void gameplay(glm::vec3 playerPosition, glm::vec3 key1, glm::vec3 key2, glm::vec3 key3, glm::vec3 key4, glm::vec3 key5, glm::vec3 key6, glm::vec3 key7, glm::vec3 key8);
void RenderText(std::shared_ptr<Shader> shader, std::string text, float x, float y, float scale, glm::vec3 color);
void setPBRProperties(Shader* shader, float metallic, float roughness, float ao);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
ImGuiIO setupImGUI(GLFWwindow* window);
void setupHUD(ImGuiIO io, int keyCounter, int width, int height, int health);
void RenderHUD();
GLuint LoadTexture(const char* filename);

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

bool gammaEnabled = false;

ArcCamera camera;
bool firstMouse = true;
static float PI = 3.14159265358979;

bool key1Found = false;
bool key2Found = false;
bool key3Found = false;
bool key4Found = false;
bool key5Found = false;
bool key6Found = false;
bool key7Found = false;
bool key8Found = false;

int keyCounter = 0;
int health = 100;

bool won = false;
bool pbsDemo = false;
bool drawWalk = false;
bool drawIdle = true;
bool InfiniteJumpEnabled = false;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation* gFoundation = nullptr;
PxPhysics* gPhysics = nullptr;

PxDefaultCpuDispatcher* gDispatcher = nullptr;
PxScene* gScene = nullptr;

PxMaterial* gMaterial = nullptr;
PxPvd* gPvd = nullptr;



unsigned int planeVAO;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
unsigned int VAO, VBO;
float startTime = 0.0f;


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
    bool fullscreen = window_reader.GetBoolean("window", "fullscreen", true);
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
        std::shared_ptr<Shader> fontShader = std::make_shared<Shader>("assets/shaders/font.vert", "assets/shaders/font.frag");
        std::shared_ptr<Shader> pbsShader = std::make_shared<Shader>("assets/shaders/pbs.vert", "assets/shaders/pbs.frag");
        std::shared_ptr<Shader> skinningShader = std::make_shared<Shader>("assets/shaders/skinning.vert", "assets/shaders/skinning.frag");
        std::shared_ptr<Shader> puzzleShader = std::make_shared<Shader>("assets/shaders/puzzle.vert", "assets/shaders/puzzle.frag");

        // Create textures
        std::shared_ptr<Texture> fireTexture = std::make_shared<Texture>("assets/textures/fire.dds");
        std::shared_ptr<Texture> torchTexture = std::make_shared<Texture>("assets/textures/torch.dds");
        std::shared_ptr<Texture> keyTexture = std::make_shared<Texture>("assets/textures/gelb.dds");

        DDSImage img = loadDDS(gcgFindTextureFile("assets/textures/Militia-Texture.dds").c_str());
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
        std::shared_ptr<Material> keyColor = std::make_shared<TextureMaterial>(textureShader, glm::vec3(0.1f, 0.7f, 0.3f), 8.0f, keyTexture);

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

        string path4 = gcgFindTextureFile("assets/geometry/adventurer/walk.fbx");
        Model adventurer(&path4[0], gPhysics, gScene, true);
        Animation idle(path4, &adventurer);
        Animator idleAnimator(&idle);

        string walkPath = gcgFindTextureFile("assets/geometry/adventurer/idle.fbx");
        Model walkModel(&walkPath[0]);
        Animation walk(walkPath, &walkModel);
        Animator walkAnimator(&walk);

        string path5 = gcgFindTextureFile("assets/geometry/key/key.obj");
        Model key(&path5[0], gPhysics, gScene, false);

        string path6 = gcgFindTextureFile("assets/geometry/bridge/bridge.obj");
        Model bridge(&path6[0], gPhysics, gScene, false);

        string path7 = gcgFindTextureFile("assets/geometry/lava/lava.obj");
        Model lava(&path7[0]);

        Player player1 = Player(adventurer, 0.0f, 0.0f, 0.0f, 1.0f, adventurer.getController());

        player1.set(adventurer);

        // Initialize camera
        camera.setCamParameters(fov, float(window_width) / float(window_height), nearZ, farZ, camera_yaw, camera_pitch);

        // Initialize lights
        DirectionalLight dirL(glm::vec3(2.0f), glm::vec3(-2.0f, -4.0f, -1.0f));
        PointLight pointL(glm::vec3(4.0f), glm::vec3(0, 5, 0), glm::vec3(1.0f, 0.7f, 1.8f));
        PointLight pointL2(glm::vec3(4.0f), glm::vec3(2, 1.5, 0), glm::vec3(1.0f, 0.4f, 0.1f));

        // Render loop
        float t = float(glfwGetTime());
        float t_sum = 0.0f;
        float dt = 0.0f;

        glm::mat4 modelDiamiond = glm::mat4(1.0f);
        modelDiamiond = glm::translate(modelDiamiond, glm::vec3(0.0f, 0.0f, 0.0f));


        mat4 viewMatrix = camera.calculateMatrix(camera.getRadius(), camera.getPitch(), camera.getYaw(), player1);
        glm::vec3 camDir = camera.getPos();

        glm::mat4 play = glm::mat4(1.0f);
        play = glm::translate(play, player1.getPosition());

        glm::vec3 prevCamDir = glm::vec3(0.0f, 0.0f, 0.0f);
        double prevRotation = 0.0f;
        double angle = 0.0f;

        glm::vec3 materialCoefficients = glm::vec3(0.1f, 0.7f, 0.1f);
        float alpha = 1.0f;
        float prevAngle = 0.0f;

        glm::vec3 key1 = glm::vec3(30, 1, 27);
        glm::vec3 key2 = glm::vec3(-30, 1, -29);
        glm::vec3 key3 = glm::vec3(-30, 1, 29);
        glm::vec3 key4 = glm::vec3(30, 1, -29);
        glm::vec3 key5 = glm::vec3(-8, 1, 34.75);
        glm::vec3 key6 = glm::vec3(-5, 1, -40.25);
        glm::vec3 key7 = glm::vec3(52.5, 1, -2);
        glm::vec3 key8 = glm::vec3(-41.25, 1, -3);

        glm::mat4 demokey1 = glm::mat4(1.0f);
        glm::mat4 demokey2 = glm::mat4(1.0f);
        

        // configure depth map FBO
        // -----------------------
        const unsigned int SHADOW_WIDTH = 16384, SHADOW_HEIGHT = 8192;
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
        float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // text rendering


        // FreeType
        // --------
        FT_Library ft;
        
        if (FT_Init_FreeType(&ft))
        {
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return -1;
        }

        // find path to font
        std::string font_name = gcgFindTextureFile("assets/fonts/Rockers_Garage.ttf");
        if (font_name.empty())
        {
            std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
            return -1;
        }

        // load font as face
        FT_Face face;
        if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
            return -1;
        }
        else {
            // set size to load glyphs as
            FT_Set_Pixel_Sizes(face, 0, 48);

            // disable byte-alignment restriction
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            // load first 128 characters of ASCII set
            for (unsigned char c = 0; c < 128; c++)
            {
                // Load character glyph 
                if (FT_Load_Char(face, c, FT_LOAD_RENDER))
                {
                    std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                    continue;
                }
                // generate texture
                unsigned int texture;
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RED,
                    face->glyph->bitmap.width,
                    face->glyph->bitmap.rows,
                    0,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    face->glyph->bitmap.buffer
                );
                // set texture options
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                // now store character for later use
                Character character = {
                    texture,
                    glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                    static_cast<unsigned int>(face->glyph->advance.x)
                };
                Characters.insert(std::pair<char, Character>(c, character));
            }
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        // destroy FreeType once we're finished
        FT_Done_Face(face);
        FT_Done_FreeType(ft);


        // configure VAO/VBO for texture quads
        // -----------------------------------
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);


        // shader configuration
        // --------------------   

        textureShader->use();
        textureShader->setUniform("shadowMap", 2);
        modelShader->use();
        modelShader->setUniform("texture_diffuse", 0);
        if (!won) {
            modelShader->setUniform("skybox", 1);
        }
        modelShader->setUniform("shadowMap", 2);
        debugDepthQuad->use();
        debugDepthQuad->setUniform("depthMap", 0);
        pbsShader -> setUniform("texture_diffuse", 0);
        pbsShader->use();
        pbsShader->setUniform("skybox", 1);
        pbsShader->setUniform("shadowMap", 2);
        skinningShader->use();
        skinningShader->setUniform("texture_diffuse", 0);
        skinningShader->setUniform("skybox", 1);
        skinningShader->setUniform("shadowMap", 2);
        
        
        glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

        lightPos *= 10;
        
        ImGuiIO io = setupImGUI(window);

        while (!glfwWindowShouldClose(window)) {

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glm::mat4 lightProjection, lightView;
            glm::mat4 lightSpaceMatrix;
            float near_plane = 0.1f, far_plane = 100.0f;
            lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
            lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightSpaceMatrix = lightProjection * lightView;
            depthShader->use();
            depthShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);
            

            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);

            fireShad.draw();
            torchShad.draw();

            player1.Draw(depthShader, camDir, false);
            depthShader->setUniform("modelMatrix", glm::mat4(1.0f));
            floor.Draw(depthShader);
            map.Draw(depthShader);
            depthShader->setUniform("modelMatrix", glm::mat4(1.0f));
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
            setupHUD(io, keyCounter, window_width, window_height, health);

            viewMatrix = camera.calculateMatrix(camera.getRadius(), camera.getPitch(), camera.getYaw(), player1);
            camDir = camera.extractCameraDirection(viewMatrix);
            viewProjectionMatrix = projection * viewMatrix;
            if (!won) {
                modelShader->setUniform("viewProjMatrix", viewProjectionMatrix);
                modelShader->setUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(play))));
                modelShader->setUniform("materialCoefficients", materialCoefficients);
                modelShader->setUniform("specularAlpha", alpha);
                setPerFrameUniforms(modelShader.get(), camera, dirL, pointL);
                modelShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);
                modelShader->setUniform("gamma", gammaEnabled);
            }
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, depthMap);

            if (!won) {
                player1.checkInputs(window, dt, camDir, InfiniteJumpEnabled);
            }

            if (drawWalk && !drawIdle) {
                skinningShader->use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture3);
                skinningShader->setUniform("viewProjMatrix", viewProjectionMatrix);
                skinningShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);
                skinningShader->setUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(play))));
                setPerFrameUniforms(skinningShader.get(), camera, dirL, pointL);
                skinningShader->setUniform("materialCoefficients", materialCoefficients);
                skinningShader->setUniform("specularAlpha", alpha);
                auto transforms = idleAnimator.GetFinalBoneMatrices();
                skinningShader->setUniform("gamma", gammaEnabled);
                idleAnimator.UpdateAnimation(dt);
                for (int i = 0; i < transforms.size(); ++i)
                {
                    skinningShader->setUniform("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
                }
                player1.Draw(skinningShader, camDir, won);
            }
            if(drawIdle && !drawWalk) {
                skinningShader->use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture3);
                skinningShader->setUniform("viewProjMatrix", viewProjectionMatrix);
                skinningShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);
                skinningShader->setUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(play))));
                setPerFrameUniforms(skinningShader.get(), camera, dirL, pointL);
                skinningShader->setUniform("materialCoefficients", materialCoefficients);
                skinningShader->setUniform("specularAlpha", alpha);
                auto transforms = walkAnimator.GetFinalBoneMatrices();
                skinningShader->setUniform("gamma", gammaEnabled);
                walkAnimator.UpdateAnimation(dt);
                for (int i = 0; i < transforms.size(); ++i)
                {
                    skinningShader->setUniform("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
                }
                player1.Draw(skinningShader, camDir, won);
            }
            

            modelShader->use();
            modelShader->setUniform("modelMatrix", glm::mat4(1.0f));
            floor.Draw(modelShader);
            map.Draw(modelShader);
            modelShader->setUniform("modelMatrix", glm::mat4(1.0f));
            lava.Draw(modelShader);

            pbsShader->use();
            pbsShader->setUniform("modelMatrix", glm::mat4(1.0f));
            pbsShader->setUniform("viewProjMatrix", viewProjectionMatrix);
            pbsShader->setUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(play))));
            pbsShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);
            pbsShader->setUniform("interpolationFactor", 0.005f);
            setPerFrameUniforms(pbsShader.get(), camera, dirL, pointL);
            setPBRProperties(pbsShader.get(), 0.0f, 0.9f, 0.7f);
            podest.Draw(pbsShader);
            modelDiamiond = glm::rotate(modelDiamiond, glm::radians(0.1f), glm::vec3(0.0f, 1.0f, 0.0f));
            pbsShader->setUniform("modelMatrix", modelDiamiond);
            pbsShader->setUniform("interpolationFactor", 0.05f);
            setPBRProperties(pbsShader.get(), 1.0f, 0.4f, 1.0f);
            diamond.Draw(pbsShader);
            if (pbsDemo) {
                pbsShader->setUniform("modelMatrix", glm::translate(demokey1, vec3(player1.getPosition().x - 1, player1.getPosition().y, player1.getPosition().z)));
                key.Draw(pbsShader);
                pbsShader->setUniform("interpolationFactor", 0.001f);
                pbsShader->setUniform("modelMatrix", glm::translate(demokey2, vec3(player1.getPosition().x - 1, player1.getPosition().y, player1.getPosition().z + 2)));
                setPBRProperties(pbsShader.get(), 0.0f, 0.9f, 1.0f);
                key.Draw(pbsShader);
            }

           
            modelShader->use();

            if (keyCounter < 4) {

                glm::mat4 keyModel = glm::translate(mat4(1.0f), key1);
                if (!key1Found) {
                    modelShader->setUniform("modelMatrix", keyModel);
                    key.Draw(modelShader);
                }
                keyModel = glm::translate(mat4(1.0f), key2);
                if (!key2Found) {
                    modelShader->setUniform("modelMatrix", keyModel);
                    key.Draw(modelShader);
                }
                keyModel = glm::translate(mat4(1.0f), key3);
                if (!key3Found) {
                    modelShader->setUniform("modelMatrix", keyModel);
                    key.Draw(modelShader);
                }
                keyModel = glm::translate(mat4(1.0f), key4);
                if (!key4Found) {
                    modelShader->setUniform("modelMatrix", keyModel);
                    key.Draw(modelShader);
                }
                keyModel = glm::translate(mat4(1.0f), key5);
                if (!key5Found) {
                    modelShader->setUniform("modelMatrix", keyModel);
                    key.Draw(modelShader);
                }
                keyModel = glm::translate(mat4(1.0f), key6);
                if (!key6Found) {
                    modelShader->setUniform("modelMatrix", keyModel);
                    key.Draw(modelShader);
                }
                keyModel = glm::translate(mat4(1.0f), key7);
                if (!key7Found) {
                    modelShader->setUniform("modelMatrix", keyModel);
                    key.Draw(modelShader);
                }
                keyModel = glm::translate(mat4(1.0f), key8);
                if (!key8Found) {
                    modelShader->setUniform("modelMatrix", keyModel);
                    key.Draw(modelShader);
                }
            }

            gameplay(player1.getPosition(), key1, key2, key3, key4, key5, key6, key7, key8);

            if (keyCounter >= 4 ) {
                modelShader->setUniform("modelMatrix", glm::mat4(1.0f));
                bridge.Draw(modelShader);
            }
   
            if (!won) {
                setPerFrameUniforms(textureShader.get(), camera, dirL, pointL);
                textureShader->setUniform("viewProjMatrix", viewProjectionMatrix);
                textureShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);
                textureShader->setUniform("gamma", gammaEnabled);
            }

            fire.draw();
            torch.draw();

            glm::vec3 firePosition = player1.getPosition() + glm::vec3(0.4f, 1.5f, 0.0f);
            glm::vec3 torchPosition = player1.getPosition() + glm::vec3(0.4f, 1.41f, 0.0f);
            fire.updateModelMatrix(glm::scale(glm::translate(glm::mat4(1.0f), firePosition), glm::vec3(0.1f, 0.1f, 0.1f)));
            torch.updateModelMatrix(glm::scale(glm::translate(glm::mat4(1.0f), torchPosition), glm::vec3(0.1f, 0.4f, 0.1f)));

            fireShad.updateModelMatrix(glm::scale(glm::translate(play, firePosition), glm::vec3(0.1f, 0.1f, 0.1f)));
            torchShad.updateModelMatrix(glm::scale(glm::translate(play, torchPosition), glm::vec3(0.1f, 0.4f, 0.1f)));

            pointL.position = player1.getPosition() + glm::vec3(0.4f, 1.5f, 0.0f);

            gScene->simulate(dt);
            gScene->fetchResults(true);

            sky->use();
            sky->setUniform("viewProjMatrix", viewProjectionMatrix);
            skybox.draw();


            if (won) {
                glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(window_width), 0.0f, static_cast<float>(window_height));
                fontShader->use();
                fontShader->setUniform("projection", projection);

                //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
                //glClear(GL_COLOR_BUFFER_BIT);

                RenderText(fontShader, "You Won!", 480.0f, 500.0f, 5.0f, glm::vec3(0.5, 0.8f, 0.2f));

                if (startTime == 0.0f) {
                    
                    startTime = glfwGetTime();
                }

                float currentTime = glfwGetTime();
                if (currentTime - startTime >= 15.0f) {
                    
                    std::cout << "5 seconds have passed!" << std::endl;
                    break;
                }
               
            }
            //std::cout << (gammaEnabled ? "Gamma enabled" : "Gamma disabled") << std::endl;
            //std::cout << (pbsDemo ? "demo enabled" : "demo disabled") << std::endl;

            // Compute frame time
            dt = t;
            t = float(glfwGetTime());
            dt = (t - dt);
            t_sum += dt;

            RenderHUD();
  
            // Swap buffers
            glfwSwapBuffers(window);
        }
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    destroyFramework();
    glfwTerminate();
    return EXIT_SUCCESS;
}


void RenderText(std::shared_ptr<Shader> shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    shader->use();
    glUniform3f(glGetUniformLocation(shader->getHandle(), "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
       
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 

        glBindBuffer(GL_ARRAY_BUFFER, 0);
      
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        x += (ch.Advance >> 6) * scale; 
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void gameplay(glm::vec3 playerPosition, glm::vec3 key1, glm::vec3 key2, glm::vec3 key3, glm::vec3 key4, glm::vec3 key5, glm::vec3 key6, glm::vec3 key7, glm::vec3 key8) {
    float x = playerPosition.x;
    float y = playerPosition.y;
    float z = playerPosition.z;
    if (x > key1.x - 0.55 && x < key1.x + 0.55 && z > key1.z - 1.7 && z < key1.z - 0.2) {
        std::cout << playerPosition.x << "," << playerPosition.z << std::endl;
        if (!key1Found) {
            keyCounter++;
        }
        key1Found = true;
    }

    if (x > key2.x - 0.5 && x < key2.x + 0.5 && z > key2.z - 1.7 && z < key2.z - 0.2) {
        if (!key2Found) {
            keyCounter++;
        }
        key2Found = true;
    }

    if (x > key3.x - 0.5 && x < key3.x + 0.5 && z > key3.z - 1.7 && z < key3.z - 0.2) {
        std::cout << playerPosition.x << "," << playerPosition.z << std::endl;
        if (!key3Found) {
            keyCounter++;
        }
        key3Found = true;
    }

    if (x > key4.x - 0.5 && x < key4.x + 0.5 && z > key4.z - 1.7 && z < key4.z - 0.2) {
        std::cout << playerPosition.x << "," << playerPosition.z << std::endl;
        if (!key4Found) {
            keyCounter++;
        }
        key4Found = true;
    }

    if (x > key5.x - 0.5 && x < key5.x + 0.5 && z > key5.z - 1.7 && z < key5.z - 0.2) {
        if (!key5Found) {
            keyCounter++;
        }
        key5Found = true;
    }

    if (x > key6.x - 0.5 && x < key6.x + 0.5 && z > key6.z - 1.7 && z < key6.z - 0.2) {
        if (!key6Found) {
            keyCounter++;
        }
        key6Found = true;
    }

    if (x > key7.x - 0.5 && x < key7.x + 0.5 && z > key7.z - 1.7 && z < key7.z - 0.2) {
        if (!key7Found) {
            keyCounter++;
        }
        key7Found = true;
    }

    if (x > key8.x - 0.5 && x < key8.x + 0.5 && z > key8.z - 1.7 && z < key8.z - 0.2) {
        if (!key8Found) {
            keyCounter++;
        }
        key8Found = true;
    }

    if (x < 0 + 1.2 && x > 0 - 1.2 && z < 0 + 1.2 && z > 0 - 1.2 && y > -2.0 && y < 2.0) {
        won = true;
    }
}

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
    if (action != GLFW_PRESS && action != GLFW_RELEASE)
        return;

    switch (key) {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        break;
    case GLFW_KEY_F1:
        if (action == GLFW_PRESS) {
            _wireframe = !_wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);
        }
        break;
    case GLFW_KEY_F2:
        if (action == GLFW_PRESS) {
            _culling = !_culling;
            if (_culling)
                glEnable(GL_CULL_FACE);
            else
                glDisable(GL_CULL_FACE);
        }
        break;
    case GLFW_KEY_F6:
        if (action == GLFW_PRESS) {
            InfiniteJumpEnabled = !InfiniteJumpEnabled;
        }
        break;
    case GLFW_KEY_N:
        if (action == GLFW_PRESS) {
            _draw_normals = !_draw_normals;
        }
        break;
    case GLFW_KEY_T:
        if (action == GLFW_PRESS) {
            _draw_texcoords = !_draw_texcoords;
        }
        break;
    case GLFW_KEY_G:
        if (action == GLFW_PRESS) {
            gammaEnabled = !gammaEnabled;
        }
        break;
    case GLFW_KEY_P:
        if (action == GLFW_PRESS) {
            pbsDemo = !pbsDemo;
        }
        break;
    case GLFW_KEY_J:
        if (health-- < 0) {
            health = 100;
        }
        health--;
        break;
    case GLFW_KEY_K:
        health++;
        if (health++ > 100) {
            health = 0;
        }
        break;
    case GLFW_KEY_W:
    case GLFW_KEY_A:
    case GLFW_KEY_S:
    case GLFW_KEY_D:
        if (action == GLFW_PRESS) {
            drawWalk = true;
            drawIdle = false;
        }
        else if (action == GLFW_RELEASE) {
            bool anyKeyPressed = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
            drawWalk = anyKeyPressed;
            drawIdle = !anyKeyPressed;
        }
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
    //gScene->addActor(*groundPlane);
}

void setPBRProperties(Shader* shader, float metallic, float roughness, float ao) {
    shader->setUniform("ao", ao);
    shader->setUniform("roughness", roughness);
    shader->setUniform("metallic", metallic);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

ImGuiIO setupImGUI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    return io;
}

void setupHUD(ImGuiIO io, int keyCount, int width, int height, int health) {
    // Initialize the start time
    static double start_time = glfwGetTime();
    string path = gcgFindTextureFile("assets/uiPictures/portrait.png");
    GLuint splashArt = LoadTexture(path.c_str());
    string keyPath = gcgFindTextureFile("assets/uiPictures/key.png");
    GLuint keyArt = LoadTexture(keyPath.c_str());

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    ImGui::Begin("Portrait", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    {
        ImGui::Image((void*)(intptr_t)splashArt, ImVec2(210, 150));
    }
    ImGui::End();
    ImGui::SetNextWindowPos(ImVec2(width / 9.15, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(width / 4.4, height / 13.5), ImGuiCond_Once);
    ImGui::Begin("Health", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    {
        int myHealth = health;
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(width / 4.8, height / 27);
        float fraction = myHealth / 100.0f;

        ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), IM_COL32(60, 60, 60, 255));

        ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + size.x * fraction, p.y + size.y), IM_COL32(255, 0, 0, 255));

        ImGui::SetWindowFontScale(2.5f);
        ImGui::GetWindowDrawList()->AddText(ImVec2(p.x + size.x / 2 - ImGui::CalcTextSize(std::to_string(myHealth).c_str()).x / 2, p.y), IM_COL32(255, 255, 255, 255), std::to_string(myHealth).c_str());
        ImGui::SetWindowFontScale(1.0f);

        ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + size.y + 10));
    }
    ImGui::End();
    ImGui::SetNextWindowPos(ImVec2(width / 1.25, height / 1.06), ImGuiCond_Once);
    ImGui::Begin("Time", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    {
        double current_time = glfwGetTime();
        double elapsed_time = current_time - start_time;
        ImGui::SetWindowFontScale(2.0f);
        ImGui::Text("Time Elapsed: %.2f seconds", elapsed_time);
        ImGui::SetWindowFontScale(1.0f);
    }
    ImGui::End();
    ImGui::SetNextWindowPos(ImVec2(1720, 0));
    ImGui::Begin("Keys", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImVec2 size = ImVec2(width / 19.2, height / 108);
        ImGui::SetWindowFontScale(2.0f);
        ImGui::GetWindowDrawList()->AddText(ImVec2(p.x + size.x - ImGui::CalcTextSize(std::to_string(keyCount).c_str()).x, p.y), IM_COL32(255, 255, 255, 255), std::to_string(keyCount).c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Image((void*)(intptr_t)keyArt, ImVec2(190, 200));
    }
    ImGui::End();
}

void RenderHUD() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

GLuint LoadTexture(const char* filename) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, nrChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return textureID;
}
