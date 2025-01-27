#pragma once

#include "Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 start_position, float start_yaw, float start_pitch, glm::vec3 start_up) : front(FRONT), movement_speed(MAX_SPEED), look_sensitivity(SENSITIVITY), fov(FOV)
{
	position = start_position;
	yaw = start_yaw;
	pitch = start_pitch;
	world_up = up;
	enabled = true;
	arcball_mode = false;
	velocity = glm::vec3(0.0f);

	UpdateCamera();
}

Camera::Camera(glm::vec3 start_position) : front(FRONT), movement_speed(MAX_SPEED), look_sensitivity(SENSITIVITY), fov(FOV), arcball_sensitivity(ARCBALL_SENSITIVITY), arcball_move_sensitivity(ARCBALL_MOVE_SENSITIVITY)
{
	position = start_position;
	yaw = YAW;
	pitch = PITCH;
	world_up = UP;
	enabled = true;
	arcball_mode = false;
	velocity = glm::vec3(0.0f);

	UpdateCamera();
}

Camera::~Camera()
{
	// TODO: Shoot Cameraman!
}

void Camera::MoveCamera(Movement_Direction direction, float deltaTime)
{
	// If camera is disabled, ignore
	if (!enabled)
	{
		velocity = glm::vec3(0.0f);
		return;
	}

	//float smooth_speed = movement_speed * deltaTime;

	/*if (direction == FWD)
		position += front * smooth_speed;
	else if (direction == AFT)
		position -= front * smooth_speed;
	else if (direction == LEFT)
		position -= right * smooth_speed;
	else if (direction == RIGHT)
		position += right * smooth_speed;
	else if (direction == UPWARD)
		position += world_up * smooth_speed;
	else if (direction == DOWNWARD)
		position -= world_up * smooth_speed;*/

	if (direction == FWD)
		velocity += front * movement_speed * deltaTime;
	else if (direction == AFT)
		velocity -= front * movement_speed * deltaTime;
	else if (direction == LEFT)
		velocity -= right * movement_speed * deltaTime;
	else if (direction == RIGHT)
		velocity += right * movement_speed * deltaTime;
	else if (direction == UPWARD)
		velocity += world_up * movement_speed * deltaTime;
	else if (direction == DOWNWARD)
		velocity -= world_up * movement_speed * deltaTime;
}

void Camera::MoveCamera(float deltaTime)
{
	// If camera is disabled, ignore
	if (!enabled)
	{
		velocity = glm::vec3(0.0f);
		return;
	}

	position += velocity * deltaTime;
}

void Camera::UpdateVelocity(float deltaTime)
{
	if (velocity.x == 0.0f && velocity.y == 0.0f && velocity.z == 0.0f)
		return;

	glm::vec3 dir = glm::normalize(velocity);
	float magnitude = glm::length(velocity);

	magnitude = std::clamp(magnitude - DRAG * deltaTime, 0.0f, MAX_SPEED);

	velocity = dir * magnitude;
}

void Camera::MoveArcballCamera(float y_offset, float deltaTime)
{
	// If camera is disabled, ignore
	if (!enabled)
		return;

	position += front * arcball_move_sensitivity * y_offset;
}

void Camera::RotateCamera(float x_offset, float y_offset)
{
	// If camera is disabled, ignore
	if (!enabled)
		return;

	// Update Pitch and Yaw
	yaw += x_offset * look_sensitivity;
	pitch += y_offset * look_sensitivity;

	// Constraint Pitch
	if (pitch > PITCH_CONSTRAINT)
		pitch = PITCH_CONSTRAINT;
	else if (pitch < -PITCH_CONSTRAINT)
		pitch = -PITCH_CONSTRAINT;

	// Update Camera Vectors after Rotation
	UpdateCamera();
}

void Camera::RotateArcballCamera(float x_offset, float y_offset, float screen_width, float screen_height, float deltaTime)
{
	// If camera is disabled, ignore
	if (!enabled)
		return;

	front = glm::normalize(glm::vec3(0.0f) - position);

	glm::vec4 homogen_position = glm::vec4(position, 1.0f);
	glm::vec4 pivot_point = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	float x_angle = x_offset * deltaTime * arcball_sensitivity * (2 * glm::pi<float>() / screen_width);
	float y_angle = y_offset * deltaTime * arcball_sensitivity * (glm::pi<float>() / screen_height);

	//float cos_angle = glm::dot(front, up);

	glm::mat4 rotation_mat_x(1.0f);
	rotation_mat_x = glm::rotate(rotation_mat_x, x_angle, up);
	homogen_position = (rotation_mat_x * (homogen_position - pivot_point)) + pivot_point;

	glm::mat4 rotation_mat_y(1.0f);
	rotation_mat_y = glm::rotate(rotation_mat_y, y_angle, right);

	position = (rotation_mat_y * (homogen_position - pivot_point)) + pivot_point;

	// Update Camera Vectors after Rotation
	UpdateArcballCamera();
}

glm::mat4 Camera::GetCurrentViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::GetCurrentProjectionMatrix(float screen_width, float screen_height)
{
	return glm::perspective(glm::radians(fov), screen_width / screen_height, Z_NEAR, Z_FAR);
}

void Camera::UpdateCamera()
{
	// Update Front Vector
	glm::vec3 new_front = glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
	front = glm::normalize(new_front);

	// Update Right and Up Vectors
	right = glm::normalize(glm::cross(front, world_up));
	up = glm::normalize(glm::cross(right, front));
}

void Camera::UpdateArcballCamera()
{
	// Update Right and Up Vectors
	right = glm::normalize(glm::cross(front, world_up));
	up = glm::normalize(glm::cross(right, front));
}