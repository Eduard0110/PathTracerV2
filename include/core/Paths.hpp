#pragma once

#include <filesystem>

namespace Paths {
    // The directory containing PathTracer.exe. This is deliberately not the
    // process working directory, which can change depending on how it starts.
    const std::filesystem::path& executableDirectory();

    std::filesystem::path shader(const std::filesystem::path& relative_path);
    std::filesystem::path resource(const std::filesystem::path& relative_path);
    std::filesystem::path renderOutput(const std::filesystem::path& filename);
    std::filesystem::path imguiSettings();
}
