#include "Scene/Scene.hpp"

uint32_t Scene::loadMesh(const std::string& file_path) {
	m_meshes.emplace_back(file_path, m_mesh_id_counter);

	return m_mesh_id_counter++;
};

namespace {
	Vec4 rotateXYZ(const Vec4& vector, const Vec3& rotation) {
		Vec4 result = vector;

		// rotate around X
		const float sin_x = std::sin(rotation.x);
		const float cos_x = std::cos(rotation.x);
		result = {
			result.x,
			result.y * cos_x - result.z * sin_x,
			result.y * sin_x + result.z * cos_x,
			result.w
		};

		// rotate around Y
		const float sin_y = std::sin(rotation.y);
		const float cos_y = std::cos(rotation.y);
		result = {
			result.x * cos_y + result.z * sin_y,
			result.y,
			-result.x * sin_y + result.z * cos_y,
			result.w
		};

		// Rotate around Z.
		const float sin_z = std::sin(rotation.z);
		const float cos_z = std::cos(rotation.z);
		result = {
			result.x * cos_z - result.y * sin_z,
			result.x * sin_z + result.y * cos_z,
			result.z,
			result.w
		};

		return result;
	}

	Vec4 transformPosition(const Vec4& position, const Transform& transform) {
		Vec4 result{
			position.x * transform.scale.x,
			position.y * transform.scale.y,
			position.z * transform.scale.z,
			1.0f
		};

		result = rotateXYZ(result, transform.rotation);
		result.x += transform.position.x;
		result.y += transform.position.y;
		result.z += transform.position.z;
		return result;
	}

	Vec4 transformNormal(const Vec4& normal, const Transform& transform) {
		// Dividing by scale applies the inverse-transpose needed for non-uniform
		// scaling before the normal is rotated into world space.
		if (transform.scale.x == 0.0f || transform.scale.y == 0.0f || transform.scale.z == 0.0f)
			return { 0.0f, 0.0f, 0.0f, 0.0f };

		Vec4 result{
			normal.x / transform.scale.x,
			normal.y / transform.scale.y,
			normal.z / transform.scale.z,
			0.0f
		};

		result = rotateXYZ(result, transform.rotation);

		const float length = std::sqrt(
			result.x * result.x +
			result.y * result.y +
			result.z * result.z
		);

		if (length > 0.0f) {
			result.x /= length;
			result.y /= length;
			result.z /= length;
		}

		result.w = 0.0f;
		return result;
	}

}

Triangle Scene::transformTriangle(const Triangle& triangle, const Transform& transform) {
	Triangle result = triangle;

	result.v0 = transformPosition(triangle.v0, transform);
	result.v1 = transformPosition(triangle.v1, transform);
	result.v2 = transformPosition(triangle.v2, transform);

	result.n0 = transformNormal(triangle.n0, transform);
	result.n1 = transformNormal(triangle.n1, transform);
	result.n2 = transformNormal(triangle.n2, transform);

	return result;
}


uint32_t Scene::createMaterial(Material material) {
	// Materials are intentionally append-only. Their public ID is therefore
	// also their dense GPU-buffer index.
	if (m_materials.size() >= std::numeric_limits<uint32_t>::max())
		throw std::overflow_error("Too many materials in scene");

	material.id = static_cast<uint32_t>(m_materials.size());
	m_materials.push_back(std::move(material));

	return m_materials.back().id;
}


uint32_t Scene::createObject(const std::string& name, uint32_t mesh_id, uint32_t material_id, const Transform& transform) {
	if(getMesh(mesh_id) == nullptr)
		throw std::invalid_argument("Invalid mesh ID");

	if (getMaterial(material_id) == nullptr)
		throw std::invalid_argument("Invalid material ID");

	const uint32_t object_id = m_scene_objects_id_counter;

	SceneObject object{ name, object_id, mesh_id, material_id, transform, true };

	m_scene_objects.push_back(std::move(object));

	++m_scene_objects_id_counter;

	return object_id;
}

const Mesh* Scene::getMesh(uint32_t meshId) const {
	for (const auto& mesh : m_meshes)
		if (mesh.getId() == meshId)
			return &mesh;

	return nullptr;
}

const Material* Scene::getMaterial(uint32_t materialId) const {
	for (const auto& material : m_materials)
		if (material.id == materialId)
			return &material;

	return nullptr;
}

const SceneObject* Scene::getObject(uint32_t object_id) const {
	for (const SceneObject& object : m_scene_objects)
		if (object.object_id == object_id)
			return &object;

	return nullptr;
}

std::size_t Scene::getLoadedTriangleCount() const {
	std::size_t count = 0;
	for (const Mesh& mesh : m_meshes)
		count += mesh.getTriangles().size();
	return count;
}

std::size_t Scene::getRenderedTriangleCount() const {
	std::size_t count = 0;
	for (const SceneObject& object : m_scene_objects) {
		if (!object.visible)
			continue;

		const Mesh* mesh = getMesh(object.mesh_id);
		if (mesh != nullptr)
			count += mesh->getTriangles().size();
	}
	return count;
}

bool Scene::setObjectTransform(
	uint32_t object_id,
	const Transform& transform)
{
	for (SceneObject& object : m_scene_objects) {
		if (object.object_id != object_id)
			continue;

		object.transform = transform;
		return true;
	}

	return false;
}

bool Scene::setObjectMaterial(uint32_t object_id, uint32_t material_id) {
	if (getMaterial(material_id) == nullptr)
		return false;

	for (SceneObject& object : m_scene_objects) {
		if (object.object_id != object_id)
			continue;

		object.material_id = material_id;
		return true;
	}

	return false;
}

bool Scene::setObjectVisibility(uint32_t object_id, bool visible) {
	for (SceneObject& object : m_scene_objects) {
		if (object.object_id != object_id)
			continue;

		object.visible = visible;
		return true;
	}

	return false;
}

bool Scene::setMaterial(uint32_t material_id, const Material& material) {
	for (Material& current_material : m_materials) {
		if (current_material.id != material_id)
			continue;

		// Identity belongs to Scene and cannot be changed by edited properties.
		const uint32_t preserved_id = current_material.id;
		current_material = material;
		current_material.id = preserved_id;
		return true;
	}

	return false;
}


void Scene::rebuildScene() {
	m_triangles.clear();

	std::size_t required_triangle_count = 0;

	// count all triangles and then reserve memory
	for (const SceneObject& object : m_scene_objects) {
		if (!object.visible)
			continue;

		const Mesh* mesh = getMesh(object.mesh_id);

		if (mesh != nullptr)
			required_triangle_count += mesh->getTriangles().size();
	}

	m_triangles.reserve(required_triangle_count);

	// transform then move every triangle
	for (const SceneObject& object : m_scene_objects) {
		if (!object.visible)
			continue;

		const Mesh* mesh = getMesh(object.mesh_id);

		if (mesh == nullptr)
			// Log an error here if desired.
			continue;


		for (const Triangle& sourceTriangle : mesh->getTriangles()) {
			Triangle triangle = transformTriangle(sourceTriangle, object.transform);

			triangle.object_id = object.object_id;
			triangle.mesh_id = object.mesh_id;
			triangle.material_id = object.material_id;

			m_triangles.push_back(std::move(triangle));
		}
	}

	m_bvh.build(m_triangles);
}
