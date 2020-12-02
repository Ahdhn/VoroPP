#include <chrono>
#include <iostream>
#include <random>
#include "VoroPP.h"

int main(int argv, char** argc)
{


    // init Voro with the computation bounding box (all vertices should be
    // inside this bounding box)
    Voro vpp(0, 1, 0, 1, 0, 1);

    int num_vertices = 100;

    // start timer
    std::chrono::high_resolution_clock::time_point start_time =
        std::chrono::high_resolution_clock::now();

    // Since, we don't know what data-structure the user is using (std::vector,
    // array, or custom data data structure), we take the input by a call back
    // lambda function. For the sake of this example, we generate the vertices
    // on the fly.
    vpp.getVoronoi(
        num_vertices,  // Total number of vertices

        // Lambda function to return the vertex location given
        // the vertex ID and its coordinate ID (0 for x, 1 for y, and 2 for z)
        [&](int index, int coord_id) {
            return double(rand()) / double(RAND_MAX);
        });

    // stop timer
    std::chrono::high_resolution_clock::time_point end_time =
        std::chrono::high_resolution_clock::now();

    // report time
    std::cout << "Voro++ took (including memory allocation) = "
              << std::chrono::duration<float>(end_time - start_time).count()
              << " (seconds)" << std::endl;

    // generate obj file with the computed voronoi
    vpp.drawVoronoi("my_vpp.obj");

    return 0;
}