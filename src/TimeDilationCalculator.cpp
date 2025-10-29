#include "TimeDilationCalculator.h"

TimeDilationCalculator::TimeDilationCalculator() 
    : current_dilation_(1.0), proper_time_(0.0), coordinate_time_(0.0) {
    start_time_ = std::chrono::system_clock::now();
}

void TimeDilationCalculator::update(const Eigen::Vector3d& observer_position,
                                  const Eigen::Vector3d& black_hole_position,
                                  double black_hole_mass) {
    auto current_time = std::chrono::system_clock::now();
    double delta_time = std::chrono::duration<double>(current_time - start_time_).count();
    start_time_ = current_time;
    
    // Calculate distance to black hole
    Eigen::Vector3d to_black_hole = black_hole_position - observer_position;
    double distance = to_black_hole.norm();
    
    // Convert black hole mass to kg
    double M = black_hole_mass * solar_mass;
    double rs = 2.0 * G * M / (c * c); // Schwarzschild radius
    
    // Calculate time dilation (Schwarzschild metric)
    if (distance > rs) {
        current_dilation_ = std::sqrt(1.0 - rs / distance);
    } else {
        current_dilation_ = 0.0; // Inside event horizon
    }
    
    // Update times
    coordinate_time_ += delta_time;
    proper_time_ += delta_time * current_dilation_;
    
    // Record history
    TimeDilationRecord record;
    record.timestamp = current_time;
    record.position = observer_position;
    record.dilation_factor = current_dilation_;
    record.proper_time = proper_time_;
    record.coordinate_time = coordinate_time_;
    
    history_.push_back(record);
    
    // Keep only recent history
    if (history_.size() > 1000) {
        history_.erase(history_.begin());
    }
}

void TimeDilationCalculator::reset() {
    current_dilation_ = 1.0;
    proper_time_ = 0.0;
    coordinate_time_ = 0.0;
    history_.clear();
    start_time_ = std::chrono::system_clock::now();
}
