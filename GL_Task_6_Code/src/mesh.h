#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"

using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    
    glm::vec3 Position;
   
    glm::vec3 Normal;
   
    glm::vec2 TexCoords;

    glm::vec3 Tangent;
   
    glm::vec3 Bitangent;

    int m_BoneIDs[MAX_BONE_INFLUENCE];
   
    float m_Weights[MAX_BONE_INFLUENCE];

};

struct Text{
    GLuint id;
    string type;
    aiString path;
};

class Mesh
{
public:
    /*  Mesh Data  */
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Text> textures;
    
    /*  Functions  */
    // Constructor
    Mesh( vector<Vertex> vertices, vector<GLuint> indices, vector<Text> textures )
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        
        // Now that we have all the required data, set the vertex buffers and its attribute pointers.
        this->setupMesh( );
    }
    
    // Render the mesh
    void Draw( std::shared_ptr<Shader> shader )
    {
        // Bind appropriate textures
        GLuint diffuseNr = 1;
        GLuint normalNr = 1;
        
        for( GLuint i = 0; i < this->textures.size(); i++ )
        {
            glActiveTexture( GL_TEXTURE0 + i ); // Active proper texture unit before binding
            // Retrieve texture number (the N in diffuse_textureN)
            stringstream ss;
            string number;
            string name = this->textures[i].type;
            
            if( name == "texture_diffuse" )
            {
                ss << diffuseNr++; // Transfer GLuint to stream
            }
            else if( name == "normalMap" )
            {
                ss << normalNr++; // Transfer GLuint to stream
            }
            
            number = ss.str( );
            // Now set the sampler to the correct texture unit
            glUniform1i( glGetUniformLocation( shader->getHandle(), ( name + number ).c_str( ) ), i );
            // And finally bind the texture
            glBindTexture( GL_TEXTURE_2D, this->textures[i].id );
        }
        
        // Also set each mesh's shininess property to a default value (if you want you could extend this to another mesh property and possibly change this value)
        //glUniform1f( glGetUniformLocation( shader->getHandle(), "material.shininess" ), 16.0f );
        
        // Draw mesh
        glBindVertexArray( this->VAO );
        glDrawElements( GL_TRIANGLES, this->indices.size( ), GL_UNSIGNED_INT, 0 );
        glBindVertexArray( 0 );
        
        // Always good practice to set everything back to defaults once configured.
        for ( GLuint i = 0; i < this->textures.size( ); i++ )
        {
            glActiveTexture( GL_TEXTURE0 + i );
            glBindTexture( GL_TEXTURE_2D, 0 );
        }
    }
    
private:
    /*  Render data  */
    GLuint VAO, VBO, EBO;
    
    /*  Functions    */
    // Initializes all the buffer objects/arrays
    void setupMesh( )
    {
        // Create buffers/arrays
        glGenVertexArrays( 1, &this->VAO );
        glGenBuffers( 1, &this->VBO );
        glGenBuffers( 1, &this->EBO );
        
        glBindVertexArray( this->VAO );
        // Load data into vertex buffers
        glBindBuffer( GL_ARRAY_BUFFER, this->VBO );
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData( GL_ARRAY_BUFFER, this->vertices.size( ) * sizeof( Vertex ), &this->vertices[0], GL_STATIC_DRAW );
        
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, this->EBO );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, this->indices.size( ) * sizeof( GLuint ), &this->indices[0], GL_STATIC_DRAW );
        
        // Set the vertex attribute pointers
        // Vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

        // weights
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, m_Weights));
    }
};