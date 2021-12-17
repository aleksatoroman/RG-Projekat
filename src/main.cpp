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
    glm::vec3 tyresPosition = glm::vec3(5.0f, 1.0f, 0.0f);
    glm::vec3 carPosition = glm::vec3(0.0f,2.0f,0.0f);
    glm::vec3 platformPosition = glm::vec3(0.0f, -0.75f, 0.0);

    //
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
        << spotLight.position.x << '\n'
        << spotLight.position.y << '\n'
        << spotLight.position.z << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n'
        << tyresPosition.x << '\n'
        << tyresPosition.y << '\n'
        << tyresPosition.z << '\n'
        << carPosition.x << '\n'
        << carPosition.y << '\n'
        << carPosition.z << '\n'
        << platformPosition.x << '\n'
        << platformPosition.y << '\n'
        << platformPosition.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> spotLight.position.x
           >> spotLight.position.y
           >> spotLight.position.z
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z
           >> tyresPosition.x
           >> tyresPosition.y
           >> tyresPosition.z
           >> carPosition.x
           >> carPosition.y
           >> carPosition.z
           >> platformPosition.x
           >> platformPosition.y
           >> platformPosition.z;
    }
}

ProgramState *programState;


void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    // Shaders
    Shader shader_rb_car("resources/shaders/rb_car_shader.vs", "resources/shaders/rb_car_shader.fs");
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
    unsigned int floorTextureDiffuse = loadTexture("resources/textures/floor/floorr.jpg");
    unsigned int floorTextureSpecular = loadTexture("resources/textures/floor/floor_specular.jpeg");

    // model lampe
    Model lamp("resources/objects/lamp/Euro Spot LED czarny 1f.obj");
    lamp.SetShaderTextureNamePrefix("material.");
    unsigned int textureLamp = loadTexture("resources/objects/lamp/metal.jpg");

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
        shader_rb_car.use();
        hasLights(shader_rb_car, false, true, true);
        shader_rb_car.setMat4("projection", projection);
        shader_rb_car.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, 0.25f * currentFrame, glm::vec3(0.0f,1.0f,0.0f));
        model = glm::translate(model, programState->carPosition);
        shader_rb_car.setMat4("model", model);

//        shader_rb_car.setInt("hasPointLight",1); // kada je 1 ima pointlight svetlo, kada je 0 nema point light svetlo
        shader_rb_car.setVec3("pointLights[0].position", programState->pointLight.position);
        shader_rb_car.setVec3("pointLights[0].ambient", programState->pointLight.ambient);
        shader_rb_car.setVec3("pointLights[0].diffuse", programState->pointLight.diffuse);
        shader_rb_car.setVec3("pointLights[0].specular", programState->pointLight.specular);
        shader_rb_car.setFloat("pointLights[0].constant", programState->pointLight.constant);
        shader_rb_car.setFloat("pointLights[0].linear", programState->pointLight.linear);
        shader_rb_car.setFloat("pointLights[0].quadratic",programState->pointLight.quadratic);


        shader_rb_car.setFloat("material.shininess", 32.0f);
        shader_rb_car.setVec3("viewPos", programState->camera.Position);

        // spotlight configurations
        shader_rb_car.setVec3("spotLight.position", programState->spotLight.position);
        spotLight.direction = glm::normalize(programState->carPosition - programState->spotLight.position);
        shader_rb_car.setVec3("spotLight.direction", programState->spotLight.direction);

        shader_rb_car.setVec3("spotLight.ambient", programState->spotLight.ambient);
        shader_rb_car.setVec3("spotLight.diffuse", programState->spotLight.diffuse);
        shader_rb_car.setVec3("spotLight.specular", programState->spotLight.specular);
        shader_rb_car.setFloat("spotLight.constant", programState->spotLight.constant);
        shader_rb_car.setFloat("spotLight.linear", programState->spotLight.linear);
        shader_rb_car.setFloat("spotLight.quadratic",programState->spotLight.quadratic);
        shader_rb_car.setFloat("spotLight.cutOff", programState->spotLight.cutOff);
        shader_rb_car.setFloat("spotLight.outerCutOff", programState->spotLight.outerCutOff);

        carModel.Draw(shader_rb_car);
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
        shader_rb_car.setMat4("model", model);
        tyresModel.Draw(shader_rb_car);

        // crtanje podloge
        hasLights(shader_rb_car, false, true, true);
        glBindVertexArray(floorVAO);
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(10.0f,10.0f,10.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorTextureSpecular);
        shader_rb_car.setMat4("model", model);
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
        shader_rb_car.setMat4("model", model);
        platform.Draw(shader_rb_car);

        // lampa 1
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureLamp);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(5.0f, 5.0f, -5.0f));

        model = glm::scale(model, glm::vec3(0.07f, 0.07f, 0.07f));
        model= glm::rotate(model, -0.7f, glm::vec3(0.0f, 1.0f, 0.0f));

        shader_rb_car.setMat4("model", model);
        lamp.Draw(shader_rb_car);

        // lampa 2
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.0f, 5.0f, 5.0f));

        model = glm::scale(model, glm::vec3(0.07f, 0.07f, 0.07f));
        model= glm::rotate(model, 2.5f, glm::vec3(0.0f, 1.0f, 0.0f));
        shader_rb_car.setMat4("model", model);

        lamp.Draw(shader_rb_car);

        //crtanje spotlight reflektora
        glBindVertexArray(lightCubeVAO);
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.8,0.8,0.8));
        model=glm::translate(model, programState->spotLight.position);
        spotlightShader.use();
        spotlightShader.setMat4("view", view);
        spotlightShader.setMat4("projection", projection);
        spotlightShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //crtanje pointlight svetla
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.5f,0.5f,0.5f));
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

        // spotlight position
        ImGui::DragFloat3("Pozicija spotlight reflektora", (float*)&programState->spotLight.position);
        // point light position
        ImGui::DragFloat3("Pozicija pointlight svetla", (float*)&programState->pointLight.position);
        // tyres position
        ImGui::DragFloat3("Pozicija guma",(float*)&programState->tyresPosition);
        // car position
        ImGui::DragFloat3("Pozicija auta",(float*)&programState->carPosition);
        // platform position
        ImGui::DragFloat3("Pozicija platforme",(float*)&programState->platformPosition);

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