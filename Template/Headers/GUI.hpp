#pragma once

#include <Camera.hpp>
#include <Timer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

/// <summary>
/// This enum stores different UI button callback types
/// </summary>
enum class GUI_BUTTON
{
    MODEL_TOGGLE,
    CAMERA_MODE_TOGGLE
};

/// <summary>
/// The SceneSettings struct stores light information,
/// default color information,
/// as well as different input or render modes
/// </summary>
struct SceneSettings {
    float light_position[3] = { -2.0f, 4.0f, -1.0f };
    float point_light_position[3] = { 0.0f, 3.0f, 0.0f };
    float base_color[3];
    float light_color[3] = { 0.3f, 0.3f, 0.3f };
    float manual_min_bias = 0.005f;
    float manual_max_bias = 0.05f;
    float manual_bias = 0.05f;
    int shadow_samples = 40;
    float sim_speed = 1.0f;
    float sim_drag_amount = 0.001f;
    bool wireframe_mode;
    bool directional_shadows_on = false;
    bool omnidirectional_shadows_on = true;
    bool use_normal_map = true;
    bool run_sim = false;
    bool sim_drag = false;
};

/// <summary>
/// Stores the rendering information for each loaded model
/// </summary>
struct ModelSettings {
    float translation[3] = { 0.0f, 0.0f, 0.0f };
    float scale[3] = { 0.1f, 0.1f, 0.1f };
    bool enabled = true;

    /// <summary>
    /// Constructs the model matrix based on translation and scale vectors
    /// </summary>
    /// <returns></returns>
    glm::mat4 GetModelMatrix()
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(translation[0], translation[1], translation[2]));
        model = glm::scale(model, glm::vec3(scale[0], scale[1], scale[2]));

        return model;
    }
};

/// <summary>
/// GUI wrapper that handles all imgui related calls
/// </summary>
class GUI
{
public:
    std::vector<ModelSettings> modelSets;
    ModelSettings customModelSettings, clothSettings;

    GUI(GLFWwindow* pWindow, Camera& camera, SceneSettings& sceneSettings, Timer& timer);

    /// <summary>
    /// Initialize our GUI wrapper
    /// </summary>
    void Init(size_t nModels);

    /// <summary>
    /// Render our GUI with updated reference data
    /// </summary>
    void Render();

    /// <summary>
    /// Perform GUI cleanup
    /// </summary>
    void Cleanup();

private:
    /// <summary>
    /// The GUI callback is used to update our reference's state using the GUI_BUTTON enum
    /// </summary>
    void GuiButtonCallback(GUI_BUTTON button);

    GLFWwindow* p_window;
    Camera& m_camera;
    SceneSettings& m_sceneSettings;
    Timer& m_timer;
    std::string m_cameraMode;
    int nModels;
};