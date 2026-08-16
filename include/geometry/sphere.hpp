// #ifndef __SPHERE_HPP_INCLUDED__
// #define __SPHERE_HPP_INCLUDED__

// #include "hitable.hpp"


// class sphere : public hitable {
// public:

//     sphere(double radius, const point3D& center) 
//     : s_radius(radius), s_center(center) {};
    
//     bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const
//     {
//         vec3 center_camera = s_center - r.origin();
//         double a = r.direction().length_pow2();
//         double h = dot(r.direction(), center_camera);
//         double c = center_camera.length_pow2() - s_radius * s_radius;

//         double D = h*h - a*c;
//         if(D < 0) return false;

//         double D_SQRT = std::sqrt(D);

//         double root = (h - D_SQRT) / a;
//         if(root <= t_min || t_max <= root) 
//         {
//             root = (h + D_SQRT) / a;
//             if(root <= t_min || t_max <= root) return false;
//         }

//         rec.t = root;
//         rec.point = r.at(root);
//         rec.hit_normal = (rec.point - s_center) / s_radius;

//         return true;
//     }

//     inline double  radius() const { return s_radius; }
//     inline point3D center() const { return s_center; }

// private:
//     double  s_radius;
//     point3D s_center;
// };

// #endif