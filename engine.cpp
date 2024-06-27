#include "engine.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

void engine_t::load_opencl()
{
  if (debug)
  {
    std::cout << "~~~~~OpenCL Initialize~~~~~~~\n";
  }
  int maxN = max_particle_count;
  platform = cl::Platform::get();

  {
    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    device = std::move(devices[0]);
  }

  context = cl::Context(device);
  std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();
  double_support
      = extensions.find(std::string("cl_khr_fp64")) != std::string::npos;
  if (debug)
  {
    std::cout << "Platform : " << platform.getInfo<CL_PLATFORM_NAME>() << "\n";
    std::cout << "Device : " << device.getInfo<CL_DEVICE_NAME>() << "\n";
    std::cout << "Max Compute Unit : "
              << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << "\n";
    // std::cout << extensions << "\n";
    std::cout << "double support : " << double_support << "\n";
  }

#if USE_DOUBLE == 1
  if (double_support == false)
  {
    std::cout << "double not supported but using ehfloat=double;\n";
    throw std::runtime_error("double not supported error");
  }

  if (debug)
  {
    std::cout << "using double precision;\n";
  }
#else
  if (debug)
  {
    std::cout << "using float precision;\n";
  }
#endif

  // buffers
  constant_buffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(constant_t));

  float3_pong = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(ehfloat3));
  int_pong = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(cl_int));
  position = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(ehfloat3));

  velocity = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(ehfloat3));

  svelocity = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(ehfloat3));

  flags = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(cl_int));

  rho = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(ehfloat));
  V = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(ehfloat));

  pressure = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(ehfloat));
  color = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(cl_int));

  nonpressure_force
      = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(ehfloat3));
  pressure_force
      = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(ehfloat3));

  grid_localindex
      = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(cl_int));
  gridindex = cl::Buffer(context, CL_MEM_READ_WRITE, maxN * sizeof(cl_int));

  int gs = gridsize.s[0] * gridsize.s[1] * gridsize.s[2];
  grid_particlecount
      = cl::Buffer(context, CL_MEM_READ_WRITE, (gs + 1) * sizeof(cl_int));
  grid_particlecount2
      = cl::Buffer(context, CL_MEM_READ_WRITE, (gs + 1) * sizeof(cl_int));

#define MAX_NEIGHBORS 200
  neighbors = cl::Buffer(context, CL_MEM_READ_WRITE,
                         maxN * MAX_NEIGHBORS * sizeof(cl_int));
  neighbor_count
      = cl::Buffer(context, CL_MEM_READ_WRITE, (maxN + 1) * sizeof(cl_int));

  queue = cl::CommandQueue(context, device);

  // loading sources
  std::ifstream file("kernels.cl");
  file.seekg(0, std::ifstream::end);
  size_t filesize = file.tellg();
  std::string source;
  source.resize(filesize);
  file.seekg(0, std::ifstream::beg);
  file.read(&source[0], filesize);

#if USE_DOUBLE == 1
  std::string typedef_ehfloat = "typedef double ehfloat;\n"
                                "typedef double2 ehfloat2;\n"
                                "typedef double3 ehfloat3;\n"
                                "typedef double4 ehfloat4;\n"
                                "typedef double8 ehfloat8;\n"
                                "typedef double16 ehfloat16;\n";
  std::string build_options = "-cl-std=CL1.2 -D EH_PI=M_PI";
#else
  std::string typedef_ehfloat = "typedef float ehfloat;\n"
                                "typedef float2 ehfloat2;\n"
                                "typedef float3 ehfloat3;\n"
                                "typedef float4 ehfloat4;\n"
                                "typedef float8 ehfloat8;\n"
                                "typedef float16 ehfloat16;\n";
  std::string build_options = "-cl-std=CL1.2 -D EH_PI=M_PI_F";
#endif

  cl::Program::Sources sources;
  sources.push_back({ typedef_ehfloat.c_str(), typedef_ehfloat.size() });
  sources.push_back({ source.c_str(), source.size() });

  program = cl::Program(context, sources);
  if (program.build({ device }, build_options.c_str()) != CL_SUCCESS)
  {
    std::cout << "error building kernel\n";
    std::cout << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>()[0].second << "\n";
    throw std::runtime_error("program building error");
  }

  kernels.assume_grid_count
      = decltype(kernels.assume_grid_count)(program, "assume_grid_count");
  kernels.move_to_new_grid
      = decltype(kernels.move_to_new_grid)(program, "move_to_new_grid");
  kernels.calculate_nonpressure_force
      = decltype(kernels.calculate_nonpressure_force)(
          program, "calculate_nonpressure_force");
  kernels.calculate_pressure
      = decltype(kernels.calculate_pressure)(program, "calculate_pressure");
  kernels.calculate_pressure_force = decltype(kernels.calculate_pressure_force)(
      program, "calculate_pressure_force");
  kernels.advect_phase1
      = decltype(kernels.advect_phase1)(program, "advect_phase1");
  kernels.advect_phase2
      = decltype(kernels.advect_phase2)(program, "advect_phase2");
  kernels.calculate_rho
      = decltype(kernels.calculate_rho)(program, "calculate_rho");

  kernels.assume_neighbor_count = decltype(kernels.assume_neighbor_count)(
      program, "assume_neighbor_count");
  kernels.make_neighborlist
      = decltype(kernels.make_neighborlist)(program, "make_neighborlist");

  kernels.prefix_sum_phase1
      = decltype(kernels.prefix_sum_phase1)(program, "prefix_sum_phase1");
  kernels.prefix_sum_phase2
      = decltype(kernels.prefix_sum_phase2)(program, "prefix_sum_phase2");

  upload_constants();
}
void engine_t::set(param_t& param)
{
  max_particle_count = param.max_particle_count;
  N = 0;
  H = param.h;
  invH = 1.0 / param.h;
  mu = param.mu;
  mass = 1;
  eta = param.eta;
  gap = H / eta;
  minbound = param.minbound;
  maxbound = param.maxbound;
  rho0 = param.rho0;
  Cs = param.Cs;
  gamma = param.gamma;
  static_rho = param.static_rho;

  ehfloat courant_dt = param.courant_dt_factor * gap / Cs;
  ehfloat diffusion_dt = 100000;
  if (mu > 0)
  {
    diffusion_dt = param.diffusion_dt_factor * gap * gap / mu;
  }
  dt = std::min(courant_dt, diffusion_dt);
  gridH = H * 1.1;
  gridinvH = 1.0 / gridH;
  gravity = param.gravity;
  pressure0 = Cs * Cs * rho0 / gamma;
  mass = rho0 * gap * gap * gap;

  for (int i = 0; i < 3; ++i)
  {
    gridsize.s[i] = (int)std::ceil((maxbound.s[i] - minbound.s[i]) * gridinvH);
  }
  if (debug)
  {
    std::cout << "maxparticle : " << max_particle_count << "\n";
    std::cout << "bound : (" << minbound.s[0] << ", " << minbound.s[1] << ", "
              << minbound.s[2] << ")x(" << maxbound.s[0] << ", "
              << maxbound.s[1] << ", " << maxbound.s[2] << ")\n";
    std::cout << "gridsize : (" << gridsize.s[0] << ", " << gridsize.s[1]
              << ", " << gridsize.s[2] << ")\n";
  }

  if (debug)
  {
    this->log();
  }
}
void engine_t::log()
{
  std::cout << "H : " << H << "\n";
  std::cout << "mu : " << mu << "\n";
  std::cout << "rho0 : " << rho0 << "\n";
  std::cout << "gamma : " << gamma << "\n";
  std::cout << "dt : " << dt << " , " << 1.0 / dt << "\n";
  std::cout << "Cs : " << Cs << "\n";
  std::cout << "pressure0 : " << Cs * Cs * rho0 / gamma << "\n";
  std::cout << "mass : " << mass << "\n";
}

void engine_t::prefix_sum(cl::Buffer& buf, int size)
{
  std::vector<cl_int> B(size + 1);
  queue.enqueueReadBuffer(buf, CL_TRUE, 0, sizeof(cl_int) * (size + 1),
                          B.data());
  int x = 0;
  for (int i = 0; i <= size; ++i)
  {
    int a = B[i];
    B[i] = x;
    x += a;
  }
  queue.enqueueWriteBuffer(buf, CL_TRUE, 0, sizeof(cl_int) * (size + 1),
                           B.data());
  /*
  cl_int err;
  kernels.prefix_sum_phase1(
    cl::EnqueueArgs(queue,cl::NDRange(32)),
    buf,
    out,
    size,
    32,
    err
  ).wait();
  check_kernel_error( err, "error prefix_sum_phase1" );

  kernels.prefix_sum_phase2(
    cl::EnqueueArgs(queue,cl::NDRange(size+1)),
    buf,
    out,
    size,
    32,
    err
  ).wait();
  check_kernel_error( err, "error prefix_sum_phase2" );
  */
}
void engine_t::grid_sort()
{
  cl::Event event;
  int gs = gridsize.s[0] * gridsize.s[1] * gridsize.s[2];
  queue.enqueueFillBuffer(grid_particlecount, cl_int(0), 0,
                          sizeof(cl_int) * (gs + 1), 0, &event);
  event.wait();

  cl_int err;
  kernels
      .assume_grid_count(cl::EnqueueArgs(queue, cl::NDRange(global_work_size)),
                         constant_buffer, grid_particlecount, grid_localindex,
                         position, gridindex, err)
      .wait();
  check_kernel_error(err, "error assume_grid_count");

  prefix_sum(grid_particlecount, gridsize.s[0] * gridsize.s[1] * gridsize.s[2]);

  // type 0 : int
  // type 1 : ehfloat
  // type 2 : ehfloat2
  // type 3 : ehfloat3
  auto move_to_new_grid = [&](cl::Buffer& A, cl::Buffer& newA, int type)
  {
    cl_int err;
    kernels.move_to_new_grid(
        cl::EnqueueArgs(queue, cl::NDRange(global_work_size)), constant_buffer,
        grid_particlecount, grid_localindex, gridindex, A, newA, type, err);
    check_kernel_error(err, "error move_to_new_grid");
    std::swap(A, newA);
  };
  move_to_new_grid(position, float3_pong, 3);
  move_to_new_grid(velocity, float3_pong, 3);
  move_to_new_grid(svelocity, float3_pong, 3);
  move_to_new_grid(flags, int_pong, 0);
  move_to_new_grid(color, int_pong, 0);

  queue.enqueueReadBuffer(grid_particlecount, CL_TRUE, sizeof(cl_int) * gs,
                          sizeof(cl_int), &N);

  upload_constants();
  calculate_global_work_size();
  queue.flush();
  queue.finish();
}
void engine_t::make_neighbors()
{
  cl_int err;
  kernels
      .assume_neighbor_count(
          cl::EnqueueArgs(queue, cl::NDRange(global_work_size)),
          constant_buffer, grid_particlecount, position, flags, neighbor_count,
          err)
      .wait();
  check_kernel_error(err, "error assume_neighbor_count");

  prefix_sum(neighbor_count, N);
  cl_int MN;
  queue.enqueueReadBuffer(neighbor_count, CL_TRUE, sizeof(cl_int) * N,
                          sizeof(cl_int), &MN);
  // std::cout << MN << "\n";
  if (MN > max_particle_count * MAX_NEIGHBORS)
  {
    throw std::runtime_error("too many neighbors...");
  }

  kernels
      .make_neighborlist(cl::EnqueueArgs(queue, cl::NDRange(global_work_size)),
                         constant_buffer, grid_particlecount, position, flags,
                         neighbor_count, neighbors, err)
      .wait();
  check_kernel_error(err, "error make_neighborlist");
}
void engine_t::calculate_rho()
{
  cl_int err;
  kernels
      .calculate_rho(cl::EnqueueArgs(queue, cl::NDRange(global_work_size)),
                     constant_buffer,
                     // grid_particlecount,
                     neighbor_count, neighbors, position, rho, V, flags, err)
      .wait();
  check_kernel_error(err, "error calculate_rho");
}
void engine_t::calculate_mass()
{
  add_waitlist();
  calculate_global_work_size();
  upload_constants();
  queue.flush();
  queue.finish();

  grid_sort();
  make_neighbors();
  calculate_rho();

  std::vector<ehfloat> rr(N);
  std::vector<int> ff(N);
  queue.enqueueReadBuffer(rho, CL_TRUE, 0, sizeof(ehfloat) * N, rr.data());
  queue.enqueueReadBuffer(flags, CL_TRUE, 0, sizeof(cl_int) * N, ff.data());
  ehfloat maxrho = 0;
  for (int i = 0; i < N; ++i)
  {
    if (ff[i] & EH_PARTICLE_STATIC)
    {
      continue;
    }
    maxrho = std::max(rr[i], maxrho);
  }
  mass *= rho0 / maxrho;
  std::cout << "Mass : " << mass << "\n";
}
void engine_t::calculate_pressure()
{
  cl_int err;
  kernels
      .calculate_pressure(cl::EnqueueArgs(queue, cl::NDRange(global_work_size)),
                          constant_buffer, rho, flags, pressure, err)
      .wait();
  check_kernel_error(err, "error calculate_pressure");
}
void engine_t::calculate_pressure_force()
{
  cl_int err;
  kernels
      .calculate_pressure_force(
          cl::EnqueueArgs(queue, cl::NDRange(global_work_size)),
          constant_buffer, neighbor_count, neighbors, position, rho, pressure,
          flags, pressure_force, V, err)
      .wait();
  check_kernel_error(err, "error calculate_pressure_force");
}
void engine_t::advect_phase1()
{
  cl_int err;
  kernels
      .advect_phase1(cl::EnqueueArgs(queue, cl::NDRange(global_work_size)),
                     constant_buffer, flags, svelocity, position, velocity, rho,
                     nonpressure_force, err)
      .wait();
  check_kernel_error(err, "error advect_phase1");
}
void engine_t::advect_phase2()
{
  cl_int err;
  kernels
      .advect_phase2(cl::EnqueueArgs(queue, cl::NDRange(global_work_size)),
                     constant_buffer, flags, svelocity,

                     position, velocity, rho, pressure_force, err)
      .wait();
  check_kernel_error(err, "error advect_phase2");
}
void engine_t::calculate_nonpressure_force()
{
  cl_int err;
  kernels
      .calculate_nonpressure_force(
          cl::EnqueueArgs(queue, cl::NDRange(global_work_size)),
          constant_buffer, neighbor_count, neighbors, position, rho, velocity,
          flags, nonpressure_force, V, err)
      .wait();
  check_kernel_error(err, "error calculate_nonpressure_force");
}
void engine_t::step()
{
  add_waitlist();
  calculate_global_work_size();
  upload_constants();
  queue.flush();
  queue.finish();

  grid_sort();
  make_neighbors();
  calculate_rho();
  calculate_nonpressure_force();
  advect_phase1();
  calculate_rho();
  calculate_pressure();
  calculate_pressure_force();
  advect_phase2();
}
