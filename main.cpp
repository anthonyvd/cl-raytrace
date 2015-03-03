#include <Magick++.h>
#include <iostream>
#include <CL/cl.h>
#include <array>
#include <string>

const char* kernel[] = {
  "void kernel render(global float* output) {", "\n",
  "  const float3 camera_position = { 0, 0, 0 };", "\n",
  "  const float3 film_origin = { 0, 0, 10 };", "\n",
  "  const float3 sphere_origin = { 0, 0, 30 };", "\n",
  "  const float radius = 5;", "\n",
  "  const float2 film_size = { 10, 10 };", "\n",
  "  const float2 film_resolution = { 480, 480 };", "\n",
  "  const float2 film_pixel_size = { film_size.x / film_resolution.x, film_size.y / film_resolution.y };", "\n",
  "  for(int x = 0; x < film_resolution.x; ++x) {", "\n",
  "    for(int y = 0; y < film_resolution.y; ++y) {", "\n",
  "      float3 offset = { film_size.x / 2, film_size.y / 2, 0 };", "\n",
  "      float3 pixel_offset = { x * film_pixel_size.x, y * film_pixel_size.y, 0 };", "\n",
  "      float3 film_intersection = film_origin - offset + pixel_offset;", "\n",
  "      float3 ray_direction = normalize(film_intersection - camera_position);", "\n",
  "      float3 v = ray_direction;", "\n",
  "      float3 u = sphere_origin - camera_position;", "\n",
  "      float3 puv = (dot(v, u) / length(v)) * v;",
  "      float3 projected = camera_position + puv;",
  "      float projected_distance_to_sphere_center = length(sphere_origin - projected);", "\n",
  "      uint index = y * film_resolution.x + x;", "\n",
  "      if(projected_distance_to_sphere_center <= radius)", "\n",
  "        output[index] = 1;", "\n",
  "      else", "\n",
  "        output[index] = 0;", "\n",
  "    }", "\n",
  "  }", "\n",
  "}", "\n",
};

#define err(val) if(val != CL_SUCCESS) { std::cerr << "An OpenCL error occured (" << __FILE__ << ":" << __LINE__ << ")-> " << val << std::endl; exit(-1); }

int main(int argc, char** argv) {
  std::cout << "Hello, world!" << std::endl;

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

  cl_program program = clCreateProgramWithSource(ctx, sizeof(kernel) / sizeof(const char*), kernel, nullptr, &error);
  err(error);

  error = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
  if (error == CL_BUILD_PROGRAM_FAILURE) {
    size_t log_size;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    char *log = (char *) malloc(log_size);
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
    printf("%s\n", log);
  } else {
    err(error);
  }

  cl_kernel kernel = clCreateKernel(program, "render", &error);
  err(error);

  cl_command_queue queue = clCreateCommandQueue(ctx, device, 0, &error);
  err(error);

  cl_mem output_buffer = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, 480 * 480 * sizeof(float), nullptr, &error);
  err(error);

  err(clSetKernelArg(kernel, 0, sizeof(cl_mem), &output_buffer));

  size_t global_work_size = 1;
  err(clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_work_size, nullptr, 0, nullptr, nullptr));

  float data[480 * 480] = { 0 };
  err(clEnqueueReadBuffer(queue, output_buffer, true, 0, 480 * 480 * sizeof(float), data, 0, nullptr, nullptr));

  Magick::Image image("480x480", "white");
  for(size_t x = 0; x < 480; ++x) {
    for(size_t y = 0; y < 480; ++y) {
      image.pixelColor(x, y, Magick::ColorGray(data[y * 480 + x]));
    }
  }
  image.write("out.png");
}
