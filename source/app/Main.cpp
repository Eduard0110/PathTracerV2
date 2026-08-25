#include "app/App.hpp"

#include <exception>
#include <iostream>

#if defined(_WIN32)
// Hybrid-graphics laptops can otherwise select the integrated GPU. NVIDIA
// Optimus and AMD PowerXpress recognise these exported application hints.
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main() {
	try {
		// Change this to false when you want to start with an empty material list.
		constexpr bool load_standard_materials_on_startup = true;
		App app(load_standard_materials_on_startup);
		app.run();
	}
	catch (const std::exception& exception) {
		std::cerr << "Fatal application error: " << exception.what() << '\n';
		return 1;
	}
	catch (...) {
		std::cerr << "Fatal application error: unknown exception\n";
		return 1;
	}

	return 0;
}
