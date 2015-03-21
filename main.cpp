#include <CL/cl.h>
#include <Magick++.h>

#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#define err(val) if(val != CL_SUCCESS) { std::cerr << "An OpenCL error occured (" << __FILE__ << ":" << __LINE__ << ")-> " << val << std::endl; exit(-1); }

std::string read_file_to_string(std::string filename) {
  std::string ret;
  std::ifstream stream(filename);
  std::string line;
  while(std::getline(stream, line)) {
    ret += line + "\n";
  }
  return ret;
}

int main(int argc, char** argv) {
  std::cout << "Hello, world!" << std::endl;
  std::cout << "Quantum size is: " << sizeof(Magick::Quantum)  << " bytes" << std::endl;

  const size_t max_platforms = 8;
  cl_platform_id platforms[max_platforms];
  cl_uint num_platforms;
  err(clGetPlatformIDs(max_platforms, platforms, &num_platforms));

  cl_device_id device;
  cl_uint num_device;
  err(clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 1, &device, &num_device));

  cl_int error;
  cl_context ctx = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &error);
  err(error);

  std::string kernel_source = read_file_to_string("kernels/raytrace.cl");

  const char* kernel_cstr = kernel_source.c_str();
  cl_program program = clCreateProgramWithSource(ctx, 1, &kernel_cstr, nullptr, &error);
  err(error);

  error = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
  if (error == CL_BUILD_PROGRAM_FAILURE) {
    size_t log_size;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
    char *log = (char *) malloc(log_size);
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, nullptr);
    printf("%s\n", log);
  } else {
    err(error);
  }

  cl_kernel kernel = clCreateKernel(program, "render", &error);
  err(error);

  cl_command_queue queue = clCreateCommandQueue(ctx, device, 0, &error);
  err(error);

  const cl_image_format format = { CL_RGBA, CL_UNORM_INT16 };
  const cl_image_desc desc = { CL_MEM_OBJECT_IMAGE2D, 480, 480, 0, 1 };
  cl_mem output_image = clCreateImage(ctx, CL_MEM_WRITE_ONLY, &format, &desc, nullptr, &error);
  err(error);

  err(clSetKernelArg(kernel, 0, sizeof(cl_mem), &output_image));

  size_t global_work_size = 480 * 480;
  err(clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_work_size, nullptr, 0, nullptr, nullptr));

  uint16_t data[480 * 480 * 4] = { 0 };
  const size_t origin[] = { 0, 0, 0 };
  const size_t size[] = { 480, 480, 1 };
  err(clEnqueueReadImage(queue, output_image, true, origin, size, 0, 0, data, 0, nullptr, nullptr));

  Magick::Image image("480x480", "white");
  for(size_t x = 0; x < 480; ++x) {
    for(size_t y = 0; y < 480; ++y) {
      size_t index = 4 * (y * 480 + x);
      image.pixelColor(x, y, Magick::Color(data[index], data[index + 1], data[index + 2]));
    }
  }
  image.write("out.png");
}
