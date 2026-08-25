#pragma once

#include <vector>
#include <limits>
#include <algorithm>

#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>

//#include <glad/glad.h>
#include "Geometry.hpp"


class BVH {
public:
	BVH()  = default;
	~BVH() = default;

	// include all triangles in a bounding box
	void includeVertex(AABB& bounding_box, const Vec4& vertex);
	void includeTriangle(AABB& bounding_box, const Triangle& triangle);
	void buildBoundingBox(AABB& bounding_box, const std::vector<Triangle>& triangles, int start, int count);

	void build(const std::vector<Triangle>& triangles);
	void buildNode(const std::vector<Triangle>& triangles, int node_index, int start, int count);
	//void uploadToGpu();

	const std::vector<BVHNode>& getBvhNodes() const { return m_nodes; }
	const std::vector<int>& getBvhTriangleIndices() const { return m_triangle_indices; }

	void printNode(int node_index, int depth = 0) const;
	void print() const;
	void exportBoxes(const std::string& filename) const;

private:
	std::vector<BVHNode> m_nodes;
	std::vector<Vec4> m_triangle_centroids;
	std::vector<int> m_triangle_indices;

	static inline constexpr int m_k_leaf_limit = 3;
	//GLuint m_bvhSSBO = 0;
	//GLuint m_triangleIndexSSBO = 0;
};
