#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
//#include "rg/Camera.h"
#include <learnopengl/model.h>

#include <iostream>


void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadTexture(char const *path);

void hasLights(Shader& shader, bool directional, bool pointLight, bool spotlight);

unsigned int loadCubemap(vector<std::string> &faces);


// resolution
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    double constant;
    double linear;
    double quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    SpotLight(){
        position = glm::vec3(0.0f, 4.0f, 0.0f);
    }
};

struct PointLight {
    glm::vec3 position;

    double constant;
    double linear;
    double quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    PointLight(){
        position = glm::vec3(-9.0f, 13.0f, 5.0f);
    }
};

struct Prozor{
    glm::vec3 position;
    float rotateX;
    float rotateY;
    float rotateZ;

    float windowScaleFactor;
};

struct DirLight {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

void initializeTransparentWindows(vector<Prozor> &prozori);

// moze da se doda sta god, mora samo posle ako hoce da se sacuva da se navede i u SaveToFile i u LoadFromFile
struct ProgramState {
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    // --Ovde ide ono sto mi dodajemo, ostalo se ne dira--

    glm::vec3 clearColor = glm::vec3(0.0f);
    SpotLight spotLight;
    PointLight pointLight;
    DirLight dirLight;
    glm::vec3 tyresPosition = glm::vec3(6.0f, 0.39f, 0.0f);
    glm::vec3 carPosition = glm::vec3(0.0f, 1.205f, 0.45f);
    glm::vec3 platformPosition = glm::vec3(0.0f, 0.4321f, 0.0f);
    glm::vec3 trophyPosition = glm::vec3(-5.170f,1.2f,6.6f);
    glm::vec3 tablePosition = glm::vec3(-4.0f,0.0f,3.8f);

    std::vector<Prozor> prozori;

    glm::vec3 pipePosition=glm::vec3 (-13.0f, 2.0f, -13.0f);
    glm::vec3 propelerPosition=glm::vec3(-0.865f, 0.27f, -0.865f);
    glm::vec3 krugPosition=glm::vec3(4.38, 4.38f, 4.38f);



    float tableScaleFactor = 1.5f;
    float windowScaleFactor = 1.2f;

    //reflektori
    glm::vec3 spotlightPositions[4] = {
            glm::vec3(5.0f, 5.0f, -5.0f),
            glm::vec3(-5.0f, 5.0f, 5.0f),
            glm::vec3(-5.0f, 5.0f, -5.0f),
            glm::vec3(5.0f, 5.0f, 5.0f)
    };
    //sijalice
    glm::vec3 circlePositions[4] = {
            glm::vec3(3.92f, 4.45f, -3.92f),
            glm::vec3(-3.92f, 4.45f, 3.92f),
            glm::vec3(-3.92f, 4.45f, -3.92f),
            glm::vec3(3.92f, 4.45f, 3.92f)
    };

            // ovo posle se ne menja
    ProgramState()
        :camera(glm::vec3(0.0f, 0.0f, 0.0f)){};


    void SaveToFile(std::string filename);
    void LoadFromFile(std::string filename);
};


void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

bool checkSpotlights[] = {
        false,false,false,false
};
bool allLightsActivated = false;
bool antialiasing=true;

ProgramState *programState;
Shader *shader_rb_car;
bool blinn = true;


void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw init, create window, glad && callback functions
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse

    // TODO , kada treba da se debaguje mora da se stavi na GLFFW_CURSOR_NORMAL inace je GLDF_CURSOR_DISABLED na pocetku
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(false);

    programState = new ProgramState;
    shader_rb_car = new Shader("resources/shaders/rb_car_shader.vs", "resources/shaders/rb_car_shader.fs");
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;


    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    // Shaders
    Shader spotlightShader("resources/shaders/spotlightShader.vs","resources/shaders/spotlightShader.fs");
    Shader skyboxShader("resources/shaders/skybox_shader.vs","resources/shaders/skybox_shader.fs");

    // Ocitavanje modela formule
    Model carModel("resources/objects/redbull-f1/Redbull-rb16b.obj");
    carModel.SetShaderTextureNamePrefix("material.");

    //model tyres
    Model tyresModel("resources/objects/pile-of-tires/source/tire.fbx");
    tyresModel.SetShaderTextureNamePrefix("material.");
    unsigned int tyresTextureDiffuse = loadTexture("resources/objects/pile-of-tires/textures/TexturesCom_Various_TireCar_1K_albedo.png");
    unsigned int tyresTextureSpecular = loadTexture("resources/objects/pile-of-tires/textures/TexturesCom_Various_TireCar_1K_ao.png");

    //model platform
    Model platform("resources/objects/platform/Rotating_Light_Platform_Final.fbx");
    platform.SetShaderTextureNamePrefix("material.");
    unsigned int platformTextureDiffuse = loadTexture("resources/objects/platform/lambert1_metallic.jpg");
    unsigned int platformTextureSpecular = loadTexture("resources/objects/platform/lambert1_roughness.jpg");

    // podloga teksture (asfalt diffuse i specular)
    unsigned int floorTextureDiffuse = loadTexture("resources/textures/asphalt.jpg");
    unsigned int floorTextureSpecular = loadTexture("resources/textures/floor/floor_specular.jpeg");

    // model lampe
    Model lamp("resources/objects/lamp/Euro Spot LED czarny 1f.obj");
    lamp.SetShaderTextureNamePrefix("material.");
    unsigned int textureLamp = loadTexture("resources/objects/lamp/metal.jpg");

    // model kruga
    Model circle("resources/objects/circle-obj/circle.obj");
    circle.SetShaderTextureNamePrefix("material.");

    // model trofeja
    Model trophy("resources/objects/trophy/trophy.obj");
    trophy.SetShaderTextureNamePrefix("material.");

    // model stola
    Model tableTrophy("resources/objects/table_model/102195.obj");
    tableTrophy.SetShaderTextureNamePrefix("material.");

    // model pipe
    Model pipe("resources/objects/tube/tube.obj");
    pipe.SetShaderTextureNamePrefix("material.");

    // model propeler
    Model propeler("resources/objects/propeller/Prop5in.fbx");
    propeler.SetShaderTextureNamePrefix("material.");

    // Svi objekti koji su osvetljenji lampom treba da koriste shader_rb_car
   // TODO Namestiti senke ili normal & parallax mapping

    SpotLight& spotLight = programState->spotLight;
    PointLight& pointLight = programState->pointLight;
    DirLight& dirLight = programState->dirLight;

    // inicijalizacija prozora
    vector<Prozor> &prozori = programState->prozori;
    initializeTransparentWindows(prozori);


    // gleda u (0,0,0) gde je auto, pozicija je definisana u samom konstruktoru
    // direction se i azurira u zavisnosti od pozicije samog svetla u render petlji
    spotLight.direction = glm::normalize(programState->carPosition - programState->spotLight.position);
    spotLight.ambient = glm::vec3(0.0f,0.0f,0.0f);
    spotLight.diffuse = glm::vec3(1.0f,1.0f,1.0f);
    spotLight.specular = glm::vec3(1.0f,1.0f,1.0f);
    spotLight.constant = 1.0f;
    spotLight.linear = 0.045f;
    spotLight.quadratic = 0.0005f;
    spotLight.cutOff = glm::cos(glm::radians(30.5f));
    spotLight.outerCutOff = glm::cos(glm::radians(45.0f));

    // pozicija je predefinisana
    pointLight.position = programState->pointLight.position;                   // trenutno spotlight osvetljava formulu a pointlight osvetljava sve
    pointLight.ambient = glm::vec3(0.2f,0.2f,0.2f);
    pointLight.diffuse = glm::vec3(0.7f,0.7f,0.7f);
    pointLight.specular = glm::vec3(1.0f,1.0f,1.0f);
    pointLight.constant = 1.0f;
    pointLight.linear = 0.08f;
    pointLight.quadratic = 0.006f;

    // direction light setup
    dirLight.direction = glm::vec3(-0.93f, 0.05f, -0.36f);
    dirLight.ambient = glm::vec3(0.08f);
    dirLight.diffuse = glm::vec3(0.3f);
    dirLight.specular = glm::vec3(0.4f);


    // cube data
    float cubeVertices[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    // lightCubeVAO with cube data
    unsigned int VBO, lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // floor data
    float floorVertices[] = {
            // positions          //normals            // texture coords
            1.0f,  0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    10.0f, 10.0f,
            1.0f, 0.0f, -1.0f,    0.0f, 1.0f, 0.0f,    10.0f, 0.0f,
            -1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 10.0f,
            -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,    0.0f, 0.0f
    };
    unsigned int floorIndices[] = {
            0, 1, 3, // first triangle
            0, 2, 3  // second triangle
    };
    // floorVAO with floor data
    unsigned int floorVBO, floorVAO, floorEBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glGenBuffers(1, &floorEBO);

    glBindVertexArray(floorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // windows transparent data
    float transparentVertices[] = {
            // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  1.0f,
            1.0f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  1.0f,

            0.0f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  0.0f,
            1.0f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  1.0f,
            1.0f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  0.0f
    };

    // transparent VAO with transparent data
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // transparenta tekstura - prozor
    unsigned int transparentTexture = loadTexture("resources/textures/window.png");

    // skybox data
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };
    // skyboxVAO with skybox data and texture loading from resources/textures/skybox/
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/skybox/right.jpg"),
                    FileSystem::getPath("resources/textures/skybox/left.jpg"),
                    FileSystem::getPath("resources/textures/skybox/top.jpg"),
                    FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
                    FileSystem::getPath("resources/textures/skybox/front.jpg"),
                    FileSystem::getPath("resources/textures/skybox/back.jpg")
            };

    unsigned int cubemapTexture = loadCubemap(faces);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // init blinn at start
    shader_rb_car->use();
    shader_rb_car->setBool("blinn", blinn);

    // Brzina pomeranja na tastaturi
    programState->camera.MovementSpeed = 7.0f;

    // render petlja
    while (!glfwWindowShouldClose(window)) {
        // FPS lock
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // update funkcija
        processInput(window);
        // sort transparent objects
        std::sort(prozori.begin(), prozori.end(),[cameraPosition = programState->camera.Position]
                (const Prozor a, const Prozor b){
                    float d1 = glm::distance(a.position, cameraPosition);
                    float d2 = glm::distance(b.position, cameraPosition);
                    return  d1 > d2;
        });
        // boja i dubina
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // projection i view matrica (uglavnom ostaje ista) i treba je postaviti kasnije sa model matricom u nekom shaderu
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        //inicijalizovana jedinicna model matrica
        glm::mat4 model = glm::mat4(1.0f);

        //Crtanje automobila
        shader_rb_car->use();
        shader_rb_car->setFloat("transparency", 1.0f);
        hasLights(*shader_rb_car, true, false, true);
        shader_rb_car->setMat4("projection", projection);
        shader_rb_car->setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0,1.0f, 0.0f));
        //model = glm::rotate(model, 0.25f * currentFrame, glm::vec3(0.0f,1.0f,0.0f));
        model = glm::translate(model, programState->carPosition);
        shader_rb_car->setMat4("model", model);

        shader_rb_car->setVec3("pointLights[0].position", programState->pointLight.position);
        shader_rb_car->setVec3("pointLights[0].ambient", programState->pointLight.ambient);
        shader_rb_car->setVec3("pointLights[0].diffuse", programState->pointLight.diffuse);
        shader_rb_car->setVec3("pointLights[0].specular", programState->pointLight.specular);
        shader_rb_car->setFloat("pointLights[0].constant", programState->pointLight.constant);
        shader_rb_car->setFloat("pointLights[0].linear", programState->pointLight.linear);
        shader_rb_car->setFloat("pointLights[0].quadratic",programState->pointLight.quadratic);


        shader_rb_car->setFloat("material.shininess", 32.0f);
        shader_rb_car->setVec3("viewPos", programState->camera.Position);

        // spotlight config 1
        shader_rb_car->setVec3("spotLight[0].position", programState->spotlightPositions[0]);
        spotLight.direction = glm::normalize(programState->carPosition - programState->spotlightPositions[0]);
        shader_rb_car->setVec3("spotLight[0].direction", programState->spotLight.direction);

        shader_rb_car->setVec3("spotLight[0].ambient", programState->spotLight.ambient);
        shader_rb_car->setVec3("spotLight[0].diffuse", programState->spotLight.diffuse);
        shader_rb_car->setVec3("spotLight[0].specular", programState->spotLight.specular);
        shader_rb_car->setFloat("spotLight[0].constant", programState->spotLight.constant);
        shader_rb_car->setFloat("spotLight[0].linear", programState->spotLight.linear);
        shader_rb_car->setFloat("spotLight[0].quadratic",programState->spotLight.quadratic);
        shader_rb_car->setFloat("spotLight[0].cutOff", programState->spotLight.cutOff);
        shader_rb_car->setFloat("spotLight[0].outerCutOff", programState->spotLight.outerCutOff);

        // spotlight config 2
        shader_rb_car->setVec3("spotLight[1].position", programState->spotlightPositions[1]);
        spotLight.direction = glm::normalize(programState->carPosition - programState->spotlightPositions[1]);
        shader_rb_car->setVec3("spotLight[1].direction", programState->spotLight.direction);

        shader_rb_car->setVec3("spotLight[1].ambient", programState->spotLight.ambient);
        shader_rb_car->setVec3("spotLight[1].diffuse", programState->spotLight.diffuse);
        shader_rb_car->setVec3("spotLight[1].specular", programState->spotLight.specular);
        shader_rb_car->setFloat("spotLight[1].constant", programState->spotLight.constant);
        shader_rb_car->setFloat("spotLight[1].linear", programState->spotLight.linear);
        shader_rb_car->setFloat("spotLight[1].quadratic",programState->spotLight.quadratic);
        shader_rb_car->setFloat("spotLight[1].cutOff", programState->spotLight.cutOff);
        shader_rb_car->setFloat("spotLight[1].outerCutOff", programState->spotLight.outerCutOff);

        // spotlight config 3
        shader_rb_car->setVec3("spotLight[2].position", programState->spotlightPositions[2]);
        spotLight.direction = glm::normalize(programState->carPosition - programState->spotlightPositions[2]);
        shader_rb_car->setVec3("spotLight[2].direction", programState->spotLight.direction);

        shader_rb_car->setVec3("spotLight[2].ambient", programState->spotLight.ambient);
        shader_rb_car->setVec3("spotLight[2].diffuse", programState->spotLight.diffuse);
        shader_rb_car->setVec3("spotLight[2].specular", programState->spotLight.specular);
        shader_rb_car->setFloat("spotLight[2].constant", programState->spotLight.constant);
        shader_rb_car->setFloat("spotLight[2].linear", programState->spotLight.linear);
        shader_rb_car->setFloat("spotLight[2].quadratic",programState->spotLight.quadratic);
        shader_rb_car->setFloat("spotLight[2].cutOff", programState->spotLight.cutOff);
        shader_rb_car->setFloat("spotLight[2].outerCutOff", programState->spotLight.outerCutOff);

        // spotlight config 4
        shader_rb_car->setVec3("spotLight[3].position", programState->spotlightPositions[3]);
        spotLight.direction = glm::normalize(programState->carPosition - programState->spotlightPositions[3]);
        shader_rb_car->setVec3("spotLight[3].direction", programState->spotLight.direction);

        shader_rb_car->setVec3("spotLight[3].ambient", programState->spotLight.ambient);
        shader_rb_car->setVec3("spotLight[3].diffuse", programState->spotLight.diffuse);
        shader_rb_car->setVec3("spotLight[3].specular", programState->spotLight.specular);
        shader_rb_car->setFloat("spotLight[3].constant", programState->spotLight.constant);
        shader_rb_car->setFloat("spotLight[3].linear", programState->spotLight.linear);
        shader_rb_car->setFloat("spotLight[3].quadratic",programState->spotLight.quadratic);
        shader_rb_car->setFloat("spotLight[3].cutOff", programState->spotLight.cutOff);
        shader_rb_car->setFloat("spotLight[3].outerCutOff", programState->spotLight.outerCutOff);

        // directional light config
        shader_rb_car->setVec3("dirLight.direction", dirLight.direction);
        shader_rb_car->setVec3("dirLight.ambient", dirLight.ambient);
        shader_rb_car->setVec3("dirLight.diffuse", dirLight.diffuse);
        shader_rb_car->setVec3("dirLight.specular", dirLight.specular);


        carModel.Draw(*shader_rb_car);

        // crtanje guma
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(programState->tyresPosition));
        model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0,1.0,0.0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0,0.0,0.0));
        model = glm::scale(model, glm::vec3(0.5f,0.5f,0.5f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tyresTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tyresTextureSpecular);
        shader_rb_car->setMat4("model", model);
        tyresModel.Draw(*shader_rb_car);

        // crtanje podloge
        hasLights(*shader_rb_car, true, false, true);
        glBindVertexArray(floorVAO);
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(10.0f,10.0f,10.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorTextureSpecular);
        shader_rb_car->setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        //Crtanje platforme
        model = glm::mat4(1.0f);
        model = glm::translate(model,programState->platformPosition);
        model = glm::scale(model, glm::vec3(0.12f, 0.12f, 0.12f));
        model= glm::rotate(model, 0.25f * currentFrame, glm::vec3(0.0f, 1.0f, 0.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, platformTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, platformTextureSpecular);
        shader_rb_car->setMat4("model", model);
        platform.Draw(*shader_rb_car);

        // lampe (reflektori)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureLamp);
        for(int i = 0; i < 4; i++){
            model = glm::mat4(1.0f);
            model = glm::translate(model, programState->spotlightPositions[i]);
            model = glm::scale(model, glm::vec3(0.07f, 0.07f, 0.07f));
            float rotation;
            switch(i){
                case 0:{
                    rotation = -0.78f;
                    break;
                }
                case 1:{
                    rotation = 2.35f;
                    break;
                }
                case 2:{
                    rotation = 0.78f;
                    break;
                }
                case 3:{
                    rotation = -2.35f;
                    break;
                }
            }
            model= glm::rotate(model, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
            shader_rb_car->setMat4("model", model);
            lamp.Draw(*shader_rb_car);
        }

        //trofej
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->trophyPosition);
        model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
        model= glm::rotate(model, 3.6f, glm::vec3(0.0f, 1.0f, 0.0f));
        shader_rb_car->setMat4("model", model);

        trophy.Draw(*shader_rb_car);

        // sto
        model = glm::mat4(1.0f);
        float scale = programState->tableScaleFactor;
        model = glm::scale(model, glm::vec3(scale));
        model = glm::translate(model, programState->tablePosition);
        shader_rb_car->setMat4("model", model);
        tableTrophy.Draw(*shader_rb_car);

        //pipe
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        model = glm::translate(model,programState->pipePosition);
        model= glm::rotate(model, 1.57f, glm::vec3(1.0f, 0.0f, 0.0f));
        shader_rb_car->setMat4("model", model);
        pipe.Draw(*shader_rb_car);

        //krug
        spotlightShader.use();
        spotlightShader.setMat4("view", view);
        spotlightShader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        model = glm::scale(model, programState->krugPosition);
        model = glm::translate(model,glm::vec3(-1.485f,0.0f,-1.485f) );
        spotlightShader.setVec3("Color",glm::vec3(0.0f));
        spotlightShader.setMat4("model", model);
        circle.Draw(spotlightShader);

        //propeler

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(7.5f, 7.5f, 7.5f));
        model = glm::translate(model,programState->propelerPosition );
        model= glm::rotate(model, 1.57f, glm::vec3(1.0f, 0.0f, 0.0f));
        model= glm::rotate(model, 0.25f * currentFrame, glm::vec3(0.0f, 0.0f, 1.0f));
        shader_rb_car->setMat4("model", model);
        propeler.Draw(spotlightShader);

        // sijalice
        spotlightShader.use();
        spotlightShader.setMat4("view", view);
        spotlightShader.setMat4("projection", projection);
        for(int i = 0; i < 4; i++){
            glm::vec3 boja;
            model = glm::mat4(1.0f);
            boja = checkSpotlights[i] ? glm::vec3(1.0f) : glm::vec3(0.0f);
            model = glm::scale(model, glm::vec3(1.2f,1.2f,1.2f));
            model = glm::translate(model, programState->circlePositions[i]);
            float rotation;
            switch(i){
                case 0:{
                    rotation = 2.1f;
                    model= glm::rotate(model, rotation, glm::vec3(1.0f, 0.0f, 1.0f));
                    break;
                }
                case 1:{
                    rotation = -2.1f;
                    model= glm::rotate(model, rotation, glm::vec3(1.0f, 0.0f, 1.0f));
                    break;
                }
                case 2:{
                    rotation = 2.1f;
                    model= glm::rotate(model, rotation, glm::vec3(1.0f, 0.0f, -1.0f));
                    break;
                }
                case 3:{
                    rotation = -2.1f;
                    model= glm::rotate(model, rotation, glm::vec3(1.0f, 0.0f, -1.0f));

                    break;
                }
            }
            if(checkSpotlights[i])
                boja=glm::vec3 (1.0f);
            else
                boja=glm::vec3(0.0f);

            spotlightShader.setVec3("Color",boja);
            spotlightShader.setMat4("model", model);
            circle.Draw(spotlightShader);
        }

        //crtanje pointlight svetla
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(10.0f));
        model = glm::translate(model, programState->pointLight.position);
        spotlightShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthFunc(GL_LEQUAL);  // zbog nepreciznosti racunanja u pokretnom zarezu se postavlja ova depth funkcija
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // translacija se uklanja zbog osecaja beskonacnosti
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // vracamo na feault

        // prozori ( blending) , mora na kraju nakon svih opaque objekata (sa alpha=1.0f) i pre skybox-a
        shader_rb_car->use();
        shader_rb_car->setFloat("transparency", 0.5f);
        hasLights(*shader_rb_car, true, false, true);
        glBindVertexArray(transparentVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);

        for(int i = 0; i < prozori.size(); ++i){
            model = glm::mat4(1.0f);
            model = glm::translate(model, prozori[i].position);
            model = glm::scale(model, glm::vec3(prozori[i].windowScaleFactor));
            model = glm::rotate(model, glm::radians(prozori[i].rotateX),glm::vec3(1.0,0.0,0.0));
            model = glm::rotate(model, glm::radians(prozori[i].rotateY),glm::vec3(0.0,1.0,0.0));
            model = glm::rotate(model, glm::radians(prozori[i].rotateZ),glm::vec3(0.0,0.0,1.0));
            shader_rb_car->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }


        // imgui

        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    delete shader_rb_car;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        //ImGui::SliderFloat("Transparency slider", &programState->transparency, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);

        // point light position
        ImGui::DragFloat3("Pozicija pointlight svetla", (float*)&programState->pointLight.position);


        // point light attenuation
        ImGui::InputDouble("pointLight.constant", &programState->pointLight.constant);
        ImGui::InputDouble("pointLight.linear", &programState->pointLight.linear);
        ImGui::InputDouble("pointLight.quadratic", &programState->pointLight.quadratic);

        ImGui::DragFloat3("Ambient directional", (float*)&programState->dirLight.ambient);
        ImGui::DragFloat3("Diffuse directional", (float*)&programState->dirLight.diffuse);
        ImGui::DragFloat3("Specular directional", (float*)&programState->dirLight.specular);

        // spotlight attenuation
        ImGui::InputDouble("spotLight.constant", &programState->spotLight.constant);
        ImGui::InputDouble("spotLight.linear", &programState->spotLight.linear);
        ImGui::InputDouble("spotLight.quadratic", &programState->spotLight.quadratic);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            programState->CameraMouseMovementUpdateEnabled = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    if(key == GLFW_KEY_1 && action == GLFW_PRESS){
        shader_rb_car->use();
        if(checkSpotlights[0]) {
            checkSpotlights[0] = !checkSpotlights[0];
            shader_rb_car->setInt("checkSpotlight[0]", 0);
        }
        else{
            checkSpotlights[0] = !checkSpotlights[0];
            shader_rb_car->setInt("checkSpotlight[0]", 1);
        }
    }
    if(key == GLFW_KEY_2 && action == GLFW_PRESS){
        shader_rb_car->use();
        if(checkSpotlights[1]) {
            checkSpotlights[1] = !checkSpotlights[1];
            shader_rb_car->setInt("checkSpotlight[1]", 0);
        }
        else{
            checkSpotlights[1] = !checkSpotlights[1];
            shader_rb_car->setInt("checkSpotlight[1]", 1);
        }
    }
    if(key == GLFW_KEY_3 && action == GLFW_PRESS){
        shader_rb_car->use();
        if(checkSpotlights[2]) {
            checkSpotlights[2] = !checkSpotlights[2];
            shader_rb_car->setInt("checkSpotlight[2]", 0);
        }
        else{
            checkSpotlights[2] = !checkSpotlights[2];
            shader_rb_car->setInt("checkSpotlight[2]", 1);
        }
    }
    if(key == GLFW_KEY_4 && action == GLFW_PRESS){
        shader_rb_car->use();
        if(checkSpotlights[3]) {
            checkSpotlights[3] = !checkSpotlights[3];
            shader_rb_car->setInt("checkSpotlight[3]", 0);
        }
        else{
            checkSpotlights[3] = !checkSpotlights[3];
            shader_rb_car->setInt("checkSpotlight[3]", 1);
        }
    }
    if(key == GLFW_KEY_M && action == GLFW_PRESS){
        if (antialiasing== true) {
            glDisable(GL_MULTISAMPLE);
            antialiasing=!antialiasing;
        }
       else {
            glEnable(GL_MULTISAMPLE);
            antialiasing = !antialiasing;
        }

        }

    if(key == GLFW_KEY_G && action == GLFW_PRESS){
        shader_rb_car->use();
        if(allLightsActivated){
            allLightsActivated = !allLightsActivated;
            for(int i = 0; i < 4; i++) {
                checkSpotlights[i] = false;
                std::string ime = "checkSpotlight[";
                ime.append(to_string(i));
                ime.append("]");
                shader_rb_car->setInt(ime,0);
            }
        }
        else{
            allLightsActivated = !allLightsActivated;
            for(int i = 0; i < 4; i++){
                checkSpotlights[i]=true;
                std::string ime = "checkSpotlight[";
                ime.append(to_string(i));
                ime.append("]");
                shader_rb_car->setInt(ime,1);
            }
        }
    }
    if(key == GLFW_KEY_B && action == GLFW_PRESS){
        shader_rb_car->use();
        blinn = !blinn;
        shader_rb_car->setBool("blinn", blinn);
    }
}
unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

void hasLights(Shader& shader, bool directional, bool pointLight, bool spotlight){
    //directional
    if(directional){
        shader.setInt("hasDirLight", 1);
    }
    else{
        shader.setInt("hasDirLight", 0);
    }
    // point light
    if(pointLight){
        shader.setInt("hasPointLight", 1);
    }
    else{
        shader.setInt("hasPointLight", 0);
    }
    // spotlight
    if(spotlight){
        shader.setInt("hasSpotLight", 1);
    }
    else{
        shader.setInt("hasSpotLight", 1);
    }
}

void initializeTransparentWindows(vector<Prozor> &prozori){
    Prozor p1;
    p1.position = glm::vec3(-5.5f,1.723f,5.69f);
    p1.windowScaleFactor = 1.2f;
    p1.rotateX = 0.0f;
    p1.rotateY = -20.0f;
    p1.rotateZ = 0.0f;
    prozori.push_back(p1);

    Prozor p2;
    p2.position = glm::vec3(-5.950f,1.723f,7.050f);
    p2.windowScaleFactor = 1.2f;
    p2.rotateX = 0.0f;
    p2.rotateY = -20.0f;
    p2.rotateZ = 0.0f;
    prozori.push_back(p2);

    Prozor p3;
    p3.position = glm::vec3(-6.03f,1.723f,6.900f);
    p3.windowScaleFactor = 1.2f;
    p3.rotateX = 0.0f;
    p3.rotateY = 68.5f;
    p3.rotateZ = 0.0f;
    prozori.push_back(p3);

    Prozor p4;
    p4.position = glm::vec3(-4.69f,1.723f,7.350f);
    p4.windowScaleFactor = 1.2f;
    p4.rotateX = 0.0f;
    p4.rotateY = 68.5f;
    p4.rotateZ = 0.0f;
    prozori.push_back(p4);

    Prozor p5;
    p5.position = glm::vec3(-5.8f,2.352f,6.365f);
    p5.windowScaleFactor = 1.4f;
    p5.rotateX = 90.0f;
    p5.rotateY = 0.0f;
    p5.rotateZ = 20.0f;
    prozori.push_back(p5);
}

unsigned int loadCubemap(vector<std::string> &faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}