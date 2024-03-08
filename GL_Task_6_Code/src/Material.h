/*
 * Copyright 2023 Vienna University of Technology.
 * Institute of Computer Graphics and Algorithms.
 * This file is part of the GCG Lab Framework and must not be redistributed.
 */
#pragma once

#include "Shader.h"
#include <glm/glm.hpp>
#include <memory>
#include "Texture.h"


/*!
 * Base material
 */
class Material {
  protected:
    /*!
     * The shader used for rendering this material
     */
    std::shared_ptr<Shader> _shader;
    /*!
     * The material's color
     */
    glm::vec3 _color;

    /*!
     * The material's coefficients (x = ambient, y = diffuse, z = specular)
     */
    glm::vec3 _materialCoefficients;

    /*!
     * Alpha value, i.e. the shininess constant
     */
    float _alpha;

  public:
    /*!
     * Base material constructor
     * @param shader: The shader used for rendering this material
     * @param color: The material's color
     * @param materialCoefficients: The material's coefficients (x = ambient, y = diffuse, z = specular)
     * @param alpha: Alpha value, i.e. the shininess constant
     */
    Material(std::shared_ptr<Shader> shader, glm::vec3 color, glm::vec3 materialCoefficients, float alpha);
    /*!
     * Base material constructor
     * @param shader: The shader used for rendering this material
     * @param materialCoefficients: The material's coefficients (x = ambient, y = diffuse, z = specular)
     * @param alpha: Alpha value, i.e. the shininess constant
     */
    Material(std::shared_ptr<Shader> shader, glm::vec3 materialCoefficients, float alpha);

    virtual ~Material();

    /*!
     * @return The shader associated with this material
     */
    Shader* getShader();

    /*!
     * Sets this material's parameters as uniforms in the shader
     */
    virtual void setUniforms();
};


/*!
 * Texture material
 */
class TextureMaterial : public Material {
  protected:
    /*!
     * The diffuse texture of this material
     */
    std::shared_ptr<Texture> _diffuseTexture;

  public:
    /*!
     * Texture material constructor
     * @param shader: The shader used for rendering this material
     * @param materialCoefficients: The material's coefficients (x = ambient, y = diffuse, z = specular)
     * @param alpha: Alpha value, i.e. the shininess constant
     * @param diffuseTexture: The diffuse texture of this material
     */
    TextureMaterial(std::shared_ptr<Shader> shader, glm::vec3 materialCoefficients, float alpha, std::shared_ptr<Texture> diffuseTexture);

    virtual ~TextureMaterial();

    /*!
     * Set's this material's parameters as uniforms in the shader
     */
    virtual void setUniforms();
};

