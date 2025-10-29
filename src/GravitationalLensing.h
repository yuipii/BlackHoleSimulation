#ifndef GRAVITATIONALLENSING_H
#define GRAVITATIONALLENSING_H

#include <Eigen/Dense>
#include <vector>

struct LensPoint {
    Eigen::Vector2d screen_pos;
    Eigen::Vector2d deflection;
    double magnification;
};

class GravitationalLensing {
public:
    GravitationalLensing();
    
    void calculate_lensing_pattern(const Eigen::Vector3d& black_hole_pos,
                                 double black_hole_mass,
                                 const Eigen::Vector3d& camera_pos,
                                 int resolution = 512);
    
    Eigen::Vector2d get_deflection(const Eigen::Vector2d& screen_pos) const;
    double get_magnification(const Eigen::Vector2d& screen_pos) const;
    
    const std::vector<LensPoint>& get_lens_map() const { return lens_map_; }
    
    void set_resolution(int resolution) { resolution_ = resolution; }
    
private:
    std::vector<LensPoint> lens_map_;
    int resolution_;
    
    const double G = 6.67430e-11;
    const double c = 299792458.0;
    const double solar_mass = 1.989e30;
    
    Eigen::Vector2d calculate_single_ray_deflection(const Eigen::Vector2d& screen_pos,
                                                  const Eigen::Vector3d& black_hole_pos,
                                                  const Eigen::Vector3d& camera_pos,
                                                  double black_hole_mass) const;
};

#endif
