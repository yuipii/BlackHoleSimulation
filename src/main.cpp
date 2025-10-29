#include "BlackHole.h"
#include "PhysicsEngine.h"
#include "Renderer.h"
#include "Camera.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <iomanip>

class Simulation {
public:
    Simulation() : camera_(std::make_unique<Camera>()) {}
    
    void run() {
        std::cout << "==================================================" << std::endl;
        std::cout << "    INTERSTELLAR BLACK HOLE SIMULATION" << std::endl;
        std::cout << "==================================================" << std::endl;
        
        // Инициализация рендерера
        renderer_ = std::make_unique<Renderer>(1920, 1080);
        if (!renderer_->initialize()) {
            std::cerr << "Failed to initialize renderer" << std::endl;
            return;
        }
        
        // Настройка сцены
        setup_scene();
        
        // Главный цикл
        main_loop();
        
        // Завершение работы
        cleanup();
    }
    
private:
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<Camera> camera_;
    std::shared_ptr<BlackHole> black_hole_;
    std::unique_ptr<PhysicsEngine> physics_engine_;
    
    void setup_scene() {
        // Создание черной дыры Гаргантюа
        BlackHoleParameters gargantua_params;
        gargantua_params.mass = 100000000.0;
        gargantua_params.spin = 0.6;
        gargantua_params.accretion_disk_inner_radius = 1.5;
        gargantua_params.accretion_disk_outer_radius = 40.0;
        gargantua_params.position = Eigen::Vector3d(0, 0, 0);
        
        black_hole_ = std::make_shared<BlackHole>(gargantua_params);
        
        // Инициализация физического движка
        physics_engine_ = std::make_unique<PhysicsEngine>();
        physics_engine_->set_black_hole(black_hole_);
        
        std::cout << "Scene setup complete" << std::endl;
    }
    
    void main_loop() {
        auto last_time = std::chrono::high_resolution_clock::now();
        int frame_count = 0;
        
        std::cout << "Starting main loop..." << std::endl;
        std::cout << "Controls: WASD - Move, Q/E - Up/Down, Mouse - Look, ESC - Exit" << std::endl;
        
        while (!glfwWindowShouldClose(renderer_->get_window())) {
            auto current_time = std::chrono::high_resolution_clock::now();
            double delta_time = std::chrono::duration<double>(current_time - last_time).count();
            last_time = current_time;
            
            // Проверка выхода
            if (glfwGetKey(renderer_->get_window(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                break;
            }
            
            // Обработка ввода
            camera_->handle_input(renderer_->get_window());
            
            // Рендеринг сцены
            renderer_->render(black_hole_, *physics_engine_);
            
            // Обновление счетчика кадров
            frame_count++;
            if (frame_count % 60 == 0) {
                double fps = 1.0 / delta_time;
                std::cout << "\rFPS: " << std::fixed << std::setprecision(1) << fps << std::flush;
            }
            
            // Небольшая задержка
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    void cleanup() {
        std::cout << "\nCleaning up..." << std::endl;
        if (renderer_) {
            renderer_->shutdown();
        }
        std::cout << "Simulation ended successfully" << std::endl;
    }
};

int main() {
    try {
        Simulation simulation;
        simulation.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal simulation error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
