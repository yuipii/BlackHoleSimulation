#!/bin/bash

# Interstellar Black Hole Simulation Build Script
# Ubuntu/Debian build environment setup and compilation

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PROJECT_NAME="Interstellar Black Hole Simulation"
BUILD_DIR="build"
BINARY_NAME="interstellar_blackhole"
THREADS=$(nproc)  # Use all available CPU cores

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_dependencies() {
    print_status "Checking system dependencies..."
    
    # Check if we're on Ubuntu/Debian
    if ! command -v apt-get &> /dev/null; then
        print_error "This script is designed for Ubuntu/Debian systems"
        exit 1
    fi
    
    # Check if we have sudo privileges
    if [ "$EUID" -ne 0 ]; then
        SUDO="sudo"
    else
        SUDO=""
    fi
    
    # List of required packages
    REQUIRED_PACKAGES=(
        "build-essential"
        "cmake"
        "libglfw3-dev"
        "libglew-dev"
        "libeigen3-dev"
        "libopencv-dev"
        "libboost-all-dev"
        "libsfml-dev"
        "libgtest-dev"
        "libglm-dev"
        "pkg-config"
        "xorg-dev"
        "libx11-dev"
        "libxrandr-dev"
        "libxinerama-dev"
        "libxcursor-dev"
        "libxi-dev"
    )
    
    # Check which packages are missing
    MISSING_PACKAGES=()
    for package in "${REQUIRED_PACKAGES[@]}"; do
        if ! dpkg -l | grep -q " $package "; then
            MISSING_PACKAGES+=($package)
        fi
    done
    
    # Install missing packages
    if [ ${#MISSING_PACKAGES[@]} -ne 0 ]; then
        print_warning "The following packages are missing and will be installed:"
        printf ' - %s\n' "${MISSING_PACKAGES[@]}"
        echo
        
        read -p "Do you want to install these packages? (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            print_status "Updating package list..."
            $SUDO apt-get update
            
            print_status "Installing missing packages..."
            $SUDO apt-get install -y "${MISSING_PACKAGES[@]}"
            
            print_success "All dependencies installed successfully"
        else
            print_error "Cannot proceed without required dependencies"
            exit 1
        fi
    else
        print_success "All dependencies are already installed"
    fi
}

check_opengl() {
    print_status "Checking OpenGL support..."
    
    if ! glxinfo | grep -q "OpenGL version"; then
        print_warning "OpenGL may not be properly installed"
        print_status "Installing Mesa OpenGL drivers..."
        $SUDO apt-get install -y mesa-utils libgl1-mesa-dev
    fi
    
    # Print OpenGL information
    if command -v glxinfo &> /dev/null; then
        OPENGL_VERSION=$(glxinfo | grep "OpenGL version" | head -n1)
        print_success "OpenGL: $OPENGL_VERSION"
    else
        print_warning "glxinfo not available, but continuing anyway..."
    fi
}

setup_project() {
    print_status "Setting up project structure..."
    
    # Create build directory
    if [ ! -d "$BUILD_DIR" ]; then
        mkdir -p "$BUILD_DIR"
        print_success "Created build directory: $BUILD_DIR"
    else
        print_warning "Build directory already exists, cleaning..."
        rm -rf "$BUILD_DIR"/*
    fi
    
    # Check if essential source files exist
    ESSENTIAL_FILES=(
        "CMakeLists.txt"
        "src/main.cpp"
        "src/BlackHole.h"
        "src/BlackHole.cpp"
        "src/Renderer.h"
        "src/Renderer.cpp"
        "src/PhysicsEngine.h"
        "src/PhysicsEngine.cpp"
        "src/Camera.h"
        "src/Camera.cpp"
        "src/ShaderManager.h"
        "src/ShaderManager.cpp"
    )
    
    for file in "${ESSENTIAL_FILES[@]}"; do
        if [ ! -f "$file" ]; then
            print_error "Missing essential file: $file"
            exit 1
        fi
    done
    
    # Create shaders directory if it doesn't exist
    if [ ! -d "src/shaders" ]; then
        mkdir -p "src/shaders"
        print_status "Created shaders directory"
    fi
    
    # Check if shader files exist, create them if missing
    SHADER_FILES=(
        "src/shaders/blackhole.vert"
        "src/shaders/blackhole.frag"
        "src/shaders/accretion.vert"
        "src/shaders/accretion.frag"
    )
    
    for shader in "${SHADER_FILES[@]}"; do
        if [ ! -f "$shader" ]; then
            print_warning "Shader file missing: $shader"
            print_status "Creating placeholder shader file..."
            touch "$shader"
        fi
    done
    
    print_success "Project structure verified"
}

configure_cmake() {
    print_status "Configuring project with CMake..."
    
    cd "$BUILD_DIR"
    
    # Run CMake configuration
    if cmake .. -DCMAKE_BUILD_TYPE=Release; then
        print_success "CMake configuration successful"
    else
        print_error "CMake configuration failed"
        cd ..
        exit 1
    fi
    
    cd ..
}

compile_project() {
    print_status "Compiling project (using $THREADS threads)..."
    
    cd "$BUILD_DIR"
    
    # Compile the project
    if make -j$THREADS; then
        print_success "Compilation successful"
    else
        print_error "Compilation failed"
        cd ..
        exit 1
    fi
    
    cd ..
}

verify_binary() {
    print_status "Verifying compiled binary..."
    
    if [ -f "$BUILD_DIR/$BINARY_NAME" ]; then
        print_success "Binary created successfully: $BUILD_DIR/$BINARY_NAME"
        
        # Check if binary is executable
        if [ -x "$BUILD_DIR/$BINARY_NAME" ]; then
            print_success "Binary is executable"
        else
            print_warning "Binary is not executable, fixing permissions..."
            chmod +x "$BUILD_DIR/$BINARY_NAME"
        fi
        
        # Get binary size
        BINARY_SIZE=$(du -h "$BUILD_DIR/$BINARY_NAME" | cut -f1)
        print_status "Binary size: $BINARY_SIZE"
        
    else
        print_error "Binary not found: $BUILD_DIR/$BINARY_NAME"
        exit 1
    fi
}

run_tests() {
    print_status "Running basic tests..."
    
    # Check if test binary exists
    if [ -f "$BUILD_DIR/${BINARY_NAME}_tests" ]; then
        print_status "Running unit tests..."
        cd "$BUILD_DIR"
        ./${BINARY_NAME}_tests
        cd ..
    else
        print_warning "No test binary found, skipping tests"
    fi
}

create_run_script() {
    print_status "Creating run script..."
    
    cat > run_simulation.sh << 'EOF'
#!/bin/bash

# Interstellar Black Hole Simulation Runner

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
BINARY="$BUILD_DIR/interstellar_blackhole"

echo "================================================"
echo "   Interstellar Black Hole Simulation"
echo "================================================"

# Check if binary exists
if [ ! -f "$BINARY" ]; then
    echo "Error: Binary not found at $BINARY"
    echo "Please run ./build.sh first to compile the project"
    exit 1
fi

# Check if binary is executable
if [ ! -x "$BINARY" ]; then
    echo "Making binary executable..."
    chmod +x "$BINARY"
fi

# Run the simulation
echo "Starting simulation..."
echo "Controls:"
echo "  WASD - Move"
echo "  Q/E - Up/Down" 
echo "  Mouse - Look around"
echo "  Shift - Fast movement"
echo "  ESC - Exit"
echo "================================================"

cd "$SCRIPT_DIR"
"$BINARY"

EXIT_CODE=$?
if [ $EXIT_CODE -ne 0 ]; then
    echo "Simulation exited with error code: $EXIT_CODE"
    exit $EXIT_CODE
fi

EOF

    chmod +x run_simulation.sh
    print_success "Created run script: run_simulation.sh"
}

create_clean_script() {
    print_status "Creating cleanup script..."
    
    cat > clean_build.sh << 'EOF'
#!/bin/bash

# Clean build directory

echo "Cleaning build directory..."

if [ -d "build" ]; then
    rm -rf build/*
    echo "Build directory cleaned"
else
    echo "Build directory doesn't exist"
fi

# Remove run scripts
if [ -f "run_simulation.sh" ]; then
    rm run_simulation.sh
    echo "Removed run_simulation.sh"
fi

if [ -f "clean_build.sh" ]; then
    rm clean_build.sh
    echo "Removed clean_build.sh"
fi

echo "Cleanup complete"

EOF

    chmod +x clean_build.sh
    print_success "Created cleanup script: clean_build.sh"
}

show_system_info() {
    print_status "System Information:"
    
    echo "OS: $(lsb_release -d | cut -f2)"
    echo "Kernel: $(uname -r)"
    echo "Architecture: $(uname -m)"
    
    if command -v nvidia-smi &> /dev/null; then
        GPU_INFO=$(nvidia-smi --query-gpu=name --format=csv,noheader | head -n1)
        echo "GPU: NVIDIA $GPU_INFO"
    elif command -v glxinfo &> /dev/null; then
        GPU_INFO=$(glxinfo | grep "OpenGL renderer" | cut -d: -f2 | sed 's/^ *//')
        echo "GPU: $GPU_INFO"
    else
        echo "GPU: Unknown (OpenGL drivers not detected)"
    fi
    
    MEMORY=$(free -h | grep Mem: | awk '{print $2}')
    echo "Memory: $MEMORY"
    echo "CPU Cores: $THREADS"
}

main() {
    echo "================================================"
    echo "   $PROJECT_NAME"
    echo "   Build System for Ubuntu/Debian"
    echo "================================================"
    echo
    
    # Show system information
    show_system_info
    echo
    
    # Execute build steps
    check_dependencies
    echo
    check_opengl
    echo
    setup_project
    echo
    configure_cmake
    echo
    compile_project
    echo
    verify_binary
    echo
    create_run_script
    echo
    create_clean_script
    echo
    
    # Final message
    print_success "Build completed successfully!"
    echo
    echo "Next steps:"
    echo "  Run the simulation: ./run_simulation.sh"
    echo "  Clean build files: ./clean_build.sh"
    echo
    echo "Enjoy your journey to Gargantua!"
    echo "================================================"
}

# Handle command line arguments
case "${1:-}" in
    --help|-h)
        echo "Usage: ./build.sh [OPTIONS]"
        echo
        echo "Options:"
        echo "  -h, --help     Show this help message"
        echo "  -c, --clean    Clean build directory only"
        echo "  -r, --run      Build and run immediately"
        echo "  -t, --tests    Build and run tests"
        echo
        exit 0
        ;;
    --clean|-c)
        if [ -d "$BUILD_DIR" ]; then
            rm -rf "$BUILD_DIR"
            echo "Build directory cleaned"
        else
            echo "Build directory doesn't exist"
        fi
        exit 0
        ;;
    --run|-r)
        main
        echo
        echo "Starting simulation immediately..."
        ./run_simulation.sh
        exit 0
        ;;
    --tests|-t)
        main
        echo
        run_tests
        exit 0
        ;;
    *)
        main
        exit 0
        ;;
esac
