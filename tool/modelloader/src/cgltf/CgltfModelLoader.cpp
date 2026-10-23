#include "CgltfModelLoader.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <filesystem>

namespace suz
{
#pragma region LOAD_GLTF_HELPER_FUNCTION
    cgltf_data* LoadGltfFile(const std::string& FilePath);

    bool CollectCgltfDataOnScenes(cgltf_data* Data, std::vector<ModelData>& OutModels, const std::string& FileFolderPath);
    bool CollectCgltfDataOnMeshes(cgltf_data* Data, std::vector<ModelData>& OutModels, const std::string& FileFolderPath);

    void LoadAccessorFloats(const cgltf_accessor* Accessor, std::vector<float>& OutData);
    bool LoadPrimitiveVertices(const cgltf_primitive* Primitive, std::vector<ModelVertexData>& OutVertices);
    bool LoadPrimitiveIndices(const cgltf_primitive* Primitive, std::vector<uint32>& OutIndices);
    bool LoadPrimitiveMaterial(const cgltf_primitive* Primitive, MaterialInfo& OutMaterial);

#pragma endregion

    std::vector<ModelData> CgltfModelLoader::LoadTargetModel(const std::string& FilePath)
    {
        cgltf_data* loadedData = LoadGltfFile(FilePath);
        std::string fileFolderPath = GetFileFolderPath(FilePath);

        std::vector<ModelData> modelData{};
        bool result = CollectCgltfDataOnScenes(loadedData, modelData, fileFolderPath);
        if (!result)
        {
            result = CollectCgltfDataOnMeshes(loadedData, modelData, fileFolderPath);
        }
        cgltf_free(loadedData);

        if (!result) return {};

        return modelData;
    }

    void CgltfModelLoader::UnLoadTargetModel(ModelData& ModelData)
    {
    }

    bool CgltfModelLoader::TargetModelFormatSupport(const std::string& ExtensionName)
    {
        return true;
    }

    std::string CgltfModelLoader::GetFileFolderPath(const std::string& FilePath)
    {
        std::filesystem::path filePath(FilePath);
        
        return filePath.parent_path().string();
    }

#pragma region LOAD_GLTF_HELPER_FUNCTION
    cgltf_data* LoadGltfFile(const std::string& FilePath)
    {
        /* 默认加载配置 */
        cgltf_options options = {};
        cgltf_data* data = nullptr;

        cgltf_result fileParseResult = cgltf_parse_file(&options, FilePath.c_str(), &data);
        if (fileParseResult != cgltf_result_success)
        {
            return nullptr;
        }

        /* 读取外部bin缓冲区文件 */
        fileParseResult = cgltf_load_buffers(&options, data, FilePath.c_str());
        if (fileParseResult != cgltf_result_success)
        {
            /* 读取缓冲区文件失败 */
            cgltf_free(data);
            return nullptr;
        }

        return data;
    }

    bool CollectCgltfDataOnScenes(cgltf_data* Data, std::vector<ModelData>& OutModels, const std::string& FileFolderPath)
    {
        if (!OutModels.empty())
        {
            OutModels.clear();
        }

        /* 遍历所有节点 */
        for (usize nodeIndex = 0; nodeIndex < Data->scene->nodes_count; nodeIndex++)
        {
            cgltf_node* node = Data->scene->nodes[nodeIndex];
            if (!node->mesh) continue;

            cgltf_mesh* mesh = node->mesh;
            for (usize primitiveIndex = 0; primitiveIndex < mesh->primitives_count; primitiveIndex++)
            {
                cgltf_primitive* primitive = &mesh->primitives[primitiveIndex];
                if (primitive->type != cgltf_primitive_type_triangles) continue;

                ModelData modelData{};

                std::vector<ModelVertexData> vertices{};
                std::vector<uint32> indices{};
                MaterialInfo material{};

                /* 读取顶点数据 */
                if (LoadPrimitiveVertices(primitive, vertices))
                {
                    /* 读取顶点索引 */
                    LoadPrimitiveIndices(primitive, indices);

                    modelData.vertices = vertices;
                    modelData.indices = indices;
                }
                
                if (LoadPrimitiveMaterial(primitive, material))
                {
                    modelData.material = material;
                }

                modelData.modelFolderPath = FileFolderPath;

                OutModels.push_back(modelData);
            }
        }

        if (!OutModels.empty())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool CollectCgltfDataOnMeshes(cgltf_data* Data, std::vector<ModelData>& OutModels, const std::string& FileFolderPath)
    {
        if (!OutModels.empty())
        {
            OutModels.clear();
        }

        for (usize meshIndex = 0; meshIndex < Data->meshes_count; meshIndex++)
        {
            cgltf_mesh* mesh = &Data->meshes[meshIndex];
            if (!mesh) continue;

            for (usize primitiveIndex = 0; primitiveIndex < mesh->primitives_count; primitiveIndex++)
            {
                cgltf_primitive* primitive = &mesh->primitives[primitiveIndex];
                if (primitive->type != cgltf_primitive_type_triangles) continue;

                ModelData modelData{};

                std::vector<ModelVertexData> vertices{};
                std::vector<uint32> indices{};
                MaterialInfo material;

                if (LoadPrimitiveVertices(primitive, vertices))
                {
                    LoadPrimitiveIndices(primitive, indices);

                    modelData.vertices = vertices;
                    modelData.indices = indices;
                }

                if (LoadPrimitiveMaterial(primitive, material))
                {
                    modelData.material = material;
                }

                modelData.modelFolderPath = FileFolderPath;

                OutModels.push_back(modelData);
            }
        }

        if (!OutModels.empty())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    /* 将Accessor读取成Float数组 */
    void LoadAccessorFloats(const cgltf_accessor* Accessor, std::vector<float>& OutData)
    {
        if (!Accessor) return;

        usize componentCount = cgltf_num_components(Accessor->type);
        OutData.resize(Accessor->count * componentCount);

        cgltf_accessor_unpack_floats(Accessor, OutData.data(), OutData.size());
    }

    /* 从一个图元生成顶点数组 */
    bool LoadPrimitiveVertices(const cgltf_primitive* Primitive, std::vector<ModelVertexData>& OutVertices)
    {
        /* 按照type取属性 */
        /* 第二参数是cgltf属性类型, 找到该类型的Accessor */
        const cgltf_accessor* locationAcc = cgltf_find_accessor(Primitive, cgltf_attribute_type_position, 0);
        const cgltf_accessor* colorAcc = cgltf_find_accessor(Primitive, cgltf_attribute_type_color, 0);
        const cgltf_accessor* texcoordAcc = cgltf_find_accessor(Primitive, cgltf_attribute_type_texcoord, 0);

        if (!locationAcc) return false;

        std::vector<float> locationData{};
        std::vector<float> colorData{};
        std::vector<float> texcoordData{};

        LoadAccessorFloats(locationAcc, locationData);
        LoadAccessorFloats(colorAcc, colorData);
        LoadAccessorFloats(texcoordAcc, texcoordData);

        /* 分配所需顶点数量内存 */
        usize verticesCount = locationAcc->count;
        OutVertices.resize(verticesCount);

        for (usize vertexIndex = 0; vertexIndex < verticesCount; vertexIndex++)
        {
            ModelVertexData vertex{};
            vertex.location = { locationData[vertexIndex * 3 + 0],locationData[vertexIndex * 3 + 1],locationData[vertexIndex * 3 + 2] };

            if (texcoordAcc)
            {
                vertex.texCoord = { texcoordData[vertexIndex * 2 + 0],texcoordData[vertexIndex * 2 + 1] };
            }
            else
            {
                vertex.texCoord = { 0.0f,0.0f };
            }

            if (colorAcc)
            {
                /* 颜色属性可能是vec3或vec4, 按照实际组件数处理 */
                usize count = cgltf_num_components(colorAcc->type);
                vertex.color = { colorData[vertexIndex * count + 0],colorData[vertexIndex * count + 1],colorData[vertexIndex * count + 2] };
            }
            else
            {
                vertex.color = { 1.0f,1.0f,1.0f };
            }

            OutVertices[vertexIndex] = vertex;
        }

        return true;
    }

    /* 从一个图元生成索引数组 */
    bool LoadPrimitiveIndices(const cgltf_primitive* Primitive, std::vector<uint32>& OutIndices)
    {
        if (!Primitive->indices) return false;

        OutIndices.resize(Primitive->indices->count);
        cgltf_accessor_unpack_indices(Primitive->indices, OutIndices.data(), sizeof(uint32), Primitive->indices->count);
        
        return true;
    }

    /* 从一个材质读取所需贴图 */
    bool LoadPrimitiveMaterial(const cgltf_primitive* Primitive, MaterialInfo& OutMaterial)
    {
        if (!Primitive->material) return false;
        cgltf_material* material = Primitive->material;

        OutMaterial.baseColorFactor = {
            material->pbr_metallic_roughness.base_color_factor[0],
            material->pbr_metallic_roughness.base_color_factor[1],
            material->pbr_metallic_roughness.base_color_factor[2],
            material->pbr_metallic_roughness.base_color_factor[3]
        };

        /* 基础颜色贴图 */
        cgltf_image* image = material->pbr_metallic_roughness.base_color_texture.texture->image;
        if (image->uri)
        {
            OutMaterial.baseColorTexturePath = std::basic_string(image->uri);
        }

        return true;
    }

#pragma endregion
}