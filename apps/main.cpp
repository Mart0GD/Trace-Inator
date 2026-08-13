#include "utils/constants.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "renderer.hpp"


int main()
{   
    scene world("scenes/HW_11/scene3.crtscene");

    renderer r;
    r.world = &world;

    r.render();
    return 0;
}
