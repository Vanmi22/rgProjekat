#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

//definisemo nekoliko mogucih opcija za pomeranje kamere
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

//pocetne vrednosti parametara kamere
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.5f;
const float ZOOM = 45.0f;

//klasa Cmarea koja obradjuje ulaze
class Camera {
public:
    //atributi kamere
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_world_up;
    //Ojlerovi uglovi
    float m_yaw;
    float m_pitch;
    //opcije kamere
    float m_movement_speed;
    float m_mouse_sensitivity;
    float m_zoom;

    //konstruktor
    Camera (glm::vec3 position = glm::vec3(0.0f),
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
            float yaw = YAW,
            float pitch = PITCH)
            : m_front(glm::vec3(0.0f, 0.0f, -1.0f)),
            m_movement_speed(SPEED),
            m_mouse_sensitivity(SENSITIVITY),
            m_zoom(ZOOM) {

        m_position = position;
        m_world_up = up;
        m_yaw = yaw;
        m_pitch = pitch;
        updateCameraVectors();

    }

    //vraca view-matricu izracunau preko Ojlerovih uglova
    glm::mat4 GetViewMatrix () {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    //obradjuje ulaze sa tastature
    void ProcessKeyboard (Camera_Movement direction, float deltaTime) {

        float velocity = m_movement_speed * deltaTime;
        if (direction == FORWARD)
            m_position += m_front * velocity;
        if (direction == BACKWARD)
            m_position -= m_front * velocity;
        if (direction == LEFT)
            m_position -= m_right * velocity;
        if (direction == RIGHT)
            m_position += m_right * velocity;

    }

    //obradjuje ulaze sa senzora misa
    void ProcessMouseMovement (float xoffset, float yoffset, GLboolean constrain_pitch = true) {

        xoffset *= m_mouse_sensitivity;
        yoffset *= m_mouse_sensitivity;

        m_yaw += xoffset;
        m_pitch += yoffset;

        //ogranicimo pitch da se ekran ne bi obrnuo
        if (constrain_pitch) {
            if (m_pitch > 89.0f)
                m_pitch = 89.0f;
            if (m_pitch < -89.0f)
                m_pitch = -89.0f;
        }

        updateCameraVectors();

    }

    //obradjuje ulaze sa tockica misa
    void ProcessMouseScroll (float yoffset) {

        m_zoom -= (float)yoffset;
        if (m_zoom < 1.0f)
            m_zoom = 1.0f;
        if (m_zoom > 45.0f)
            m_zoom = 45.0f;
    }

private:

    //racuna m_front od (novih) Ojlerovih uglova
    void updateCameraVectors () {

        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);
        m_right = glm::normalize(glm::cross(m_front, m_world_up));
        m_up = glm::normalize(glm::cross(m_right, m_front));

    }
};
#endif