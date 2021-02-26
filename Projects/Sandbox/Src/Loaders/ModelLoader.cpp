#include "PreCompiled.h"
#include "ModelLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace RS;

bool ModelLoader::Load(const std::string& filePath, ModelResource*& outModel)
{
    std::string path = std::string(RS_MODEL_PATH) + filePath;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = std::string(RS_MODEL_PATH); // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path, reader_config))
    {
        if (!reader.Error().empty())
            LOG_ERROR("TinyObjReader: %s", reader.Error());
        else
            LOG_ERROR("TinyObjReader: Failed to parse file!");
        return false;
    }

    if (!reader.Warning().empty())
        LOG_WARNING("TinyObjReader: %s", reader.Warning());

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    MeshResource& mesh = outModel->Mesh;

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++)
    {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            int fv = shapes[s].mesh.num_face_vertices[f];

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++)
            {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[(size_t)3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[(size_t)3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[(size_t)3 * idx.vertex_index + 2];
                tinyobj::real_t nx = attrib.normals[(size_t)3 * idx.normal_index + 0];
                tinyobj::real_t ny = attrib.normals[(size_t)3 * idx.normal_index + 1];
                tinyobj::real_t nz = attrib.normals[(size_t)3 * idx.normal_index + 2];
                tinyobj::real_t tx = attrib.texcoords[(size_t)2 * idx.texcoord_index + 0];
                tinyobj::real_t ty = attrib.texcoords[(size_t)2 * idx.texcoord_index + 1];

                // Naive implementation.
                MeshResource::Vertex vertex = {};
                vertex.Position     = glm::vec3(vx, vy, vz);
                vertex.Normal       = glm::vec3(nx, ny, nz);
                vertex.Tangent      = glm::vec3(0.f);
                vertex.Bitangent    = glm::vec3(0.f);
                vertex.UV           = glm::vec2(tx, ty);
                mesh.Indices.push_back(mesh.Vertices.size());
                mesh.Vertices.push_back(vertex);
            }
            index_offset += fv;

            // per-face material
            //shapes[s].mesh.material_ids[f];
        }
    }

    return true;
}

bool ModelLoader::LoadWithAssimp(const std::string& filePath, ModelResource*& outModel)
{
    std::string path = std::string(RS_MODEL_PATH) + filePath;

    bool useCW      = true;
    bool useLH      = false;
    bool useUVTL    = false;
    Assimp::Importer importer;

    uint32 flags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
    if (useCW)      flags |= aiProcess_FlipWindingOrder;
    if (useLH)      flags |= aiProcess_MakeLeftHanded;
    if (useUVTL)    flags |= aiProcess_FlipUVs;
    const aiScene* pScene = importer.ReadFile(path.c_str(), flags);

    if (!pScene)
    {
        LOG_ERROR("Assimp Errror: %s", importer.GetErrorString());
        return false;
    }

    return RecursiveLoad(pScene, outModel);
}

bool ModelLoader::RecursiveLoad(const aiScene*& pScene, ModelResource*& outModel)
{
    return true;
}
