// Local Headers
#include "glitter.hpp"
#include <Shader.hpp>
#include <Camera.hpp>
#include <Model.hpp>
#include <Skybox.hpp>
#include <GUI.hpp>
#include <ShadowMap.hpp>
#include <ShadowCubemap.hpp>
#include <CustomModel.hpp>
#include <ClothMesh.hpp>
#include <Tests.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
// To use stb_image, add this in *one* C++ source file.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <direct.h>

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void MouseMovementCallback(GLFWwindow* window, double x_pos, double y_pos);
void ProcessInput(GLFWwindow* window);
void AddModel(std::string& file);

Camera cam = Camera(glm::vec3(0.0f));
float deltaTime = 0.0f, lastFrame = 0.0f;
float lastX;
float lastY;
bool first_mouse_flag = true;

std::vector<Model> models(MAX_MODELS);
GUI* guiPointer;
Timer timer;
SceneSettings settings;
bool spacebar_down = false, p_down = false;
uint32_t nModels = 0;

int main(int argc, char * argv[]) {

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGLTemplate", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    glfwSetFramebufferSizeCallback(mWindow, FramebufferSizeCallback);
    glfwSetCursorPosCallback(mWindow, MouseMovementCallback);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    // tell GLFW to capture our mouse
    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // OpenCL initialization
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if (all_platforms.size() == 0) {
        std::cout << " No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    cl::Platform default_platform = all_platforms[0];
    std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if (all_devices.size() == 0) {
        std::cout << " No devices found.\n";
        exit(1);
    }

    default_device = all_devices[0];
    std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";

    cl_context_properties properties[] =
    {
      CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
      CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
      CL_CONTEXT_PLATFORM, (cl_context_properties)default_platform(),
      NULL
    };

    cl_int err = CL_SUCCESS;
    context = clCreateContext(properties, 1, &default_device(), NULL, NULL, &err);

    if (err != CL_SUCCESS) {
        std::cout << "Error creating context" << " " << err << "\n";
        //exit(-1);
    }

    queue = cl::CommandQueue(context, default_device);
    
    // Read kernel source
    char buffer[1024];
    getcwd(buffer, 1024);
    std::string kernel_char(buffer);
    kernel_char += "\\..\\Template\\Sources\\gpu_src\\test.cl";
    kernel_source = ReadFile2(kernel_char.c_str());
    sources.push_back({ kernel_source.c_str(), kernel_source.length() });

    // Build program and compile
    program = cl::Program(context, sources);

    if (program.build({ default_device }) != CL_SUCCESS)
    {
        std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
        exit(1);
    }

    // Prepare buffers
    debug_buffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * mWidth * mHeight);

    // OpenGL shaders
    std::string vs_char(buffer);
    std::string fs_char(buffer);
    vs_char += "\\..\\Template\\Shaders\\basic_model.vert";
    fs_char += "\\..\\Template\\Shaders\\basic_model.fs";
    Shader simple_shader(vs_char.c_str(), fs_char.c_str());

    std::string lightingVertChar(buffer);
    std::string lightingFragChar(buffer);
    lightingVertChar += "\\..\\Template\\Shaders\\lighting_shader.vert";
    lightingFragChar += "\\..\\Template\\Shaders\\lighting_shader.frag";
    Shader lightingShader(lightingVertChar.c_str(), lightingFragChar.c_str());
    lightingShader.use();
    lightingShader.setInt("shadowMap", 4);
    lightingShader.setInt("shadowCubemap", 5);

    std::string customVertChar(buffer);
    std::string customFragChar(buffer);
    customVertChar += "\\..\\Template\\Shaders\\customModelShader.vert";
    customFragChar += "\\..\\Template\\Shaders\\customModelShader.frag";
    Shader customModelShader(customVertChar.c_str(), customFragChar.c_str());

    std::string sky_vs_char(buffer);
    std::string sky_fs_char(buffer);
    sky_vs_char += "\\..\\Template\\Shaders\\skybox.vert";
    sky_fs_char += "\\..\\Template\\Shaders\\skybox.frag";
    Shader skyShader(sky_vs_char.c_str(), sky_fs_char.c_str());

    std::string shadowVertChar(buffer);
    std::string shadowFragChar(buffer);
    shadowVertChar += "\\..\\Template\\Shaders\\shadowShader.vert";
    shadowFragChar += "\\..\\Template\\Shaders\\shadowShader.frag";
    Shader shadowShader(shadowVertChar.c_str(), shadowFragChar.c_str());

    std::string shadowCubeVertChar(buffer);
    std::string shadowCubeGeomChar(buffer);
    std::string shadowCubeFragChar(buffer);
    shadowCubeVertChar += "\\..\\Template\\Shaders\\shadowCubeShader.vert";
    shadowCubeGeomChar += "\\..\\Template\\Shaders\\shadowCubeShader.geom";
    shadowCubeFragChar += "\\..\\Template\\Shaders\\shadowCubeShader.frag";
    Shader shadowCubeShader(shadowCubeVertChar.c_str(), shadowCubeFragChar.c_str(), shadowCubeGeomChar.c_str());

    std::string quadVertChar(buffer);
    std::string quadFragChar(buffer);
    quadVertChar += "\\..\\Template\\Shaders\\quad.vert";
    quadFragChar += "\\..\\Template\\Shaders\\quad.frag";
    Shader quadShader(quadVertChar.c_str(), quadFragChar.c_str());
    quadShader.use();
    quadShader.setInt("screenTexture", 0);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    // Setup OpenGL Buffers
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hardcoded_vertices), hardcoded_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // OpenGL texture
    unsigned int gl_texture;
    glGenTextures(1, &gl_texture);
    glBindTexture(GL_TEXTURE_2D, gl_texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;

    std::string tex_char(buffer);
    tex_char += "\\..\\textures\\wall.jpg";
    unsigned char* data = stbi_load(tex_char.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    glGenerateMipmap(GL_TEXTURE_2D);

    target_texture = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_texture, &err);
    std::cout << "Created CL Image2D with err:\t" << err << std::endl;

    // Flush GL queue        
    glFinish();
    glFlush();

    // Acquire shared objects
    err = clEnqueueAcquireGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;

    tester = cl::Kernel(program, "tex_test");
    cl::NDRange global_test(width, height);
    tester(cl::EnqueueArgs(queue, global_test), target_texture).wait();

    // We have to generate the mipmaps again!!!
    glGenerateMipmap(GL_TEXTURE_2D);

    // Release shared objects                                                          
    err = clEnqueueReleaseGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
    std::cout << "Releasing GL objects with err:\t" << err << std::endl;

    // Flush CL queue
    err = clFinish(queue());
    std::cout << "Finished CL queue with err:\t" << err << std::endl;

    // Create Camera
    cam = Camera(glm::vec3(0.0f));

    // Load Test model
    std::string modelChar(buffer);
    modelChar += "\\..\\models\\backpack.obj";
    AddModel(modelChar);

    // Load goblets
    std::string gobletChar(buffer);
    gobletChar += "\\..\\models\\brass_goblets_2k.obj";
    AddModel(gobletChar);

    // Load floor
    std::string floorChar(buffer);
    floorChar += "\\..\\models\\floor.obj";
    AddModel(floorChar);

    // Load skybox
    std::string skyChar(buffer);
    skyChar += "\\..\\textures\\Yokohama3\\";
    Skybox sky(skyChar, skyShader);

    GUI gui(mWindow, cam, settings, timer);
    guiPointer = &gui;
    gui.Init(nModels);

    // Shadow maps
    ShadowMap shadow(shadowShader, quadShader, 2048, 2048, mWidth, mHeight);
    ShadowCubemap shadowCubemap(shadowCubeShader, quadShader, 2048, 2048, mWidth, mHeight);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mWidth, mHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // create a color attachment texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // Custom model
    CustomModel testCustom(customDebug);

    // Cloth mesh
    //ClothMesh cloth(5.0f, 5.0f, 8, 8, 10, "clothPineapple.png");
    ClothMesh cloth(5.0f, 5.0f, 8, 8, 16, "clothPineapple.png");

    // Seed RNGs
    srand(static_cast <unsigned> (time(0)));

    // Test sphere intersections
    //SphereIntersectionTesting();

    //return true;

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false)
    {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        timer.Tick();

        // input
        // -----
        ProcessInput(mWindow);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render to depth map
        shadow.Render(settings.light_position, glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, shadow.nearPlane, shadow.farPlane), models, nModels, gui,
            &testCustom, &cloth);

        // Render to omnidirectional depth map
        shadowCubemap.Render(settings.point_light_position, models, nModels, gui, &testCustom, &cloth);

        // Switch to regular framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mWidth, mHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //shadow.Debug();

        if (settings.wireframe_mode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        cam.UpdateVelocity(deltaTime);
        cam.MoveCamera(deltaTime);

        // view/projection transformations
        glm::mat4 projection = cam.GetCurrentProjectionMatrix(mWidth, mHeight);
        glm::mat4 view = cam.GetCurrentViewMatrix();

        sky.Render(view, projection);

        // Update vertices
        testCustom.UpdateVertices(currentFrame);

        // Render Custom Model
        customModelShader.use();
        customModelShader.setMat4("projection", projection);
        customModelShader.setMat4("view", view);
        testCustom.Render(customModelShader, glm::mat4(1.0f));

        // Render cloth
        if (settings.run_sim)
        {
            cloth.Simulate(settings.sim_wind, settings.sim_wind_amount, settings.sim_drag, settings.sim_drag_amount,
                gui.clothSettings.GetModelMatrix(), static_cast<float>(timer.GetData().DeltaTime) * settings.sim_speed);
            cloth.UpdateVertices(currentFrame);
        }
        if (gui.clothSettings.enabled)
            cloth.Render(customModelShader, gui.clothSettings.GetModelMatrix());

        lightingShader.use();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        lightingShader.setVec3("viewPos", cam.position);
        lightingShader.setVec3("lightPos", glm::vec3(settings.light_position[0], settings.light_position[1], settings.light_position[2]));
        lightingShader.setVec3("lightColor", glm::vec3(settings.light_color[0], settings.light_color[1], settings.light_color[2]));
        lightingShader.setFloat("minBias", settings.manual_min_bias);
        lightingShader.setFloat("maxBias", settings.manual_max_bias);
        lightingShader.setFloat("bias", settings.manual_bias);
        lightingShader.setInt("shadowSamples", settings.shadow_samples);
        lightingShader.setBool("directionalShadows", settings.directional_shadows_on);
        lightingShader.setBool("omniShadows", settings.omnidirectional_shadows_on);
        lightingShader.setBool("normalMapOn", settings.use_normal_map);
        lightingShader.setFloat("farPlane", shadowCubemap.farPlane);
        lightingShader.setVec3("pointLightPos", glm::vec3(settings.point_light_position[0], settings.point_light_position[1], settings.point_light_position[2]));

        // Shadow stuff
        lightingShader.setMat4("lightSpaceMatrix",
            shadow.GetLightSpaceMatrix(settings.light_position, glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, shadow.nearPlane, shadow.farPlane)));
        
        // TODO: Remove from hardcoded!
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, shadow.depthMap);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubemap.depthCubemap);

        for (size_t i = 0; i < nModels; i++)
            if (gui.modelSets[i].enabled)
                models[i].Draw(lightingShader, gui.modelSets[i].GetModelMatrix());

        // Render GUI
        gui.Render();

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return EXIT_SUCCESS;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void ProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Ignore Keyboard Inputs for Camera Movement if arcball_mode == true
    if (cam.arcball_mode)
        return;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam.MoveCamera(FWD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam.MoveCamera(AFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam.MoveCamera(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam.MoveCamera(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cam.MoveCamera(UPWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cam.MoveCamera(DOWNWARD, deltaTime);

    // Enable/Disable Camera
    if (spacebar_down && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        cam.enabled = !cam.enabled;

        // Enable/Disable Cursor
        if (cam.enabled)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        spacebar_down = false;
    }

    if (!spacebar_down && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        spacebar_down = true;

    if (p_down && glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
    {
        settings.run_sim = !settings.run_sim;
        p_down = false;
    }

    if (!p_down && glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        p_down = true;
}

void MouseMovementCallback(GLFWwindow* window, double x_pos, double y_pos)
{
    float xpos = static_cast<float>(x_pos);
    float ypos = static_cast<float>(y_pos);

    if (first_mouse_flag)
    {
        lastX = xpos;
        lastY = ypos;
        first_mouse_flag = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    if (cam.arcball_mode)
        cam.RotateArcballCamera(xoffset, yoffset, mWidth, mHeight, deltaTime);
    else
        cam.RotateCamera(xoffset, yoffset);
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void AddModel(std::string& file)
{
    if (nModels >= MAX_MODELS - 1)
        throw std::runtime_error("max models reached!");

    models[nModels] = Model(file);
    nModels++;
    std::cout << "Loaded model " << file << std::endl;
}
