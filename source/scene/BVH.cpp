#include "scene/BVH.hpp"

void BVH::includeVertex(AABB& bounding_box, const Vec4& vertex) {
	bounding_box.min.x = std::min(bounding_box.min.x, vertex.x);
	bounding_box.min.y = std::min(bounding_box.min.y, vertex.y);
	bounding_box.min.z = std::min(bounding_box.min.z, vertex.z);
	bounding_box.max.x = std::max(bounding_box.max.x, vertex.x);
	bounding_box.max.y = std::max(bounding_box.max.y, vertex.y);
	bounding_box.max.z = std::max(bounding_box.max.z, vertex.z);
}

void BVH::includeTriangle(AABB& bounding_box, const Triangle& triangle) {
	includeVertex(bounding_box, triangle.v0);
	includeVertex(bounding_box, triangle.v1);
	includeVertex(bounding_box, triangle.v2);
}

void BVH::buildBoundingBox(AABB& bounding_box, const std::vector<Triangle>& triangles, int start, int count) {
	for (int i = start; i < count + start; ++i) {
		includeTriangle(bounding_box, triangles[m_triangle_indices[i]]);
	}
}

void BVH::build(const std::vector<Triangle>& triangles) {
	// clear any old BVH data
	m_nodes.clear();
	m_triangle_centroids.clear();
	m_triangle_indices.clear();
	// create node BVH
	BVHNode root_node;
	AABB root_bounding_box;

	// build a list of traingle centorids and indices
	m_triangle_centroids.resize(triangles.size());
	m_triangle_indices.resize(  triangles.size());

	int count = static_cast<int>(triangles.size());

	for (int i = 0; i < count; i++)
	{	
		m_triangle_centroids[i] = (triangles[i].v0 + triangles[i].v1 + triangles[i].v2) / 3.0f;
		m_triangle_indices[i] = i;
	}

	// build the root bounding box
	buildBoundingBox(root_bounding_box, triangles, 0, count);

	root_node.count = count;
	root_node.bounding_box = root_bounding_box;

	m_nodes.push_back(root_node);


	buildNode(triangles, 0, 0, count);
}

void BVH::buildNode(const std::vector<Triangle>& triangles, int node_index, int start, int count) {
	BVHNode& node = m_nodes[node_index];

	if (count <= m_k_leaf_limit)
		return;

	// split node
	Vec4 box_size = node.bounding_box.max - node.bounding_box.min;

	// decide axis
	int axis = 0;
	if (box_size.y > box_size.x && box_size.y > box_size.z)
		axis = 1;
	else if (box_size.z > box_size.x && box_size.z > box_size.y)
		axis = 2;

	// sort triangles by centroid along the chosen axis
	std::sort(
		m_triangle_indices.begin() + start,
		m_triangle_indices.begin() + start + count,
		[this, axis](int a, int b)
		{
			return m_triangle_centroids[a][axis] <
				m_triangle_centroids[b][axis];
		}
	);

	int mid = start + count / 2;

	BVHNode left_node;
	BVHNode right_node;
	AABB left_bounding_box;
	AABB right_bounding_box;

	left_node.start = start;
	left_node.count = mid - start;

	right_node.start = mid;
	right_node.count = start + count - mid;

	buildBoundingBox(left_bounding_box, triangles, left_node.start, left_node.count);
	buildBoundingBox(right_bounding_box, triangles, right_node.start, right_node.count);

	left_node.bounding_box = left_bounding_box;
	right_node.bounding_box = right_bounding_box;

	int left_index = static_cast<int>(m_nodes.size());
	m_nodes.push_back(left_node);

	int right_index = static_cast<int>(m_nodes.size());
	m_nodes.push_back(right_node);

	m_nodes[node_index].left = left_index;
	m_nodes[node_index].right = right_index;

	buildNode(triangles, left_index, left_node.start, left_node.count);
	buildNode(triangles, right_index, right_node.start, right_node.count);
}

void BVH::print() const {
	if (m_nodes.empty())
		return;

	printNode(0);
}

void BVH::printNode(int node_index, int depth) const {
	const BVHNode& node = m_nodes[node_index];

	// Calculate bounding box size
	Vec4 size = node.bounding_box.max - node.bounding_box.min;

	// Indentation makes the tree structure visible
	for (int i = 0; i < depth; ++i)
		std::cout << "  ";

	std::cout << "Node " << node_index
		<< " | start: " << node.start
		<< " | count: " << node.count
		<< " | min: ("
		<< node.bounding_box.min.x << ", "
		<< node.bounding_box.min.y << ", "
		<< node.bounding_box.min.z << ")"
		<< " | max: ("
		<< node.bounding_box.max.x << ", "
		<< node.bounding_box.max.y << ", "
		<< node.bounding_box.max.z << ")"
		<< " | size: ("
		<< size.x << ", "
		<< size.y << ", "
		<< size.z << ")"
		<< '\n';

	// Leaf node
	if (node.count <= m_k_leaf_limit)
		return;

	printNode(node.left, depth + 1);
	printNode(node.right, depth + 1);
}


void BVH::exportBoxes(const std::string& filename) const {
	std::ofstream file(filename);

	if (!file.is_open())
	{
		std::cerr << "Failed to open BVH export file: "
			<< filename << '\n';
		return;
	}

	file << "node,depth,start,count,"
		<< "min_x,min_y,min_z,"
		<< "max_x,max_y,max_z,"
		<< "size_x,size_y,size_z\n";

	std::function<void(int, int)> exportNode =
		[&](int node_index, int depth)
		{
			const BVHNode& node = m_nodes[node_index];

			const auto size_x =
				node.bounding_box.max.x - node.bounding_box.min.x;

			const auto size_y =
				node.bounding_box.max.y - node.bounding_box.min.y;

			const auto size_z =
				node.bounding_box.max.z - node.bounding_box.min.z;

			file << std::setprecision(9)
				<< node_index << ','
				<< depth << ','
				<< node.start << ','
				<< node.count << ','
				<< node.bounding_box.min.x << ','
				<< node.bounding_box.min.y << ','
				<< node.bounding_box.min.z << ','
				<< node.bounding_box.max.x << ','
				<< node.bounding_box.max.y << ','
				<< node.bounding_box.max.z << ','
				<< size_x << ','
				<< size_y << ','
				<< size_z << '\n';

			if (node.count <= m_k_leaf_limit)
				return;

			exportNode(node.left, depth + 1);
			exportNode(node.right, depth + 1);
		};

	if (!m_nodes.empty())
		exportNode(0, 0);

	std::cout << "BVH exported to: " << filename << '\n';
}

//BVH::~BVH() {
//	glDeleteBuffers(1, &m_bvhSSBO);
//	glDeleteBuffers(1, &m_triangleIndexSSBO);
//}
