/*
 * Copyright 2023 Vienna University of Technology.
 * Institute of Computer Graphics and Algorithms.
 * This file is part of the GCG Lab Framework and must not be redistributed.
 */
#pragma once


#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

/*!
 * Arc ball camera, modified by mouse input
 */
class Camera {
  protected:
    glm::mat4 _viewMatrix;
    glm::mat4 _projMatrix;
    int _mouseX, _mouseY;
    float _yaw, _pitch;
    float ggg, ooo;
    glm::vec3 _position;
    glm::vec3 _strafe;
    glm::vec3 ttt;
    glm::vec3 tt;

  public:
    /*!
     * Camera constructor
     * @param fov: field of view, in degrees
     * @param aspect: aspect ratio
     * @param near_plane_distance: near_plane_distance plane
     * @param far_plane_distance: far_plane_distance plane
     */
    Camera(float fov, float aspect, float near, float far);

    ~Camera();

    /*!
     * @param yaw the new value for _yaw
     */
    void setYaw(float yaw);

    /*!
     * @param pitch the new value for _pitch
     */
    void setPitch(float pitch);

    /*!
     * @return the current position of the camera
     */
    glm::vec3 getPosition() const;

    /*!
     * @return the view-projection matrix
     */
    glm::mat4 getViewProjectionMatrix() const;

    /*!
     * Updates the camera's position and view matrix according to the input
     * @param x: current mouse x position
     * @param y: current mouse y position
     * @param zoom: zoom multiplier
     * @param dragging: is the camera dragging
     * @param strafing: is the camera strafing
     */
    void update(int x, int y, float zoom, bool dragging, bool strafing);

    /*!
     *Top secret
     */
    void updateQuaternionBasedObfuscated(int x, int y, float zoom, bool dragging, bool strafing);
};
