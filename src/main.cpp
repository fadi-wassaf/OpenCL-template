#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_HPP_ENABLE_EXCEPTIONS 
#include <CL/opencl.hpp>

#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>

double getEventTime(cl::Event &event){
    return (event.getProfilingInfo<CL_PROFILING_COMMAND_END>() - event.getProfilingInfo<CL_PROFILING_COMMAND_START>()) / 1e6;
}

int main(int argc, char* argv[]){

    // Program Mode 
    /*
        Program Arguments:
        [1] - Program mode: list_platforms, list_devices, compute (default mode is compute)
        [2] - the platform index used for list_devices, compute
        [3] - the device index used for compute
    */
    std::string mode = "compute";
    std::vector<std::string> modes = {"list_platforms", "list_devices", "compute"};
    int platform_id = 0, device_id = 0; // Default platform and device IDs
    
    if(argc >= 2){
        mode = argv[1];
        if(std::find(modes.begin(), modes.end(), mode) == modes.end()){
            std::cout << "Invalid mode (" << mode << ") specified. Valid Modes: list_platforms, list_devices, or compute.\n";
            exit(1);
        }
    } else {
        std::cout << "Not enough arguments supplied.\n";
        exit(1);
    }

    // Change default platform and device ids based on user input
    if(argc >= 3)
        platform_id = std::stoi(argv[2]);
    if(argc == 4)
        device_id = std::stoi(argv[3]);

    // Get the available OpenCL platforms
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if(platforms.size() == 0){
        std::cout << "No platforms found.\n";
        exit(1);
    }

    // If in the list_platforms mode, print out the available ones and exit
    if(mode == "list_platforms"){
        std::cout << "Available Platforms:\n";
        for(int i = 0; i < platforms.size(); i++)
            std::cout << "   [" << i << "] - " << platforms[i].getInfo<CL_PLATFORM_NAME>() << '\n';
        exit(0);
    }

    // Select the chosen platform based on the user input
    cl::Platform platform = platforms[platform_id];
    std::cout << "Selected Platform: [" << platform_id << "] - " << platform.getInfo<CL_PLATFORM_NAME>() << '\n';

    // Get the available devices on the selected platform
    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    if(devices.size() == 0){
        std::cout << "No devices found.\n";
        exit(1);
    }

    // If in the list_devices mode, print out the available ones along with some info and exit
    if(mode == "list_devices"){
        std::cout << "Available Devices:\n";
        for(int i = 0; i < devices.size(); i++){
            std::cout << "   [" << i << "] - " << devices[i].getInfo<CL_DEVICE_NAME>() << '\n';
            std::cout << "    | Device Type = " << devices[i].getInfo<CL_DEVICE_TYPE>() << '\n';
            std::cout << "    | Device Version = " << devices[i].getInfo<CL_DEVICE_VERSION>() << '\n';
            std::cout << "    | Max Work Group Size = " << devices[i].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << '\n';
        }
        exit(0);
    }

    // Select the chosen device on the previously chosen platform
    cl::Device device = devices[device_id];
    std::cout << "Selected Device: [" << device_id << "] - " << device.getInfo<CL_DEVICE_NAME>() << '\n';

    // Should be set properly based on need
    const int LOCAL_GROUP_SIZE = device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
    
    if (mode == "compute"){
        try {
            // Get source from .cl file
            std::ifstream source_file("../src/add.cl");
            std::string source_string( (std::istreambuf_iterator<char>(source_file)), (std::istreambuf_iterator<char>()));

            // Create a context for the chosen device
            cl::Context context(device);

            // Build the program from the kernel source file contents
            cl::Program program(context, source_string.c_str(), true);
            program.build(device); 

            // Initalize vectors for test adding example
            int LENGTH = LOCAL_GROUP_SIZE*4;
            std::vector<float> a(LENGTH);
            std::fill(std::begin(a), std::end(a), 1);
            std::vector<float> b(LENGTH);
            std::fill(std::begin(b), std::end(b), 2);
            std::vector<float> c(LENGTH);

            // Initialize the needed buffers
            cl::Buffer b_a(context, CL_MEM_WRITE_ONLY, LENGTH*sizeof(float));
            cl::Buffer b_b(context, CL_MEM_WRITE_ONLY, LENGTH*sizeof(float));
            cl::Buffer b_c(context, CL_MEM_READ_ONLY, LENGTH*sizeof(float));

            // Setup the kernel with its arguments
            cl::Kernel kernel(program, "add");
            kernel.setArg(0, b_a);
            kernel.setArg(1, b_b);
            kernel.setArg(2, b_c);

            // Initialize command queue
            cl::CommandQueue queue(context, cl::QueueProperties::Profiling);

            // Write data
            cl::Event writeA, writeB;
            queue.enqueueWriteBuffer(
                b_a, CL_TRUE,
                0, LENGTH * sizeof(float),
                a.data(), nullptr, &writeA
            );
            queue.enqueueWriteBuffer(
                b_b, CL_TRUE,
                0, LENGTH * sizeof(float),
                b.data(), nullptr, &writeB
            );

            // Excecute the kernel
            cl::Event executeKernel;
            queue.enqueueNDRangeKernel(
                kernel, cl::NullRange,
                cl::NDRange(LENGTH),
                cl::NDRange(LOCAL_GROUP_SIZE),
                nullptr, &executeKernel
            );

            // Read data
            cl::Event readC;
            queue.enqueueReadBuffer(
                b_c, CL_TRUE,
                0, LENGTH * sizeof(float),
                c.data(), nullptr, &readC
            );

            queue.finish();

            std::cout << "Write vector 'a' contents to 'b_a': " << getEventTime(writeA) << " ms\n";
            std::cout << "Write vector 'b' contents to 'b_b': " << getEventTime(writeB) << " ms\n";
            std::cout << "Add elements ('b_a' + 'b_b'): " << getEventTime(executeKernel) << " ms\n";
            std::cout << "Read 'b_c' contents to vector 'c': " << getEventTime(readC) << " ms\n";

        } catch (cl::BuildError &e) {
            for(const auto &log : e.getBuildLog()){
                std::cerr << "Build log for device " << log.first.getInfo<CL_DEVICE_NAME>().c_str() << '\n';
                std::cerr << log.second.c_str() << '\n';
            }
        } catch (cl::Error &e) {
            std::cerr << "OpenCL Error: " << e.what() << " returned " << e.err() << '\n';
        }
    } 

    return 0;
}
