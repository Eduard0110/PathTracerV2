#pragma once

#include <cstddef>
#include <vector>

#include "scene/Mesh.hpp"
#include "scene/BVH.hpp"
#include "scene/Geometry.hpp"
#include "camera/Camera.hpp"

class Scene {
public:
    Scene()  = default;
    ~Scene() = default;

	void rebuildScene();  // clears initial scene and rebuilds it with current meshes
	uint32_t loadMesh(const std::string& file_path);
	uint32_t createMaterial(Material material);
	uint32_t createObject(const std::string& name, uint32_t mesh_id, uint32_t material_id, const Transform& transform = {});

	// BVH and triangle getters for SSBO 
	const std::vector<Triangle>& getTriangles() const { return m_triangles; }
	const std::vector<BVHNode>& getBvhNodes() const { return m_bvh.getBvhNodes(); }
	const std::vector<int>& getBvhTriangleIndicies() const { return m_bvh.getBvhTriangleIndices(); }
	std::size_t getLoadedTriangleCount() const;
	std::size_t getRenderedTriangleCount() const;

	// camera getter / mutable access
	Camera& getCamera() { return m_camera;  }
	const Camera& getCamera() const { return m_camera; }

	// mainly gui helpers
	const std::vector<Material>& getMaterials() const { return m_materials; }
	const std::vector<SceneObject>& getObjects() const { return m_scene_objects; }
	const std::vector<Mesh>& getMeshes() const { return m_meshes; }

	// mesh look up in case if a mesh is deleted, which will cause indicies to misalign
	const Mesh* getMesh(uint32_t mesh_id) const;
	const Material* getMaterial(uint32_t material_id) const;
	const SceneObject* getObject(uint32_t object_id) const;

	bool setObjectTransform(uint32_t object_id, const Transform& transform);
	bool setObjectMaterial(uint32_t object_id, uint32_t material_id);
	bool setObjectVisibility(uint32_t object_id, bool visible);
	bool setMaterial(uint32_t material_id, const Material& material);

private:
	// transform helpers
	static Triangle transformTriangle(const Triangle& triangle, const Transform& transform);

private:
    std::vector<Mesh> m_meshes;
	std::vector<Triangle> m_triangles;  // contains triangles from all meshes
	std::vector<SceneObject> m_scene_objects;
	std::vector<Material> m_materials;
	BVH m_bvh;  // Bounding Volume Hierarchy for efficient path tracing
	

	uint32_t m_mesh_id_counter = 0;
	uint32_t m_scene_objects_id_counter = 0;

	Camera m_camera;
};
