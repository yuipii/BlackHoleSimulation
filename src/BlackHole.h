#ifndef BLACKHOLE_H
#define BLACKHOLE_H

#include <Eigen/Dense>
#include <vector>
#include <memory>

struct BlackHoleParameters {
    double mass;  // Solar masses
    double spin;  // Kerr parameter (0-1)
    double accretion_disk_inner_radius;
    double accretion_disk_outer_radius;
    Eigen::Vector3d position;
    
    BlackHoleParameters() : 
        mass(100000000.0),  // 100 million solar masses
        spin(0.6),
        accretion_disk_inner_radius(1.5),
        accretion_disk_outer_radius(40.0),
        position(0, 0, 0) {}
};

class BlackHole {
public:
    BlackHole();
    explicit BlackHole(const BlackHoleParameters& params);
    
    // Gravitational lensing calculations
    Eigen::Vector3d calculate_gravitational_lensing(const Eigen::Vector3d& ray_origin, 
                                                   const Eigen::Vector3d& ray_direction) const;
    
    // Time dilation factor at given position
    double calculate_time_dilation(const Eigen::Vector3d& position) const;
    
    // Calculate photon sphere properties
    double get_photon_sphere_radius() const;
    double get_event_horizon_radius() const;
    
    // Accretion disk sampling
    std::vector<Eigen::Vector3d> sample_accretion_disk(int num_samples) const;
    
    const BlackHoleParameters& get_parameters() const { return parameters_; }
    
private:
    BlackHoleParameters parameters_;
    
    // Kerr metric calculations
    double kerr_metric_component(const Eigen::Vector4d& position) const;
    Eigen::Vector4d calculate_geodesic_derivative(const Eigen::Vector4d& position, 
                                                 const Eigen::Vector4d& momentum) const;
    
    // Ray tracing through curved spacetime
    void trace_geodesic(Eigen::Vector3d& position, Eigen::Vector3d& direction, 
                       double step_size, int max_steps) const;
};

#endif
