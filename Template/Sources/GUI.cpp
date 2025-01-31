#include <GUI.hpp>

GUI::GUI(GLFWwindow* pWindow, Camera& camera, SceneSettings& sceneSettings, Timer& timer)
    :
    p_window(pWindow),
    m_camera(camera),
    m_sceneSettings(sceneSettings),
    m_timer(timer),
    m_cameraMode("Camera Type: Normal Camera")
{
    //
}

void GUI::Init(size_t nModels)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(p_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Initialize model settings
    this->nModels = nModels;
    modelSets.resize(nModels);

    // Initialize floor model settings
    modelSets[2].translation[1] = -2.0f;
    modelSets[2].scale[0] = 0.5f;
    modelSets[2].scale[1] = 0.5f;
    modelSets[2].scale[2] = 0.5f;

    // Initialize cloth settings
    clothSettings.translation[1] = 6.0f;
    clothSettings.scale[0] = 1.0f;
    clothSettings.scale[1] = 1.0f;
    clothSettings.scale[2] = 1.0f;
}

void GUI::Render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    TimeData time = m_timer.GetData();
    std::string label;

    ImGui::Begin("Control Window");
    ImGui::Text("DeltaTime: %f", time.DeltaTime);
    ImGui::Text("FPS: %.2f", time.FPS);
    ImGui::Text("Use SPACEBAR to enable/disable cursor!");

    ImGui::ColorEdit3("Base color", (float*)m_sceneSettings.base_color);
    ImGui::ColorEdit3("Manual light color", (float*)m_sceneSettings.light_color);
    ImGui::SliderFloat3("Light position", m_sceneSettings.light_position, -10.0f, 25.0f);
    ImGui::SliderFloat3("Point light position", m_sceneSettings.point_light_position, -10.0f, 25.0f);
    ImGui::SliderFloat("Manual min bias", &m_sceneSettings.manual_min_bias, 0.0f, 1.0f);
    ImGui::SliderFloat("Manual max bias", &m_sceneSettings.manual_max_bias, 0.0f, 1.0f);
    ImGui::SliderFloat("Manual bias", &m_sceneSettings.manual_bias, 0.0f, 1.0f);
    ImGui::SliderInt("Shadow samples", &m_sceneSettings.shadow_samples, 16, 64);
    ImGui::Checkbox("Toggle wireframe", &m_sceneSettings.wireframe_mode);
    ImGui::Checkbox("Toggle normal map", &m_sceneSettings.use_normal_map);
    ImGui::Checkbox("Toggle directional shadows", &m_sceneSettings.directional_shadows_on);
    ImGui::Checkbox("Toggle omnidirectional shadows", &m_sceneSettings.omnidirectional_shadows_on);
    ImGui::Text("%s", m_cameraMode.c_str());
    if (ImGui::Button("Switch Camera Modes"))
        GuiButtonCallback(GUI_BUTTON::CAMERA_MODE_TOGGLE);

    ImGui::Separator();
    ImGui::Text("Simulation settings");
    ImGui::SliderFloat("Speed", &m_sceneSettings.sim_speed, 0.01f, 1.0f, "%.2f");
    ImGui::SliderFloat("Drag amount", &m_sceneSettings.sim_drag_amount, 0.01f, 2.0f, "%.2f");
    ImGui::Checkbox("Drag on", &m_sceneSettings.sim_drag);
    ImGui::SliderFloat("Wind amount", &m_sceneSettings.sim_wind_amount, 0.01f, 2.0f, "%.2f");
    ImGui::Checkbox("Wind on", &m_sceneSettings.sim_wind);
    ImGui::Checkbox("Play", &m_sceneSettings.run_sim);
    std::string strEnabled = std::string("Cloth enabled");
    std::string strTranslation = std::string("Cloth translation");
    std::string strScale = std::string("Cloth scaling");
    ImGui::Checkbox(strEnabled.c_str(), &clothSettings.enabled);
    ImGui::SliderFloat3(strTranslation.c_str(), clothSettings.translation, -10.0f, 10.0f);
    ImGui::SliderFloat3(strScale.c_str(), clothSettings.scale, 0.001f, 2.0f);

    ImGui::Separator();
    ImGui::Text("Models");
    for (size_t i = 0; i < nModels; i++)
    {
        const std::string strEnabled = std::string("Model ") + std::to_string(i) + " enabled";
        const std::string strTranslation = std::string("Model ") + std::to_string(i) + " translation";
        const std::string strScale = std::string("Model ") + std::to_string(i) + " scaling";
        ImGui::Checkbox(strEnabled.c_str(), &modelSets[i].enabled);
        ImGui::SliderFloat3(strTranslation.c_str(), modelSets[i].translation, -10.0f, 10.0f);
        ImGui::SliderFloat3(strScale.c_str(), modelSets[i].scale, 0.001f, 2.0f);
    }
    ImGui::Separator();
    strEnabled = std::string("Custom Model enabled");
    strTranslation = std::string("Custom Model translation");
    strScale = std::string("Custom Model scaling");
    ImGui::Checkbox(strEnabled.c_str(), &customModelSettings.enabled);
    ImGui::SliderFloat3(strTranslation.c_str(), customModelSettings.translation, -10.0f, 10.0f);
    ImGui::SliderFloat3(strScale.c_str(), customModelSettings.scale, 0.001f, 2.0f);

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::Cleanup()
{
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUI::GuiButtonCallback(GUI_BUTTON button)
{
    switch (button)
    {
    case GUI_BUTTON::MODEL_TOGGLE:
        break;
    case GUI_BUTTON::CAMERA_MODE_TOGGLE:
        m_camera.arcball_mode = !m_camera.arcball_mode;
        m_cameraMode = m_camera.arcball_mode ?
            std::string("Camera Type: Arcball Camera") : std::string("Camera Type: Normal Camera");
        break;
    default:
        break;
    }
}