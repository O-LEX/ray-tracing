#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

// 定方向の列挙体
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// カメラのデフォルト設定
const float YAW        = -90.0f;
const float PITCH      =  0.0f;
const float SPEED      =  2.5f;
const float SENSITIVITY=  0.1f;
const float ZOOM       =  45.0f;

class Camera {
public:
    // カメラ属性
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // オイラー角度
    float Yaw;
    float Pitch;

    // カメラオプション
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // コンストラクタ
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    // ビューマトリックスを返す
    glm::mat4 GetViewMatrix();

    // キーボード入力を処理
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    // マウス入力を処理
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    // マウスホイール入力を処理
    void ProcessMouseScroll(float yoffset);

private:
    // カメラのベクトルを更新
    void updateCameraVectors();
};

#endif