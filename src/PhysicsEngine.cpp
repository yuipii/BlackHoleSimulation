#include "PhysicsEngine.h"

PhysicsEngine::PhysicsEngine() : black_hole_(nullptr) {}

void PhysicsEngine::set_black_hole(const std::shared_ptr<BlackHole>& black_hole) {
    black_hole_ = black_hole;
}

void PhysicsEngine::add_body(const CelestialBody& body) {
    bodies_.push_back(body);
}

void PhysicsEngine::update(double delta_time) {
    compute_gravitational_forces();
    
    // Update bodies using Verlet integration
    for (auto& body : bodies_) {
        Eigen::Vector3d new_position = body.position + body.velocity * delta_time + 
                                      0.5 * body.acceleration * delta_time * delta_time;
        
        // Store old acceleration
        Eigen::Vector3d old_acceleration = body.acceleration;
        
        // Update position
        body.position = new_position;
        
        // Compute new acceleration
        compute_gravitational_forces();
        
        // Update velocity
        body.velocity += 0.5 * (old_acceleration + body.acceleration) * delta_time;
    }
}

void PhysicsEngine::compute_gravitational_forces() {
    const double solar_mass = 1.989e30;
    
    for (auto& body : bodies_) {
        body.acceleration = Eigen::Vector3d::Zero();
        
        // Black hole gravity
        if (black_hole_) {
            Eigen::Vector3d to_bh = black_hole_->get_parameters().position - body.position;
            double distance = to_bh.norm();
            
            if (distance > 0.0) {
                double bh_mass = black_hole_->get_parameters().mass * solar_mass;
                double force_magnitude = G * bh_mass / (distance * distance);
                body.acceleration += to_bh.normalized() * force_magnitude;
            }
        }
        
        // Other bodies gravity (simplified)
        for (const auto& other : bodies_) {
            if (&body != &other) {
                Eigen::Vector3d to_other = other.position - body.position;
                double distance = to_other.norm();
                
                if (distance > 0.0) {
                    double force_magnitude = G * other.mass / (distance * distance);
                    body.acceleration += to_other.normalized() * force_magnitude;
                }
            }
        }
    }
}

Eigen::Vector3d PhysicsEngine::calculate_tidal_forces(const Eigen::Vector3d& position) const {
    if (!black_hole_) return Eigen::Vector3d::Zero();
    
    const double solar_mass = 1.989e30;
    double bh_mass = black_hole_->get_parameters().mass * solar_mass;
    Eigen::Vector3d bh_pos = black_hole_->get_parameters().position;
    
    Eigen::Vector3d to_bh = bh_pos - position;
    double distance = to_bh.norm();
    
    if (distance == 0.0) return Eigen::Vector3d::Zero();
    
    // Tidal force gradient (simplified)
    double force_gradient = 2.0 * G * bh_mass / (distance * distance * distance);
    
    return to_bh.normalized() * force_gradient;
}
