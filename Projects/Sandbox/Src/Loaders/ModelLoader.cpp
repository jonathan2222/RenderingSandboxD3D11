#include "PreCompiled.h"
#include "ModelLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

using namespace RS;

Model* ModelLoader::Load(const std::string& filePath)
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
        return nullptr;
    }

    if (!reader.Warning().empty())
        LOG_WARNING("TinyObjReader: %s", reader.Warning());

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    Model* pModel = new Model();
    
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
                Model::Vertex vertex = {};
                vertex.Position = glm::vec3(vx, vy, vz);
                vertex.Normal = glm::vec3(nx, ny, nz);
                vertex.UV = glm::vec2(tx, ty);
                pModel->Indices.push_back(pModel->Vertices.size());
                pModel->Vertices.push_back(vertex);
            }
            index_offset += fv;

            // per-face material
            //shapes[s].mesh.material_ids[f];
        }
    }

    return pModel;
}
