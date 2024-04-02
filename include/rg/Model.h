#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <rg/Mesh.h>
#include <rg/Shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

unsigned int TextureFromFile (const char* path, const std::string &directory);

class Model {
public:

    //podaci modela
    std::vector<Texture> m_textures_loaded;
    std::vector<Mesh> m_meshes;
    std::string m_directory;

    //konstruktor
    Model (std::string const &path) {
        loadModel(path);
    }

    void Draw(Shader &shader) {
        for (unsigned int i = 0; i < m_meshes.size(); i++) {
            m_meshes[i].Draw(shader);
        }
    }

    void SetShaderTextureNamePrefix(std::string prefix) {
        for (Mesh& mesh : m_meshes) {
            mesh.m_glslIdentifierPrefix = prefix;
        }
    }

private:

    //ucitavanje modela sa podrzanom ekstenzijom fajla i cuvanje mesh-ova u vektor
    void loadModel (std::string const &path) {

        //citanje fajla preko ASSIMP-a
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,  aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        //provera da li je doslo do greske
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << "\n";
            return;
        }

        //dobijanje putanje direktorijuma
        m_directory = path.substr(0, path.find_last_of('/'));

        //obrada ASSIMP-ovih cvorova rekurzivno
        processNode(scene->mRootNode, scene);

    }

    //obrada cvorova rekuzivno
    void processNode (aiNode* node, const aiScene* scene) {

        //obrada svakog mesh-a u trenutnom cvoru
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_meshes.push_back(processMesh(mesh, scene));
        }

        //obrada svih dete-cvorova rekurzivno
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh (aiMesh* mesh, const aiScene* scene) {

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        //prolazenje svih vertexa mesh-a
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {

            Vertex vertex;
            glm::vec3 vector;

            //koordinate
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.m_position = vector;

            //normale
            if (mesh->HasNormals()) {

                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.m_normal = vector;

            }

            //koordinate teksture
            if (mesh->mTextureCoords[0]) {

                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.m_texture_coordinates = vec;

                //tangenta
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.m_tangent = vector;

                // bitangenta
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.m_bitangent = vector;

            } else
                vertex.m_texture_coordinates = glm::vec2(0.0f);

            vertices.push_back(vertex);
        }

        //prolazenje kroz sve face-ove mesh-a
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        //obrada materijala
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiColor3D color(0.0f, 0.0f, 0.0f);
        material->Get(AI_MATKEY_COLOR_AMBIENT, color);

        // 1. difuzna mapa
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. spekularna mapa
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normalna mapa
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height mapa
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        return Mesh(vertices, indices, textures);
    }

    //preoverava sve teksture datog tipa i ucitava teksturu ako nije vec ucitana
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string type_name) {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool skip = false;
            for (unsigned int j = 0; j < m_textures_loaded.size(); j++) {
                if (std::strcmp(m_textures_loaded[j].m_path.data(), str.C_Str()) == 0) {
                    textures.push_back(m_textures_loaded[j]);
                    skip = true;
                    break;
                }
            }

            if (!skip) {
                Texture texture;
                texture.m_id = TextureFromFile(str.C_Str(), this->m_directory);
                texture.m_type = type_name;
                texture.m_path = str.C_Str();
                textures.push_back(texture);
                m_textures_loaded.push_back(texture);
            }
        }

        return textures;

    }

};

unsigned int TextureFromFile(const char* path, const std::string& directory)
{
    std::cout << std::string(path) << "\n";
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
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


#endif