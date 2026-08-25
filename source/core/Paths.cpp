#include "core/Paths.hpp"

#include <stdexcept>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Paths {
    const std::filesystem::path& executableDirectory() {
        static const std::filesystem::path directory = [] {

            std::wstring executable_path(32768, L'\0');
            const DWORD length = GetModuleFileNameW(
                nullptr,
                executable_path.data(),
                static_cast<DWORD>(executable_path.size())
            );

            if (length == 0 || length >= executable_path.size())
                throw std::runtime_error("Could not determine the PathTracer executable directory");

            executable_path.resize(length);
            return std::filesystem::path(executable_path).parent_path();
        }();

        return directory;
    }

    std::filesystem::path shader(const std::filesystem::path& relative_path) {
        return executableDirectory() / "shaders" / relative_path;
    }

    std::filesystem::path resource(const std::filesystem::path& relative_path) {
        return executableDirectory() / "resources" / relative_path;
    }

    std::filesystem::path renderOutput(const std::filesystem::path& filename) {
        const std::filesystem::path directory = executableDirectory() / "renders";
        std::error_code error;
        std::filesystem::create_directories(directory, error);
        if (error)
            throw std::runtime_error("Could not create the renders folder: " + error.message());
        return directory / filename.filename();
    }

    std::filesystem::path imguiSettings() {
        return executableDirectory() / "imgui.ini";
    }
}
