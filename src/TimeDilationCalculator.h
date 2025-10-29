#ifndef TIMEDILATIONCALCULATOR_H
#define TIMEDILATIONCALCULATOR_H

#include <Eigen/Dense>
#include <vector>
#include <chrono>

struct TimeDilationRecord {
    std::chrono::system_clock::time_point timestamp;
    Eigen::Vector3d position;
    double dilation_factor;
    double proper_time;
    double coordinate_time;
};

class TimeDilationCalculator {
public:
    TimeDilationCalculator();
    
    void update(const Eigen::Vector3d& observer_position, 
                const Eigen::Vector3d& black_hole_position,
                double black_hole_mass);
    
    double get_dilation_factor() const { return current_dilation_; }
    double get_proper_time() const { return proper_time_; }
    double get_coordinate_time() const { return coordinate_time_; }
    
    const std::vector<TimeDilationRecord>& get_history() const { return history_; }
    
    void reset();
    
private:
    double current_dilation_;
    double proper_time_;
    double coordinate_time_;
    std::vector<TimeDilationRecord> history_;
    
    std::chrono::system_clock::time_point start_time_;
    
    const double G = 6.67430e-11;
    const double c = 299792458.0;
    const double solar_mass = 1.989e30;
};

#endif
