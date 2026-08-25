#include "core/Diagnostics.hpp"

#include <cstdlib>
#include <iostream>

#include <glad/glad.h>

namespace Diagnostics {

void glfwErrorCallback(int error, const char* message) {
	std::cerr << "GLFW Error [" << error << "]: " << message << std::endl;
}

void printOpenGLInfo() {
	const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);

    std::cout << "GPU vendor: " << reinterpret_cast<const char*>(vendor) << '\n';
    std::cout << "GPU renderer: " << reinterpret_cast<const char*>(renderer) << '\n';


	std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n\n";

	int work_grp_cnt[3], work_grp_size[3], invocations;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	std::cout << "Max work group count: "
		<< "x: " << work_grp_cnt[0] << ", "
		<< "y: " << work_grp_cnt[1] << ", "
		<< "z: " << work_grp_cnt[2] << '\n';

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

	std::cout << "Max work group size: "
		<< "x: " << work_grp_size[0] << ", "
		<< "y: " << work_grp_size[1] << ", "
		<< "z: " << work_grp_size[2] << '\n';

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &invocations);
	std::cout << "Max work group invocations: " << invocations << "\n\n";
}

}
