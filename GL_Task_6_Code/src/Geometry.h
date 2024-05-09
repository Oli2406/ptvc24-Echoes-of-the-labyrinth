/*
 * Copyright 2023 Vienna University of Technology.
 * Institute of Computer Graphics and Algorithms.
 * This file is part of the GCG Lab Framework and must not be redistributed.
 */
#pragma once


#include "Material.h"
#include "Shader.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

/*!
 * Stores all data for a geometry object
 */
struct GeometryData {
    /*!
     * Vertex positions
     */
    std::vector<glm::vec3> positions;
    /*!
     * Geometry indices
     */
    std::vector<unsigned int> indices;

    /*!
     * Vertex colors
     */
    std::vector<glm::vec3> colors;
    /*!
     * Vertex normals
     */
    std::vector<glm::vec3> normals;
    /*!
     * Vertex UV coordinates
     */
    std::vector<glm::vec2> uvs;
};

class Geometry {
  protected:
    /*!
     * Vertex array object
     */
    GLuint vao;
    /*!
     * Vertex buffer object that stores the vertex positions
     */
    GLuint vboPositions;

    /*!
     * Vertex buffer object that stores the vertex color
     */
    GLuint vboColor;

    /*!
     * Vertex buffer object that stores the vertex normals
     */
    GLuint vboNormals;
    /*!
     * Vertex buffer object that stores the vertex UV coordinates
     */
    GLuint vboUVs;
    /*!
     * Vertex buffer object that stores the indices
     */
    GLuint vboIndices;

    /*!
     * Number of elements to be rendered
     */
    unsigned int elements;

    /*!
     * Material of the geometry object
     */
    std::shared_ptr<Material> material;

    /*!
     * Model matrix of the object
     */
    glm::mat4 modelMatrix;

  public:
    /*!
     * Geometry object constructor
     * Creates VAO and VBOs and binds them
     * @param modelMatrix: model matrix of the object
     * @param data: data for the geometry object
     * @param material: material of the geometry object
     */
    Geometry(glm::mat4 modelMatrix, const GeometryData& data, std::shared_ptr<Material> material);
    ~Geometry();

    /*!
     * Draws the object
     * Uses the shader, sets the uniform and issues a draw call
     */
    void draw();

    /*!
     * Transforms the object, i.e. updates the model matrix
     * @param transformation: the transformation matrix to be applied to the object
     */
    void transform(glm::mat4 transformation);

    // updates the modelmatrix
    void Geometry::updateModelMatrix(glm::mat4 newModelMatrix);

    // returns modelmatrix
    glm::mat4 Geometry::getModelMatrix() const;

    /*!
     * Resets the model matrix to the identity matrix
     */
    void resetModelMatrix();

    /*!
     * Creates a cube geometry
     * @param width: width of the cube
     * @param height: height of the cube
     * @param depth: depth of the cube
     * @return all cube data
     */
    static GeometryData createCubeGeometry(float width, float height, float depth);

    /*!
     *	Creates a cornell box geometry with vertex color attribute
     *	@param width		width of the box
     *	@param height		height of the box
     *	@param depth		depth of the box
     *	@return all box data
     */
    static GeometryData createCornellBoxGeometry(float width, float height, float depth);
    /*!
     * Creates a cylinder geometry
     * @param segments: number of segments of the cylinder
     * @param height: height of the cylinder
     * @param radius: radius of the cylinder
     * @return all cylinder data
     */
    static GeometryData createCylinderGeometry(unsigned int segments, float height, float radius);

    /*!
     *	Creates a cylinder geometry along a bezier curve
     *	@param segments		number of segments of the cylinder
     *	@param controlPoints    control points of the bezier curve
     *  @param bezierSegments   number of segments of the bezier curve
     *	@param radius		radius of the cylinder
     *	@return all cylinder data
     */
    static GeometryData createBezierCylinderGeometry(unsigned int segments, std::vector<glm::vec3> controlPoints, unsigned int controlSegments, float radius);
    /*!
     * Creates a sphere geometry
     * @param longitudeSegments: number of longitude segments of the sphere
     * @param latitudeSegments: number of latitude segments of the sphere
     * @param radius: radius of the sphere
     * @return all sphere data
     */
    static GeometryData createSphereGeometry(unsigned int longitudeSegments, unsigned int latitudeSegments, float radius);
};
