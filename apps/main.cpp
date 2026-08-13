#include "utils/constants.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "renderer.hpp"

#include <chrono>
#include <iostream>

int main()
{   
    scene world("scenes/HW_11/scene3.crtscene");

    renderer r;
    r.world = &world;

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    
    r.render();
    
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    const double seconds = duration.count() / 1'000'000.0;
    std::cout << "time for exectution: " << seconds << " seconds." << std::endl;
    return 0;
}
