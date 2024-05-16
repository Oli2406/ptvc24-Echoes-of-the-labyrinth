#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <memory>

#include "Shader.h"
#include "Mesh.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "PxPhysicsAPI.h"
#include "cooking/PxCooking.h"
#include "characterkinematic/PxControllerManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define COLLISION_GROUP_DYNAMIC 1
#define COLLISION_GROUP_STATIC 2
#define COLLISION_FLAG_DYNAMIC (1<<0)
#define COLLISION_FLAG_STATIC (1<<1)


using namespace physx;

GLint TextureFromFile(const char* path, string directory);

class Model
{
public:

    // Constructor, expects a filepath to a 3D model.
    Model(GLchar* path)
    {
        this->loadModel(path);
    }

    Model(GLchar* path, PxPhysics* physics, PxScene* scene, bool isDynamic) {
        this->loadModel(path);
        this->initPhysics(physics, scene, isDynamic);
    }

    Model() {

    }

    // Draws the model, and thus all its meshes
    void Draw(std::shared_ptr<Shader> shader)
    {
        for (GLuint i = 0; i < this->meshes.size(); i++)
        {
            this->meshes[i].Draw(shader);
        }
    }

private:

    vector<Mesh> meshes;
    string directory;
    vector<Text> textures_loaded;	// Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    PxPhysics* physics;
    PxScene* scene;
    vector<PxRigidStatic*> physxActors;
    PxRigidStatic* actor;
    PxController* controller;
    PxConvexMeshCookingResult* convexMesh;
    float scale;

public:

    // Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string path)
    {
        // Read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        // Check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // Retrieve the directory path of the filepath
        this->directory = path.substr(0, path.find_last_of('/'));

        // Process ASSIMP's root node recursively
        this->processNode(scene->mRootNode, scene);
    }

    void initPhysics(PxPhysics* physics, PxScene* scene, bool isDynamic) {
        this->physics = physics;
        this->scene = scene;

        PxMaterial* material = physics->createMaterial(0.5f, 0.5f, 0.1f);
        if (!material) {
            std::cerr << "Failed to create material" << std::endl;
            return;
        }
        if (!isDynamic) {
            for (GLuint i = 0; i < this->meshes.size(); i++) {
                Mesh& mesh = this->meshes[i];

                PxTriangleMesh* triangleMesh = createTriangle(this->meshes[i]);
                if (!triangleMesh) {
                    std::cerr << "Failed to create triangle mesh for mesh: " << i << std::endl;
                    continue;
                }

                PxTriangleMeshGeometry geom(triangleMesh);
                PxTransform transform(PxVec3(0, 0, 0));
                PxRigidStatic* staticActor = createStatic(transform, geom, physics, material, scene);

                this->physxActors.push_back(staticActor);
                this->actor = staticActor;
            }
        }

        if (isDynamic) {
            PxCapsuleControllerDesc desc;
            desc.height = 0.5f;
            desc.radius = 0.1f;
            desc.position = PxExtendedVec3(0, 5, 0);
            desc.upDirection = PxVec3(0, 1, 0);
            desc.material = material;

            PxControllerManager* controllerManager = PxCreateControllerManager(*scene);
            this->controller = controllerManager->createController(desc);

            if (this->controller) {
                this->controller->setPosition(PxExtendedVec3(2, 2, 2));
            }
            else {
                std::cerr << "Failed to create character controller" << std::endl;
            }
        }
    }

    PxRigidStatic* getPlayerModel() {
        return this->actor;
    }

    PxController* getController() {
        return this->controller;
    }


private:

    PxRigidDynamic* createDynamic(const PxTransform& t, const PxGeometry& geometry, PxPhysics* physics, PxMaterial* material, PxScene* scene, const PxVec3& velocity = PxVec3(0)) {
        PxRigidDynamic* dynamic = PxCreateDynamic(*physics, t, geometry, *material, 10.0f);
        dynamic->setAngularDamping(0.5f);
        dynamic->setLinearVelocity(velocity);
        scene->addActor(*dynamic);
        return dynamic;
    }

    PxRigidStatic* createStatic(const PxTransform& t, const PxGeometry& geometry, PxPhysics* physics, PxMaterial* material, PxScene* scene) {
        PxRigidStatic* staticActor = PxCreateStatic(*physics, t, geometry, *material);
        scene->addActor(*staticActor);
        return staticActor;
    }

    PxTriangleMesh* createTriangle(const Mesh& mesh)
    {
        // Extract vertices and indices from Mesh
        vector<Vertex> vertices = mesh.vertices;
        vector<GLuint> indices = mesh.indices;

        PxVec3* pxVertices = new PxVec3[vertices.size()];
        for (size_t i = 0; i < vertices.size(); i++)
        {
            pxVertices[i] = PxVec3(vertices[i].Position.x, vertices[i].Position.y, vertices[i].Position.z);
        }

        PxTriangleMeshDesc meshDesc;
        PxTriangleMeshCookingResult::Enum result;
        meshDesc.points.count = vertices.size();
        meshDesc.points.stride = sizeof(PxVec3);
        meshDesc.points.data = pxVertices;

        meshDesc.triangles.count = indices.size() / 3;
        meshDesc.triangles.stride = 3 * sizeof(GLuint);
        meshDesc.triangles.data = indices.data();

        const PxCookingParams cookingParams(physics->getTolerancesScale());

        PxTriangleMesh* triangleMesh = PxCreateTriangleMesh(cookingParams, meshDesc);

        if (triangleMesh) {
            cout << "Triangle mesh created successfully" << endl;
        }
        else {
            cout << "Failed to create triangle mesh" << endl;
        }

        delete[] pxVertices;

        return triangleMesh;
    }


    // Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene)
    {
        // Process each mesh located at the current node
        for (GLuint i = 0; i < node->mNumMeshes; i++)
        {
            // The node object only contains indices to index the actual objects in the scene.
            // The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

            this->meshes.push_back(this->processMesh(mesh, scene));
        }

        // After we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (GLuint i = 0; i < node->mNumChildren; i++)
        {
            this->processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // Data to fill
        vector<Vertex> vertices;
        vector<GLuint> indices;
        vector<Text> textures;

        // Walk through each of the mesh's vertices
        for (GLuint i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.

            // Positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // Normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;


            // Texture Coordinates
            if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
                cout << "No textures were loaded." << endl;
            }

            vertices.push_back(vertex);
        }

        // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (GLuint i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // Retrieve all indices of the face and store them in the indices vector
            for (GLuint j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        // Process materials
        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            // We assume a convention for sampler names in the shaders. Each diffuse texture should be named
            // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
            // Same applies to other texture as the following list summarizes:
            // Diffuse: texture_diffuseN
            // Specular: texture_specularN
            // Normal: texture_normalN

            // 1. Diffuse maps
            vector<Text> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            // 2. Specular maps
            vector<Text> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        // Return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }



    // Checks all material textures of a given type and loads the textures if they're not loaded yet.
    // The required info is returned as a Texture struct.
    vector<Text> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Text> textures;

        for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            // Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            GLboolean skip = false;

            for (GLuint j = 0; j < textures_loaded.size(); j++)
            {
                if (textures_loaded[j].path == str)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)

                    break;
                }
            }

            if (!skip)
            {   // If texture hasn't been loaded already, load it
                Text texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str;
                textures.push_back(texture);

                this->textures_loaded.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }

        return textures;
    }
};

GLint TextureFromFile(const char* path, string directory)
{
    //Generiere Textur-ID und lade Texturdaten
    string filename = string(path);
    filename = directory + '/' + filename;
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height;
    int nrChannels = 3;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return -1;
    }

    // Textur an ID zuweisen
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Parameter setzen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    return textureID;
}
