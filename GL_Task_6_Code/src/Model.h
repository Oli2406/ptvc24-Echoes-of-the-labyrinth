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
#include <cstring>
#include "animData.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define COLLISION_GROUP_DYNAMIC 1
#define COLLISION_GROUP_STATIC 2
#define COLLISION_FLAG_DYNAMIC (1<<0)
#define COLLISION_FLAG_STATIC (1<<1)

#include "globals.h"


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

    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }

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
    std::map<string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;

public:
    
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
            desc.height = 1.8f;
            desc.radius = 0.5f;
            desc.material = material;

            PxControllerManager* controllerManager = PxCreateControllerManager(*scene);
            this->controller = controllerManager->createController(desc);

            if (this->controller) {
                this->controller->setPosition(PxExtendedVec3(5, 2, 5));
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


    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const& path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    void SetVertexBoneDataToDefault(Vertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }


    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Text> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            SetVertexBoneDataToDefault(vertex);
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        vector<Text> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        ExtractBoneWeightForVertices(vertices, mesh, scene);

        return Mesh(vertices, indices, textures);
    }

    void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if (vertex.m_BoneIDs[i] < 0)
            {
                vertex.m_Weights[i] = weight;
                vertex.m_BoneIDs[i] = boneID;
                break;
            }
        }
    }


    void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
    {
        auto& boneInfoMap = m_BoneInfoMap;
        int& boneCount = m_BoneCounter;

        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = boneCount;
                newBoneInfo.offset = ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                boneInfoMap[boneName] = newBoneInfo;
                boneID = boneCount;
                boneCount++;
            }
            else
            {
                boneID = boneInfoMap[boneName].id;
            }
            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId <= vertices.size());
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }


    unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false)
    {
        string filename = string(path);
        filename = directory + '/' + filename;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }

    static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
    {
        glm::mat4 to;
        //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }

    vector<Text> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Text> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                textures.push_back(textures_loaded[j]);
            }
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                Text texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
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
    glTexImage2D(GL_TEXTURE_2D, 0, gammaEnabled ? GL_SRGB : GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
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
