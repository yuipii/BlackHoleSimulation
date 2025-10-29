#include "GravitationalLensing.h"
#include <cmath>
#include <algorithm>

GravitationalLensing::GravitationalLensing() : resolution_(512) {}

void GravitationalLensing::calculate_lensing_pattern(
    const Eigen::Vector3d& black_hole_pos,
    double black_hole_mass,
    const Eigen::Vector3d& camera_pos,
    int resolution) {
    
    resolution_ = resolution;
    lens_map_.clear();
    lens_map_.reserve(resolution_ * resolution_);
    
    double M = black_hole_mass * solar_mass;
    double rs = 2.0 * G * M / (c * c);
    
    for (int i = 0; i < resolution_; ++i) {
        for (int j = 0; j < resolution_; ++j) {
            LensPoint point;
            
            // Convert to normalized screen coordinates [-1, 1]
            point.screen_pos = Eigen::Vector2d(
                (2.0 * i / resolution_ - 1.0),
                (2.0 * j / resolution_ - 1.0)
            );
            
            point.deflection = calculate_single_ray_deflection(
                point.screen_pos, black_hole_pos, camera_pos, black_hole_mass);
            
            // Calculate approximate magnification
            double jacobian = std::abs((1.0 + point.deflection.x()) * (1.0 + point.deflection.y()) - 
                              point.deflection.x() * point.deflection.y());
            point.magnification = 1.0 / std::max(jacobian, 0.001);
            
            lens_map_.push_back(point);
        }
    }
}

Eigen::Vector2d GravitationalLensing::calculate_single_ray_deflection(
    const Eigen::Vector2d& screen_pos,
    const Eigen::Vector3d& black_hole_pos,
    const Eigen::Vector3d& camera_pos,
    double black_hole_mass) const {
    
    double M = black_hole_mass * solar_mass;
    
    // Simple point mass lens model
    Eigen::Vector3d ray_dir = Eigen::Vector3d(screen_pos.x(), screen_pos.y(), -1.0).normalized();
    Eigen::Vector3d ray_origin = camera_pos;
    
    // Find closest approach to black hole
    Eigen::Vector3d to_bh = black_hole_pos - ray_origin;
    double t = to_bh.dot(ray_dir);
    Eigen::Vector3d closest_point = ray_origin + ray_dir * t;
    Eigen::Vector3d impact_vector = black_hole_pos - closest_point;
    double impact_parameter = impact_vector.norm();
    
    // Einstein radius for point mass
    double einstein_radius = std::sqrt(4.0 * G * M / (c * c) * 
                                      std::abs(t) / (impact_parameter + 1e-10));
    
    // Deflection angle (simplified)
    double deflection_angle = 4.0 * G * M / (c * c * impact_parameter);
    
    // Convert to screen deflection
    Eigen::Vector2d deflection;
    if (impact_parameter > 0.0) {
        Eigen::Vector2d impact_dir(impact_vector.x(), impact_vector.y());
        impact_dir.normalize();
        deflection = impact_dir * deflection_angle * 0.1; // Scaling factor
    }
    
    return deflection;
}

Eigen::Vector2d GravitationalLensing::get_deflection(const Eigen::Vector2d& screen_pos) const {
    // Find closest point in lens map (simplified)
    int i = static_cast<int>((screen_pos.x() + 1.0) * 0.5 * resolution_);
    int j = static_cast<int>((screen_pos.y() + 1.0) * 0.5 * resolution_);
    
    i = std::clamp(i, 0, resolution_ - 1);
    j = std::clamp(j, 0, resolution_ - 1);
    
    return lens_map_[i * resolution_ + j].deflection;
}

double GravitationalLensing::get_magnification(const Eigen::Vector2d& screen_pos) const {
    int i = static_cast<int>((screen_pos.x() + 1.0) * 0.5 * resolution_);
    int j = static_cast<int>((screen_pos.y() + 1.0) * 0.5 * resolution_);
    
    i = std::clamp(i, 0, resolution_ - 1);
    j = std::clamp(j, 0, resolution_ - 1);
    
    return lens_map_[i * resolution_ + j].magnification;
}
