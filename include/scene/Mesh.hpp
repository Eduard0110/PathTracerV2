#pragma once

#include <vector>
#include <string>
#include "Geometry.hpp"

class Mesh {
public:
	Mesh(const std::string& file_path, uint32_t id);
	~Mesh() = default;

	void loadOBJ(const std::string& file_path, uint32_t id);
	const std::vector<Triangle>& getTriangles() const { return m_triangles; }

	uint32_t getId() const { return m_id; }
	const std::string& getName() const { return m_name; }
	const std::string& getFilePath() const { return m_file_path; }

private:
	uint32_t m_id;
	std::string m_name;
	std::string m_file_path;
    std::vector<Triangle> m_triangles;
};
