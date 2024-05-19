#pragma once

#include "Material.h"
#include "Shader.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include "Model.h"

class Skybox {

private: 
    unsigned int skyboxVAO, 
                 skyboxVBO, 
                 skyboxEBO, 
                 cubemapTexture;

public:

    Skybox::Skybox() {
        float skyboxVertices[] =
        {
            -250.0f, -250.0f,  250.0f,
             250.0f, -250.0f,  250.0f,
             250.0f, -250.0f, -250.0f,
            -250.0f, -250.0f, -250.0f,
            -250.0f,  250.0f,  250.0f,
             250.0f,  250.0f,  250.0f,
             250.0f,  250.0f, -250.0f,
            -250.0f,  250.0f, -250.0f
        };

        unsigned int skyboxIndices[] =
        {
            1, 2, 6,
            6, 5, 1,

            0, 4, 7,
            7, 3, 0,

            4, 5, 6,
            6, 7, 4,

            0, 3, 2,
            2, 1, 0,

            0, 1, 5,
            5, 4, 0,

            3, 7, 6,
            6, 2, 3
        };
        
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glGenBuffers(1, &skyboxEBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        std::string facesCubemap[6] =
        {
            gcgFindTextureFile("assets/geometry/cubemap/right.png"),
            gcgFindTextureFile("assets/geometry/cubemap/left.png"),
            gcgFindTextureFile("assets/geometry/cubemap/top.png"),
            gcgFindTextureFile("assets/geometry/cubemap/bottom.png"),
            gcgFindTextureFile("assets/geometry/cubemap/front.png"),
            gcgFindTextureFile("assets/geometry/cubemap/back.png")
        };

        glGenTextures(1, &cubemapTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        for (unsigned int i = 0; i < 6; i++)
        {
            int width, height, nrChannels;
            unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                stbi_set_flip_vertically_on_load(false);
                glTexImage2D
                (
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0,
                    GL_RGB,
                    width,
                    height,
                    0,
                    GL_RGB,
                    GL_UNSIGNED_BYTE,
                    data
                );
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
                stbi_image_free(data);
            }
        }

    }

    void Skybox::draw() {
        glDepthFunc(GL_LEQUAL);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
    }
};