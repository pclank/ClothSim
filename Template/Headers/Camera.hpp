#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <algorithm>

// Camera Translation Directions
enum Movement_Direction {
	FWD,
	AFT,
	LEFT,
	RIGHT,
	UPWARD,
	DOWNWARD
};

// Constraints and Defaults
const float PITCH_CONSTRAINT = 89.0f;
const float YAW = -90.0f;
const float PITCH = 0.0f;
//const float MAX_SPEED = 50.0f;
const float MAX_SPEED = 10.0f;
const float SENSITIVITY = 0.1f;
const float ARCBALL_SENSITIVITY = 5.0f;
const float ARCBALL_MOVE_SENSITIVITY = 0.2f;
const float FOV = 45.0f;
const float Z_NEAR = 0.1f;
const float Z_FAR = 100.0f;
const float DRAG = 2.0f;
const glm::vec3 FRONT = glm::vec3(0.0f, 0.0f, 1.0f);
const glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);

class Camera {
public:
	// Position Attributes
	glm::vec3 position;
	glm::vec3 up;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 world_up;

	// Rotation Attributes
	float yaw;
	float pitch;

	// Settings
	float movement_speed;
	float look_sensitivity;
	float arcball_sensitivity;
	float arcball_move_sensitivity;
	float fov;
	glm::vec3 velocity;
	bool enabled;
	bool arcball_mode;

	// Customizable Constructor
	Camera(glm::vec3 start_position, float start_yaw, float start_pitch, glm::vec3 start_up);
	// Simple Constructor
	Camera(glm::vec3 start_position);

	~Camera();

	// Get View Matrix
	glm::mat4 GetCurrentViewMatrix();

	// Get Projection Matrix
	glm::mat4 GetCurrentProjectionMatrix(float screen_width, float screen_height);

	// Move Camera from Keyboard Input
	void MoveCamera(Movement_Direction direction, float deltaTime);

	void MoveCamera(float deltaTime);

	// Move Camera from Mouse Scroll Input in Arcball Mode
	void MoveArcballCamera(float y_offset, float deltaTime);

	// Rotate Camera from Mouse Movement
	void RotateCamera(float x_offset, float y_offset);

	// Rotate Camera from Mouse Movement, in an Arcball Fashion
	void RotateArcballCamera(float x_offset, float y_offset, float screen_width, float screen_height, float deltaTime);

	void UpdateVelocity(float deltaTime);

private:
	// Update Camera Vectors after Rotation (also after constructing)
	void UpdateCamera();

	// Update Camera Vectors after Arcball Rotation
	void UpdateArcballCamera();
};