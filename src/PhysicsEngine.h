#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H

#include "BlackHole.h"
#include <Eigen/Dense>
#include <vector>

struct CelestialBody {
    Eigen::Vector3d position;
    Eigen::Vector3d velocity;
    Eigen::Vector3d acceleration;
    double mass;
    double radius;
    
    CelestialBody(const Eigen::Vector3d& pos, const Eigen::Vector3d& vel, double m, double r)
        : position(pos), velocity(vel), acceleration(Eigen::Vector3d::Zero()), mass(m), radius(r) {}
};

class PhysicsEngine {
public:
    PhysicsEngine();
    
    void set_black_hole(const std::shared_ptr<BlackHole>& black_hole);
    void add_body(const CelestialBody& body);
    
    void update(double delta_time);
    
    const std::vector<CelestialBody>& get_bodies() const { return bodies_; }
    std::shared_ptr<BlackHole> get_black_hole() const { return black_hole_; }
    
    // Calculate tidal forces
    Eigen::Vector3d calculate_tidal_forces(const Eigen::Vector3d& position) const;
    
private:
    std::shared_ptr<BlackHole> black_hole_;
    std::vector<CelestialBody> bodies_;
    
    const double G = 6.67430e-11;  // Gravitational constant
    
    void compute_gravitational_forces();
};

#endif
