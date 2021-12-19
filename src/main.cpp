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

// moze da se doda sta god, mora samo posle ako hoce da se sacuva da se navede i u SaveToFile i u LoadFromFile
struct ProgramState {
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;

    // --Ovde ide ono sto mi dodajemo, ostalo se ne dira--

    glm::vec3 clearColor = glm::vec3(0.0f);
    SpotLight spotLight;
    PointLight pointLight;
    glm::vec3 tyresPosition = glm::vec3(6.0f, 0.39f, 0.0f);
    glm::vec3 carPosition = glm::vec3(0.0f, 1.205f, 0.45f);
    glm::vec3 platformPosition = glm::vec3(0.0f, 0.4321f, 0.0f);
    glm::vec3 trophyPosition = glm::vec3(5.0f,1.0f,7.0f);
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
        << camera.Front.z << '\n'
        << trophyPosition.x << '\n'
        << trophyPosition.y << '\n'
        << trophyPosition.z << '\n';
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
           >> camera.Front.z
           >> trophyPosition.x
           >> trophyPosition.y
           >> trophyPosition.z;
    }
}

bool checkSpotlights[] = {
        false,false,false,false
};
bool allLightsActivated = false;


ProgramState *programState;
Shader *shader_rb_car;


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

    // glfw window creation
    // --------------------
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
    // vratiti na disabled
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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

    // Ocitavanje modela formule
    Model carModel("resources/objects/redbull-f1/Redbull-rb16b.obj");
    carModel.SetShaderTextureNamePrefix("material.");

    //tyres
    Model tyresModel("resources/objects/pile-of-tires/source/tire.fbx");
    tyresModel.SetShaderTextureNamePrefix("material.");
    unsigned int tyresTextureDiffuse = loadTexture("resources/objects/pile-of-tires/textures/TexturesCom_Various_TireCar_1K_albedo.png");
    unsigned int tyresTextureSpecular = loadTexture("resources/objects/pile-of-tires/textures/TexturesCom_Various_TireCar_1K_ao.png");

    //platform
    Model platform("resources/objects/platform/Rotating_Light_Platform_Final.fbx");
    platform.SetShaderTextureNamePrefix("material.");
    unsigned int platformTextureDiffuse = loadTexture("resources/objects/platform/lambert1_metallic.jpg");
    unsigned int platformTextureSpecular = loadTexture("resources/objects/platform/lambert1_roughness.jpg");

    // podloga
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

    // Svi objekti koji su osvetljenji lampom treba da koriste ovu strukturu za spotlight (ako bude vise lampi, pravi se niz)
   // TODO podloga treba da se ucita i napravi na isti nacin sa ovim shaderom. Bice Bag da kada se postavi podloga, mali deo ispod auta ce biti potpuno osvetljen, ali kada namestimo senke to nece biti slucaj

    SpotLight& spotLight = programState->spotLight;
    PointLight& pointLight = programState->pointLight;

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


    /* TODO treba da se zameni ovaj kocka lampion sa pavi objekat lampom
     * Iskoristi ovo za postavljanje koordinata modela
    */

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

    // render petlja
    programState->camera.MovementSpeed = 7.0f; // Brzina pomeranja na tastaturi
    while (!glfwWindowShouldClose(window)) {
        // FPS lock
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // update funkcija
        processInput(window);

        //msaa antialiasing
        glEnable(GL_MULTISAMPLE);

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
        hasLights(*shader_rb_car, false, true, true);
        shader_rb_car->setMat4("projection", projection);
        shader_rb_car->setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0,1.0f, 0.0f));
        //model = glm::rotate(model, 0.25f * currentFrame, glm::vec3(0.0f,1.0f,0.0f));
        model = glm::translate(model, programState->carPosition);
        shader_rb_car->setMat4("model", model);

//        shader_rb_car->setInt("hasPointLight",1); // kada je 1 ima pointlight svetlo, kada je 0 nema point light svetlo
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
        hasLights(*shader_rb_car, false, true, true);
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

        //sijalice
        spotlightShader.use();
        spotlightShader.setMat4("view", view);
        spotlightShader.setMat4("projection", projection);
        for(int i = 0; i < 4; i++){
            model = glm::mat4(1.0f);
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
            glm::vec3 boja;

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
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);

        // point light position
        ImGui::DragFloat3("Pozicija pointlight svetla", (float*)&programState->pointLight.position);

        // pozicija trofeja
        ImGui::DragFloat3("Pozicija trofeja", (float*)&programState->trophyPosition);

        // point light attenuation
        ImGui::InputDouble("pointLight.constant", &programState->pointLight.constant);
        ImGui::InputDouble("pointLight.linear", &programState->pointLight.linear);
        ImGui::InputDouble("pointLight.quadratic", &programState->pointLight.quadratic);

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
    if(key == GLFW_KEY_G && action == GLFW_PRESS){
        shader_rb_car->use();
        if(allLightsActivated){
            allLightsActivated = !allLightsActivated;
            for(int i = 0; i < 4; i++) {
                checkSpotlights[i]=false;
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