#include "ds/bvh.hpp"
#include "scene.hpp"

bvh::bvh()
    : root_index(0)
    , nodes_cnt(1)
    , node_pool(nullptr)
    , triangles(nullptr)
    , t_table(nullptr) {}

void bvh::build(const scene& world)
{
    const size_t triangles_cnt = world.triangles_cnt();

    // Cannot have more than 2 * triangles - 1 nodes
    this->node_pool = arena.alloc<node>(triangles_cnt * 2 - 1); 

    // Allocate pointers for each triangle and default construct them
    this->triangles = arena.alloc<mesh_triangle>(triangles_cnt, false);

    // Index table for triangles
    this->t_table   = arena.alloc<uint32_t>(triangles_cnt);
    
    for(size_t i = 0; i < triangles_cnt; ++i) t_table[i] = i;

    int32_t iter = 0;
    for(const mesh& m: world.geometry)
    {
        for(size_t i = 0; i < m.tvi.size(); i+= 3)
        {
            // Indexes 
            const int32_t& v0_index = m.tvi[i];
            const int32_t& v1_index = m.tvi[i + 1];
            const int32_t& v2_index = m.tvi[i + 2];

            // Construct triangle
            new (&triangles[iter++]) mesh_triangle(m, v0_index, v1_index, v2_index);
        }
    }

    // Create root node
    node& root = node_pool[root_index];
    root.index = 0;                         // the root is a leaf for starters
    root.triangles_cnt = triangles_cnt;

    // Set the root aabb bounds
    update_bounds(root_index);

    // Create tree recursivly
    split(root_index);
}

void bvh::split(const int32_t node_index)
{
    node& n = node_pool[node_index];

    // Decide the axis
    int32_t split_axis = -1;
    fp split_pos = 0;
    fp split_cost = find_best_split_plane(n, split_axis, split_pos);
    
    // bottom condition
    fp parent_cost = get_cost(n);
    if(split_cost >= parent_cost) return;
    
    int32_t axis = split_axis;
    fp split_positon = split_pos;

    // Partion --> Like Quick sort
    int32_t i = n.index;
    int32_t j = i + n.triangles_cnt - 1;
    while(i <= j)
    {
        int32_t idx = t_table[i];
        const point3D& center = triangles[idx].get_center();

        if(center[axis] < split_positon)
        {
            ++i;
        }
        else 
        {
            std::swap(t_table[i], t_table[j--]);
        }
    }

    int32_t left_cnt = i - n.index;

    // We can get a split with no triangles on left or right
    if(left_cnt == 0 || left_cnt == n.triangles_cnt) return;

    int32_t left_child_index  = nodes_cnt++;
    int32_t right_child_index = nodes_cnt++;

    node& l_child = node_pool[left_child_index];
    node& r_child = node_pool[right_child_index];

    l_child.index = n.index;
    l_child.triangles_cnt  = left_cnt;

    r_child.index = i;
    r_child.triangles_cnt  = n.triangles_cnt - left_cnt;

    // making the current root non leaf
    n.triangles_cnt = 0; 
    n.index = left_child_index;

    update_bounds(left_child_index);
    update_bounds(right_child_index);

    // Continue recursion
    split(left_child_index);
    split(right_child_index);
}


bool bvh::trace(const ray& r, fp t_min, fp t_max, hit_record& info) const
{
    int32_t stack[64];
    int32_t top = 0;

    stack[top] = root_index;
    
    bool intersection = false;

    /*
    *   Do two checks a non-leaf cycle.
    *   it can invalidate the the next one by updating the t_max value saving time ...
    */
    while(top >= 0)
    {
        const int32_t idx = stack[top--]; 
        const node& n = node_pool[idx];

        if(n.leaf())
        {
            hit_record tmp;

            for (size_t i = 0; i < n.triangles_cnt; i++)
            {
                const int32_t idx = t_table[n.index + i];
                const mesh_triangle& t = triangles[idx];

                if(t.intersects(r, t_min, t_max, tmp))
                {
                    t_max = tmp.t;
                    intersection = true;
                    info = tmp;
                }
            }
        }
        else
        {
            int32_t l_index = n.index;
            int32_t r_index = n.index + 1;

            // check for intersection
            fp l_dist = intersects(r, node_pool[l_index].box, t_max);
            fp r_dist = intersects(r, node_pool[r_index].box, t_max);

            if(l_dist > r_dist)
            {
                std::swap(l_dist, r_dist);
                std::swap(l_index, r_index);
            }

            // Add the closest one first
            if(r_dist < 1e30f) stack[++top] = r_index;
            if(l_dist < 1e30f) stack[++top] = l_index;
        }
    }

    return intersection;
}


void bvh::update_bounds(const int32_t node_index)
{
    node& n = node_pool[node_index];

    for(size_t i = 0; i < n.triangles_cnt; ++i)
    {
        const size_t index = t_table[n.index + i];
        const mesh_triangle& t = triangles[index];

        n.box.grow_to_include(t);
    }
}

fp bvh::evaluate_SAH(const node& n, int32_t axis, fp pos) const
{
    aabb left, right;
    int32_t l_cnt = 0;
    int32_t r_cnt = 0;

    for (size_t i = 0; i < n.triangles_cnt; i++)
    {
        const mesh_triangle& t = triangles[t_table[n.index + i]];
        if(t.get_center()[axis] < pos)
        {
            left.grow_to_include(t);
            ++l_cnt;
        }
        else
        {
            right.grow_to_include(t);
            ++r_cnt;
        }
    }

    fp cost = l_cnt * left.area() + r_cnt * right.area();
    return cost > 0 ? cost : 1e30;
}


fp bvh::find_best_split_plane(const node& n, int32_t& split_axis, fp& split_pos) const
{
    fp best_cost = 1e30;

    // Preallocated memory 
    fp      left_area[BINS - 1], right_area[BINS - 1];
    int32_t left_cnt[BINS - 1] , right_cnt[BINS - 1];

    for(int32_t axis = 0; axis < 3; ++axis)
    {
        // Crop only where the geometry is
        fp min = 1e30f; fp max = -1e30f;
        for(int32_t i = 0; i < n.triangles_cnt; ++i)
        {
            const mesh_triangle& t = triangles[t_table[n.index + i]];
            min = std::min(min, t.get_center()[axis]);
            max = std::max(max, t.get_center()[axis]);
        }

        if(min == max) continue;

        // Fill bins
        bin bin[BINS]{};
        fp scale = BINS / (max - min);
        for(int32_t i = 0; i < n.triangles_cnt; ++i)
        {
            const mesh_triangle& t = triangles[t_table[n.index + i]];

            // calculate index (mind fp error)
            int32_t index = std::min(BINS - 1, static_cast<int32_t>((t.get_center()[axis] - min) * scale));

            // update bin
            bin[index].triangles_cnt++;
            bin[index].box.grow_to_include(t);
        }

        // Calculate prefix sums
        aabb left_box, right_box;
        int32_t left_sum = 0, right_sum = 0;

        for (int32_t i = 0; i < BINS - 1; ++i)
        {
            // Left cuts
            left_sum += bin[i].triangles_cnt;
            left_cnt[i] = left_sum;
            left_box.grow_to_include(bin[i].box);
            left_area[i] = left_box.area();

            // Right cuts
            right_sum += bin[BINS - 1 - i].triangles_cnt;
            right_cnt[BINS - 2 - i] = right_sum;
            right_box.grow_to_include(bin[BINS - 1 - i].box);
            right_area[BINS - 2 - i] = right_box.area();
        }
    
        // Calculate final costs
        scale = (max - min) / BINS;
        for (int32_t i = 0; i < BINS - 1; ++i)
        {
            fp cost = left_cnt[i] * left_area[i] + right_cnt[i] * right_area[i];
            if(cost < best_cost)
            {
                best_cost = cost;
                split_axis = axis;
                split_pos = min + scale * (i + 1);
            }
        }
        
    }

    return best_cost;
}

fp bvh::get_cost(const node& n) const { return n.box.area() * n.triangles_cnt; }