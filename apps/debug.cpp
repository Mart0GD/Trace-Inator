#include "utils/constants.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "renderer.hpp"

#include <iostream>
#include <fstream>
#include <string>

int main()
{   
    scene world("../scenes/HW_13/scene1.crtscene");

    renderer r;
    r.world = &world;

    int current_depth = 0;
    char command;

    std::cout << "--- BVH Interactive Debugger ---\n";
    std::cout << "Controls:\n";
    std::cout << "  '+' / 'n' -> Next depth level\n";
    std::cout << "  '-' / 'p' -> Previous depth level\n";
    std::cout << "  'q'       -> Quit\n";

    while (true) {
        std::cout << "\nRendering BVH at depth: " << current_depth << "...\n";

        std::string filename = "../debug.ppm";
        std::ofstream out(filename);

        r.render(out, current_depth);

        out.close();
        std::cout << "Enter command (+ / - / q): ";
        
        if (!(std::cin >> command) || command == 'q') {
            break;
        }

        if (command == '+' || command == 'n') {
            current_depth++;
        } else if ((command == '-' || command == 'p') && current_depth > 0) {
            current_depth--;
        }
    }

    return 0;
}