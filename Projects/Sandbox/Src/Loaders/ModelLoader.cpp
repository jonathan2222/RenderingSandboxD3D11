#include "PreCompiled.h"
#include "ModelLoader.h"

#include <algorithm>

#include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

#pragma warning( push )
#pragma warning( disable : 6011 )
#pragma warning( disable : 6262 )
#pragma warning( disable : 6308 )
#pragma warning( disable : 6387 )
#pragma warning( disable : 26451 )
#pragma warning( disable : 28182 )
#include <stb_image.h>
#pragma warning( pop )

using namespace RS;

bool ModelLoader::Load(const std::string& filePath, ModelResource*& outModel, ModelLoadDesc::LoaderFlags flags)
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
    //auto& materials = reader.GetMaterials();

    // TODO: Add materials and make one model for each shape!
    outModel->Meshes.resize(1);
    MeshObject& mesh = outModel->Meshes[0];

    if (flags & ModelLoadDesc::LoaderFlag::LOADER_FLAG_GENERATE_BOUNDING_BOX)
    {
        mesh.BoundingBox.min = glm::vec3(FLT_MAX);
        mesh.BoundingBox.max = glm::vec3(FLT_MIN);
    }

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++)
    {
        outModel->Name += (s != 0 ? "_" : "") + shapes[s].name;

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
                MeshObject::Vertex vertex = {};
                vertex.Position     = glm::vec3(vx, vy, vz);
                vertex.Normal       = glm::vec3(nx, ny, nz);
                vertex.Tangent      = glm::vec3(0.f);
                vertex.Bitangent    = glm::vec3(0.f);
                vertex.UV           = glm::vec2(tx, ty);
                mesh.Indices.push_back((uint32)mesh.Vertices.size());
                mesh.Vertices.push_back(vertex);

                if (flags & ModelLoadDesc::LoaderFlag::LOADER_FLAG_GENERATE_BOUNDING_BOX)
                {
                    mesh.BoundingBox.min = Maths::GetMinElements(mesh.BoundingBox.min, vertex.Position);
                    mesh.BoundingBox.max = Maths::GetMaxElements(mesh.BoundingBox.max, vertex.Position);
                }
            }
            index_offset += fv;

            // per-face material
            //shapes[s].mesh.material_ids[f];
        }
    }

    // Add a default material
    {
        auto pResourceManager = ResourceManager::Get();
        
        std::string key = path + "TINYOBJ_Material_DEFAULT";
        ResourceID materialID = pResourceManager->GetIDFromString(key);
        if (materialID == 0)
        {
            // Add a new material if it does not exist!
            auto [pMaterialResource, newMaterialID] = pResourceManager->AddResource<MaterialResource>(Resource::Type::MATERIAL);
            pResourceManager->AddStringToIDAssociation(key, newMaterialID);
            materialID = newMaterialID;

            pMaterialResource->Name = "Default Material";

            pResourceManager->LoadTextureResource(pResourceManager->DefaultTextureOnePixelWhite);
            pResourceManager->LoadTextureResource(pResourceManager->DefaultTextureOnePixelBlack);
            pResourceManager->LoadTextureResource(pResourceManager->DefaultTextureOnePixelNormal);
            pMaterialResource->AlbedoTextureHandler             = pResourceManager->DefaultTextureOnePixelWhite;
            pMaterialResource->NormalTextureHandler             = pResourceManager->DefaultTextureOnePixelNormal;
            pMaterialResource->AOTextureHandler                 = pResourceManager->DefaultTextureOnePixelWhite;
            pMaterialResource->MetallicTextureHandler           = pResourceManager->DefaultTextureOnePixelBlack;
            pMaterialResource->RoughnessTextureHandler          = pResourceManager->DefaultTextureOnePixelWhite;
            pMaterialResource->MetallicRoughnessTextureHandler  = pResourceManager->DefaultTextureOnePixelBlack;
        }

        mesh.MaterialHandler = materialID;
    }

    if (flags & ModelLoadDesc::LoaderFlag::LOADER_FLAG_GENERATE_BOUNDING_BOX)
    {
        outModel->BoundingBox.min = mesh.BoundingBox.min;
        outModel->BoundingBox.max = mesh.BoundingBox.max;
    }

    if (flags & ModelLoadDesc::LoaderFlag::LOADER_FLAG_WINDING_ORDER_CW)
    {
        std::reverse(mesh.Indices.begin(), mesh.Indices.end());
    }

    if (flags & ModelLoadDesc::LoaderFlag::LOADER_FLAG_NO_MESH_DATA_IN_RAM)
    {
        mesh.Vertices.clear();
        mesh.Indices.clear();
    }

    if (flags & ModelLoadDesc::LoaderFlag::LOADER_FLAG_UPLOAD_MESH_DATA_TO_GUP)
    {
        LOG_ERROR("LOADER_FLAG_UPLOAD_MESH_DATA_TO_GUP is not implemented for Loader:TINYOBJ!");
        return false;
    }

    if (flags & ModelLoadDesc::LoaderFlag::LOADER_FLAG_USE_UV_TOP_LEFT)
    {
        LOG_WARNING("LOADER_FLAG_USE_UV_TOP_LEFT is not implemented for Loader:TINYOBJ!");
        return false;
    }

    return true;
}

bool ModelLoader::LoadWithAssimp(const std::string& filePath, ModelResource* outModel, ModelLoadDesc::LoaderFlags flags)
{
    std::string path = std::string(RS_MODEL_PATH) + filePath;

    static const bool s_UseLH = false;
    Assimp::Importer importer;

    // Remove the line and point primitives. This ensure the mesh always contains only triangles together with the aiProcess_Triangulate flag.
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

    int assimpFlags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
    if (flags & ModelLoadDesc::LoaderFlag::LOADER_FLAG_WINDING_ORDER_CW)
        assimpFlags |= aiProcess_FlipWindingOrder;
    if (s_UseLH)
        assimpFlags |= aiProcess_MakeLeftHanded;
    if (flags & ModelLoadDesc::LoaderFlag::LOADER_FLAG_USE_UV_TOP_LEFT)
        assimpFlags |= aiProcess_FlipUVs;
    if (flags & ModelLoadDesc::LoaderFlag::LOADER_FLAG_GENERATE_BOUNDING_BOX)
        assimpFlags |= aiProcess_GenBoundingBoxes;
    const aiScene* pScene = importer.ReadFile(path.c_str(), assimpFlags);

    if (!pScene)
    {
        LOG_ERROR("Assimp Errror: %s", importer.GetErrorString());
        return false;
    }

    return RecursiveLoadMeshes(pScene, pScene->mRootNode, outModel, glm::mat4(1.f), flags, path);
}

bool ModelLoader::RecursiveLoadMeshes(const aiScene*& pScene, aiNode* pNode, ModelResource* pParent, glm::mat4 accTransform, ModelLoadDesc::LoaderFlags flags, const std::string& modelPath)
{
    bool succeeded = true;
    ModelResource* pTargetParent = nullptr;

    glm::mat4 transform = glm::make_mat4x4(&pNode->mTransformation.a1);
    transform = glm::transpose(transform);

    if (pParent->pParent == nullptr)
    {
        pParent->BoundingBox.min = glm::vec3(FLT_MAX);
        pParent->BoundingBox.max = glm::vec3(FLT_MIN);
    }

    // Only add models which have meshes.
    if (pNode->mNumMeshes > 0)
    {
        // Create an AABB that encompasses all of the messhes' individual AABBs.
        AABB modelAABB;
        modelAABB.min = glm::vec3(FLT_MAX);
        modelAABB.max = glm::vec3(FLT_MIN);
        
        pParent->Children.push_back(ModelResource());
        ModelResource* pNewModel = &pParent->Children.back();
        pNewModel->pParent = pParent;

        pNewModel->Meshes.resize((size_t)pNode->mNumMeshes);
        for (uint32 m = 0; m < pNode->mNumMeshes; m++)
        {
            uint32 meshIndex    = pNode->mMeshes[m];
            aiMesh* pMesh       = pScene->mMeshes[meshIndex];
            MeshObject& mesh  = pNewModel->Meshes[m];
            FillMesh(pScene, mesh, pMesh, flags, modelPath);

            // Expand the bounding box of this model to fit all meshes inside it.
            modelAABB.min = Maths::GetMinElements(mesh.BoundingBox.min, modelAABB.min);
            modelAABB.max = Maths::GetMaxElements(mesh.BoundingBox.max, modelAABB.max);
        }

        // Add transform.
        pNewModel->Transform = transform;
        transform = glm::mat4(1.f); // Reset the accumulated transform for its children.

        // Add an AABB that encompasses all meshes this node has.
        pNewModel->BoundingBox = modelAABB;

        pNewModel->Name = std::string(pNode->mName.C_Str());

        pTargetParent = pNewModel;
    }
    else
    {
        // Skip saving this node and go to the next nodes instead. However, save the transform.
        pTargetParent = pParent;
        transform = accTransform * transform;
    }

    // Add children if this node has any.
    if (pNode->mNumChildren > 0)
    {
        for (uint32 i = 0; i < pNode->mNumChildren; i++)
        {
            aiNode* pChild = pNode->mChildren[(size_t)i];
            succeeded &= RecursiveLoadMeshes(pScene, pChild, pTargetParent, transform, flags, modelPath);
        }
    }

    // Expand Bounding box of the model to fit all child models inside it.
    for (ModelResource& child : pTargetParent->Children)
    {
        // Expand the bounding box
        pTargetParent->BoundingBox.min = Maths::GetMinElements(child.BoundingBox.min, pTargetParent->BoundingBox.min);
        pTargetParent->BoundingBox.max = Maths::GetMaxElements(child.BoundingBox.max, pTargetParent->BoundingBox.max);
    }

    if (pParent->pParent == nullptr)
    {
        pParent->Name = std::string(pNode->mName.C_Str());
    }

    return succeeded;
}

void ModelLoader::FillMesh(const aiScene*& pScene, MeshObject& outMesh, aiMesh*& pMesh, ModelLoadDesc::LoaderFlags flags, const std::string& modelPath)
{
    // Add Materials
    // TODO: Add default material if the mesh is missing one!
    LoadMaterial(pScene, outMesh, pMesh, flags, modelPath);

    // Add vertices
    outMesh.NumVertices = pMesh->mNumVertices;
    outMesh.Vertices.resize((uint64)outMesh.NumVertices);
    for (uint32 v = 0; v < outMesh.NumVertices; v++)
    {
        MeshObject::Vertex& vertex = outMesh.Vertices[v];

        // Position
        aiVector3D& pos = pMesh->mVertices[v];
        vertex.Position = glm::vec3(pos.x, pos.y, pos.z);

        // Normals
        if (pMesh->HasNormals())
        {
            aiVector3D& norm = pMesh->mNormals[v];
            vertex.Normal = glm::vec3(norm.x, norm.y, norm.z);
        }

        // Tangents and Bitangents
        if (pMesh->HasTangentsAndBitangents())
        {
            aiVector3D& tan = pMesh->mTangents[v];
            aiVector3D& bitan = pMesh->mBitangents[v];
            vertex.Tangent = glm::vec3(tan.x, tan.y, tan.z);
            vertex.Bitangent = glm::vec3(bitan.x, bitan.y, bitan.z);

            // Check if they are correct!
            vertex.Tangent = glm::normalize(vertex.Tangent - glm::dot(vertex.Tangent, vertex.Normal) * vertex.Normal);
            vertex.Bitangent = glm::normalize(glm::cross(vertex.Normal, vertex.Tangent));

            glm::vec3 bi = glm::normalize(glm::cross(vertex.Normal, vertex.Tangent));
            //vertex.Bitangent = bi;
            glm::vec3 tan2 = glm::normalize(glm::cross(vertex.Bitangent, vertex.Normal));
            //vertex.Tangent = tan2;
            glm::vec3 diff = glm::abs(bi) - glm::abs(glm::normalize(vertex.Bitangent));
            if (glm::all(glm::greaterThan(diff, glm::vec3(FLT_EPSILON))))
            {
                LOG_WARNING("TANGENT & BITANGENT are wrong!");
            }
        }

        // UVs
        if (pMesh->HasTextureCoords(0))
        {
            aiVector3D& uvw = pMesh->mTextureCoords[0][v];
            vertex.UV = glm::vec2(uvw.x, uvw.y);
        }
    }

    // Add indices
    // The face will always have three vertices because of triangulation and the primitive removal configuration.
    outMesh.NumIndices = pMesh->mNumFaces * 3;
    outMesh.Indices.resize((uint64)outMesh.NumIndices);
    for (uint32 f = 0; f < pMesh->mNumFaces; f++)
    {
        uint32 index = f * 3;
        aiFace& face = pMesh->mFaces[f];
        outMesh.Indices[(uint64)index + 0] = face.mIndices[0];
        outMesh.Indices[(uint64)index + 1] = face.mIndices[1];
        outMesh.Indices[(uint64)index + 2] = face.mIndices[2];
    }

    // Add bounding box
    outMesh.BoundingBox.min = glm::vec3(pMesh->mAABB.mMin.x, pMesh->mAABB.mMin.y, pMesh->mAABB.mMin.z);
    outMesh.BoundingBox.max = glm::vec3(pMesh->mAABB.mMax.x, pMesh->mAABB.mMax.y, pMesh->mAABB.mMax.z);

    // Upload data to the GUP
    if (flags & ModelLoadDesc::LoaderFlag::LOADER_FLAG_UPLOAD_MESH_DATA_TO_GUP)
    {
        // Create the vertex buffer.
        {
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth = (UINT)(sizeof(MeshObject::Vertex) * outMesh.Vertices.size());
            bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0;
            bufferDesc.MiscFlags = 0;
            bufferDesc.StructureByteStride = 0;

            D3D11_SUBRESOURCE_DATA data;
            data.pSysMem = outMesh.Vertices.data();
            data.SysMemPitch = 0;
            data.SysMemSlicePitch = 0;

            HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &outMesh.pVertexBuffer);
            RS_D311_ASSERT_CHECK(result, "Failed to create vertex buffer!");
        }

        // Create the index buffer.
        {
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth            = (UINT)(sizeof(uint32) * outMesh.Indices.size());
            bufferDesc.Usage                = D3D11_USAGE_IMMUTABLE;
            bufferDesc.BindFlags            = D3D11_BIND_INDEX_BUFFER;
            bufferDesc.CPUAccessFlags       = 0;
            bufferDesc.MiscFlags            = 0;
            bufferDesc.StructureByteStride  = 0;

            D3D11_SUBRESOURCE_DATA data;
            data.pSysMem            = outMesh.Indices.data();
            data.SysMemPitch        = 0;
            data.SysMemSlicePitch   = 0;

            HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &outMesh.pIndexBuffer);
            RS_D311_ASSERT_CHECK(result, "Failed to create index buffer!");
        }

        // Create Constant Buffer to hold transform data and other mesh related data.
        {
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth            = sizeof(MeshObject::MeshData);
            bufferDesc.Usage                = D3D11_USAGE_DYNAMIC;
            bufferDesc.BindFlags            = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.MiscFlags            = 0;
            bufferDesc.StructureByteStride  = 0;

            MeshObject::MeshData initialData = {};
            D3D11_SUBRESOURCE_DATA data;
            data.pSysMem            = &initialData;
            data.SysMemPitch        = 0;
            data.SysMemSlicePitch   = 0;

            HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &outMesh.pMeshBuffer);
            RS_D311_ASSERT_CHECK(result, "Failed to create constant buffer!");
        }
    }

    // Clear the vertices and indices buffers if the flag was set.
    if (flags & ModelLoadDesc::LoaderFlag::LOADER_FLAG_NO_MESH_DATA_IN_RAM)
    {
        outMesh.Vertices.clear();
        outMesh.Indices.clear();
    }
}

void ModelLoader::LoadMaterial(const aiScene*& pScene, MeshObject& outMesh, aiMesh*& pMesh, ModelLoadDesc::LoaderFlags flags, const std::string& modelPath)
{
    RS_UNREFERENCED_VARIABLE(flags);

    auto pResourceManager = ResourceManager::Get();
    uint32 materialIndex = pMesh->mMaterialIndex;
    if(materialIndex >= 0)
    {
        std::string key = modelPath + "_Material_" + std::to_string(materialIndex);
        ResourceID materialID = pResourceManager->GetIDFromString(key);
        if (materialID == 0)
        {
            // Add a new material if it does not exist!
            auto [pMaterialResource, newMaterialID] = pResourceManager->AddResource<MaterialResource>(Resource::Type::MATERIAL);
            pResourceManager->AddStringToIDAssociation(key, newMaterialID);
            materialID = newMaterialID;

            aiMaterial* pMaterial = pScene->mMaterials[materialIndex];

            aiString name;
            pMaterial->Get(AI_MATKEY_NAME, name);
            pMaterialResource->Name = std::string(name.C_Str());

            pMaterialResource->InfoBuffer = {};

            std::string folderPath = modelPath.substr(0, modelPath.find_last_of("\\/")) + "/";

            bool succeeded = false;
            // Load albedo
            {
                pMaterialResource->AlbedoTextureHandler = LoadTextureResource(aiTextureType_DIFFUSE, 0, pScene, pMaterial, pResourceManager->DefaultTextureOnePixelWhite, folderPath, succeeded);
                if (!succeeded)
                    pMaterialResource->AlbedoTextureHandler = LoadTextureResource(aiTextureType_BASE_COLOR, 0, pScene, pMaterial, pResourceManager->DefaultTextureOnePixelWhite, folderPath, succeeded);
                if (!succeeded)
                    pMaterialResource->AlbedoTextureHandler = LoadTextureResource(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, pScene, pMaterial, pResourceManager->DefaultTextureOnePixelWhite, folderPath, succeeded);
            }

            // Load normal map
            {
                pMaterialResource->NormalTextureHandler = LoadTextureResource(aiTextureType_NORMALS, 0, pScene, pMaterial, pResourceManager->DefaultTextureOnePixelNormal, folderPath, succeeded);
                if (!succeeded)
                    pMaterialResource->NormalTextureHandler = LoadTextureResource(aiTextureType_NORMAL_CAMERA, 0, pScene, pMaterial, pResourceManager->DefaultTextureOnePixelNormal, folderPath, succeeded);
                if (!succeeded)
                    pMaterialResource->NormalTextureHandler = LoadTextureResource(aiTextureType_HEIGHT, 0, pScene, pMaterial, pResourceManager->DefaultTextureOnePixelNormal, folderPath, succeeded);
            }

            // Load AO
            {
                pMaterialResource->AOTextureHandler = LoadTextureResource(aiTextureType_AMBIENT_OCCLUSION, 0, pScene, pMaterial, pResourceManager->DefaultTextureOnePixelWhite, folderPath, succeeded);
                if (!succeeded)
                    pMaterialResource->AOTextureHandler = LoadTextureResource(aiTextureType_AMBIENT, 0, pScene, pMaterial, pResourceManager->DefaultTextureOnePixelWhite, folderPath, succeeded);
            }

            bool hasMetallic = false;
            // Load Metallic
            {
                pMaterialResource->MetallicTextureHandler = LoadTextureResource(aiTextureType_METALNESS, 0, pScene, pMaterial, pResourceManager->DefaultTextureOnePixelBlack, folderPath, hasMetallic);
                if (!hasMetallic)
                    pMaterialResource->MetallicTextureHandler = LoadTextureResource(aiTextureType_REFLECTION, 0, pScene, pMaterial, pResourceManager->DefaultTextureOnePixelBlack, folderPath, hasMetallic);
            }

            bool hasRoughness = false;
            // Load Roughness
            {
                pMaterialResource->RoughnessTextureHandler = LoadTextureResource(aiTextureType_DIFFUSE_ROUGHNESS, 0, pScene, pMaterial, pResourceManager->DefaultTextureOnePixelWhite, folderPath, hasRoughness);
                if (!hasRoughness)
                    pMaterialResource->RoughnessTextureHandler = LoadTextureResource(aiTextureType_SHININESS, 0, pScene, pMaterial, pResourceManager->DefaultTextureOnePixelWhite, folderPath, hasRoughness);
            }

            // Load combined Metallic and Roughness
            if(!hasMetallic && !hasRoughness)
            {
                pMaterialResource->MetallicRoughnessTextureHandler = LoadTextureResource(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, pScene, pMaterial, pResourceManager->DefaultTextureOnePixelBlack, folderPath, succeeded);
                pMaterialResource->InfoBuffer.Info.x = 1.f;
            }
            else
            {
                pMaterialResource->MetallicRoughnessTextureHandler = pResourceManager->DefaultTextureOnePixelBlack;
                pMaterialResource->InfoBuffer.Info.x = 0.f;
            }

            // Create the constant buffer
            {
                D3D11_BUFFER_DESC bufferDesc = {};
                bufferDesc.ByteWidth = sizeof(MaterialBuffer);
                bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
                bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                bufferDesc.MiscFlags = 0;
                bufferDesc.StructureByteStride = 0;

                D3D11_SUBRESOURCE_DATA data;
                data.pSysMem = &pMaterialResource->InfoBuffer;
                data.SysMemPitch = 0;
                data.SysMemSlicePitch = 0;

                HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &pMaterialResource->pConstantBuffer);
                RS_D311_ASSERT_CHECK(result, "Failed to create constant buffer for material \"{}\"!", pMaterialResource->Name.c_str());
            }
        }

        outMesh.MaterialHandler = materialID;
    }
}

ResourceID ModelLoader::LoadTextureResource(aiTextureType type, uint32 index, const aiScene*& pScene, aiMaterial* pMaterial, ResourceID defaultTextureID, const std::string& folderPath, bool& succeeded)
{
    auto pResourceManager = ResourceManager::Get();
    ResourceID textureID = 0;

    if (pMaterial->GetTextureCount(type) > index)
    {
        aiString path;
        pMaterial->GetTexture(type, index, &path); // Only take One Texture
        const aiTexture* pEmbeddedTexture = pScene->GetEmbeddedTexture(path.C_Str());
        if (pEmbeddedTexture)
        {
            // The texture is an embedded texture, load it from memory!
            if (std::strcmp(pEmbeddedTexture->achFormatHint, "png") == 0 ||
                std::strcmp(pEmbeddedTexture->achFormatHint, "jpg") == 0)
            {
                TextureLoadDesc loadDesc = {};
                loadDesc.ImageDesc.Memory.pData         = &pEmbeddedTexture->pcData[0].b;
                loadDesc.ImageDesc.Memory.Size          = pEmbeddedTexture->mWidth;
                loadDesc.ImageDesc.Memory.IsCompressed  = true;
                loadDesc.ImageDesc.IsFromFile           = false;
                loadDesc.ImageDesc.NumChannels          = ImageLoadDesc::Channels::RGBA;
                loadDesc.ImageDesc.Name                 = std::string(path.C_Str()); // Use the path as a key!
                auto [pTexture, ID] = pResourceManager->LoadTextureResource(loadDesc);
                textureID = ID;
                succeeded = true;
            }
            else
            {
                LOG_WARNING("Embedded format not supported!");
                pResourceManager->LoadTextureResource(defaultTextureID);
                textureID = defaultTextureID;
                succeeded = false;
            }
        }
        else
        {
            std::string newPath = std::string(path.C_Str());
            std::replace(newPath.begin(), newPath.end(), '\\', '/');

            // Load Texture with ResourceManger!
            TextureLoadDesc loadDesc = {};
            loadDesc.ImageDesc.IsFromFile   = true;
            loadDesc.ImageDesc.File.Path    = folderPath + newPath;
            loadDesc.ImageDesc.File.UseDefaultFolder = false; // Use the same folder as the model.
            loadDesc.ImageDesc.Name         = loadDesc.ImageDesc.File.Path;
            loadDesc.ImageDesc.NumChannels  = ImageLoadDesc::Channels::RGBA;
            auto [pTexture, ID] = ResourceManager::Get()->LoadTextureResource(loadDesc);
            RS_UNREFERENCED_VARIABLE(pTexture);
            textureID = ID;
            succeeded = true;
        }
    }
    else
    {
        pResourceManager->LoadTextureResource(defaultTextureID);
        textureID = defaultTextureID;
        succeeded = false;
    }

    return textureID;
}
