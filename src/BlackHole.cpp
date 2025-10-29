#include "BlackHole.h"
#include <cmath>
#include <random>

BlackHole::BlackHole() : parameters_() {}

BlackHole::BlackHole(const BlackHoleParameters& params) : parameters_(params) {}

Eigen::Vector3d BlackHole::calculate_gravitational_lensing(
    const Eigen::Vector3d& ray_origin, 
    const Eigen::Vector3d& ray_direction) const {
    
    const double G = 6.67430e-11;
    const double c = 299792458.0;
    const double solar_mass = 1.989e30;
    
    double M = parameters_.mass * solar_mass;
    double rs = 2.0 * G * M / (c * c);  // Schwarzschild radius
    
    Eigen::Vector3d bh_pos = parameters_.position;
    Eigen::Vector3d relative_pos = ray_origin - bh_pos;
    
    double impact_parameter = (relative_pos - relative_pos.dot(ray_direction) * ray_direction).norm();
    double deflection_angle = 4.0 * G * M / (c * c * impact_parameter);
    
    // Simple gravitational lensing approximation
    if (impact_parameter < 10.0 * rs) {
        double distortion_factor = rs / impact_parameter;
        Eigen::Vector3d lensed_direction = ray_direction;
        
        // Apply distortion based on impact parameter
        Eigen::Vector3d to_bh = (bh_pos - ray_origin).normalized();
        double dot_product = ray_direction.dot(to_bh);
        
        if (dot_product > 0.1) {
            double distortion = distortion_factor * (1.0 - dot_product);
            lensed_direction = lensed_direction + to_bh * distortion;
            lensed_direction.normalize();
        }
        
        return lensed_direction;
    }
    
    return ray_direction;
}

double BlackHole::calculate_time_dilation(const Eigen::Vector3d& position) const {
    const double G = 6.67430e-11;
    const double c = 299792458.0;
    const double solar_mass = 1.989e30;
    
    double M = parameters_.mass * solar_mass;
    double rs = 2.0 * G * M / (c * c);
    
    Eigen::Vector3d relative_pos = position - parameters_.position;
    double r = relative_pos.norm();
    
    if (r <= rs) return 0.0;  // Inside event horizon
    
    // Schwarzschild time dilation factor
    double time_dilation = std::sqrt(1.0 - rs / r);
    
    return std::max(0.0, time_dilation);
}

double BlackHole::get_photon_sphere_radius() const {
    const double G = 6.67430e-11;
    const double c = 299792458.0;
    const double solar_mass = 1.989e30;
    
    double M = parameters_.mass * solar_mass;
    double rs = 2.0 * G * M / (c * c);
    
    return 1.5 * rs;  // Photon sphere for non-rotating black hole
}

double BlackHole::get_event_horizon_radius() const {
    const double G = 6.67430e-11;
    const double c = 299792458.0;
    const double solar_mass = 1.989e30;
    
    double M = parameters_.mass * solar_mass;
    return 2.0 * G * M / (c * c);
}

std::vector<Eigen::Vector3d> BlackHole::sample_accretion_disk(int num_samples) const {
    std::vector<Eigen::Vector3d> samples;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    double inner_r = parameters_.accretion_disk_inner_radius * get_event_horizon_radius();
    double outer_r = parameters_.accretion_disk_outer_radius * get_event_horizon_radius();
    
    std::uniform_real_distribution<double> radius_dist(std::sqrt(inner_r), std::sqrt(outer_r));
    std::uniform_real_distribution<double> angle_dist(0, 2 * M_PI);
    std::uniform_real_distribution<double> height_dist(-0.1 * outer_r, 0.1 * outer_r);
    
    for (int i = 0; i < num_samples; ++i) {
        double r = std::pow(radius_dist(gen), 2);
        double theta = angle_dist(gen);
        double height = height_dist(gen);
        
        Eigen::Vector3d point(
            r * std::cos(theta),
            height,
            r * std::sin(theta)
        );
        
        samples.push_back(point + parameters_.position);
    }
    
    return samples;
}
