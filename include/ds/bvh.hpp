#ifndef __BVH_HPP_INCLUDED__
#define __BVH_HPP_INCLUDED__

#include "utils/constants.hpp"
#include "memory/allocator.hpp"

#include "geometry/aabb.hpp"
#include "geometry/mesh_triangle.hpp"
#include "geometry/hitable.hpp"

#include <stack>

// Forward Declaration
class scene;

class bvh {

    /*
    *   **Index** 
    *   Leaf --> the first triangle
    *   Non leaf --> the left child
    */
    struct node
    {
        aabb box;               // 24 bytes
        int32_t index;          // 4 bytes
        int32_t triangles_cnt;  // 4 bytes

        node() : box{}, index(-1), triangles_cnt(0) {};

        inline bool leaf() const { return triangles_cnt > 0; }
    }; // 32 bytes (half cache line)

    /*
    *   Structure for heuristical spliting of planes
    */
    struct bin
    {
        aabb box{};
        int32_t triangles_cnt = 0;
    };

public:

    bvh();

    void build(const scene& world);
    bool trace(const ray& r, fp t_min, fp t_max, hit_record& info) const;
    bool trace_debug(const ray& r, fp t_min, fp t_max, hit_record& info, int target_depth) const;
    
    inline bool empty() const { return nodes_cnt == 0; }

private:

    void split(const int32_t node_index);
    void update_bounds(const int32_t node_index);
    
    fp   evaluate_SAH(const node& n, int32_t axis, fp pos) const;
    fp   find_best_split_plane(const node& n, int32_t& axis, fp& pos) const;
    fp   get_cost(const node& n) const;
    
    // debug
    void get_nodes_at_depth(int, int, int, std::vector<const aabb*>&) const;

    allocator arena;            // allocator 
    node* node_pool;            // all the nodes for the tree
    mesh_triangle* triangles;   // all the triangles inside the tree
    uint32_t* t_table;          // indexes of the triangles for oredering

    int32_t root_index;         // root node index
    int32_t nodes_cnt;          // count of all the nodes

    static constexpr int32_t MAX_TRIANGLES_PER_NODE = 2;  
    static constexpr int32_t BINS                   = 4;
};

#endif