#pragma once
//#include "mymath.hpp"
#include <vector>
#include <functional>
#include <iostream>
#include "flags.h"

#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

#if defined(__APPLE__) || defined(__MACOSX)
#include "opencl.hpp"
#define USE_DOUBLE 0
#else
#include <CL/cl2.hpp>
#define USE_DOUBLE 1
#endif

#if USE_DOUBLE==1
using ehfloat = cl_double;
using ehfloat2 = cl_double2;
using ehfloat3 = cl_double3;
using ehfloat4 = cl_double4;
#else
using ehfloat = cl_float;
using ehfloat2 = cl_float2;
using ehfloat3 = cl_float3;
using ehfloat4 = cl_float4;
#endif

struct param_t
{
  int max_particle_count = 10000;
  // space
  ehfloat h = 0.1;
  ehfloat eta = 3;

  ehfloat static_rho = 1.3;

  ehfloat3 minbound = { 0,0,0 };
  ehfloat3 maxbound = { 1,1,1 };

  // mass, pressure
  ehfloat rho0 = 1;
  ehfloat Cs = 10;
  ehfloat gamma = 7;

  ehfloat mu = 0.05;

  ehfloat courant_dt_factor = 0.2;
  ehfloat diffusion_dt_factor = 0.2;

  ehfloat3 gravity = {0,0};
};

struct particle_info_t
{
  ehfloat3 position = {0,0};
  ehfloat3 velocity = {0,0};
  ehfloat3 svelocity = {0,0};
  cl_int flag = 0;
  cl_int color = 0;
};

struct engine_t
{
  struct constant_t {
    ehfloat3 minbound;
    ehfloat3 maxbound;
    ehfloat3 gravity;
    cl_int3 gridsize;
    ehfloat eta;
    ehfloat gap;
    ehfloat H;
    ehfloat invH;
    ehfloat gridH;
    ehfloat gridinvH;
    ehfloat mu;
    ehfloat mass;
    ehfloat dt;
    ehfloat Cs;
    ehfloat rho0;
    ehfloat gamma;
    ehfloat pressure0;
    ehfloat static_rho;
    cl_int N;
  };

  union {
    struct constant_t constants;
  struct {
    ehfloat3 minbound;
    ehfloat3 maxbound;
    ehfloat3 gravity;
    cl_int3 gridsize;
    ehfloat eta;
    ehfloat gap;
    ehfloat H;
    ehfloat invH;
    ehfloat gridH;
    ehfloat gridinvH;
    ehfloat mu;
    ehfloat mass;
    ehfloat dt;
    ehfloat Cs;
    ehfloat rho0;
    ehfloat gamma;
    ehfloat pressure0;
    ehfloat static_rho;
    cl_int N;
  };
  };
  int max_particle_count;
  int global_work_size;


  bool double_support;
  cl::Platform platform;
  cl::Device device;
  cl::Context context;
  cl::CommandQueue queue;

  cl::Buffer constant_buffer;

  // grid-base
  cl::Buffer grid_particlecount, grid_particlecount2;

  // retain values on grid_sort
  cl::Buffer position;
  cl::Buffer velocity;
  cl::Buffer svelocity;
  cl::Buffer flags;
  cl::Buffer float3_pong, int_pong;
  cl::Buffer pressure;
  cl::Buffer V;
  cl::Buffer rho;
  cl::Buffer color;
  cl::Buffer neighbors, neighbor_count;

  cl::Buffer nonpressure_force;
  cl::Buffer pressure_force;

  cl::Buffer gridindex;
  cl::Buffer grid_localindex;


  cl::Program program;

  struct
  {
    cl::KernelFunctor<
      cl::Buffer&,
      cl::Buffer&,
      cl_int,
      cl_int
    > prefix_sum_phase1{ cl::Kernel() };
    cl::KernelFunctor<
      cl::Buffer&,
      cl::Buffer&,
      cl_int,
      cl_int
    > prefix_sum_phase2{ cl::Kernel() };
    cl::KernelFunctor<
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&
    > assume_grid_count{ cl::Kernel() };
    cl::KernelFunctor<
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl_int
    > move_to_new_grid{ cl::Kernel() };

    cl::KernelFunctor<
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&
    > assume_neighbor_count{ cl::Kernel() };
    cl::KernelFunctor<
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&
    > make_neighborlist{ cl::Kernel() };

    cl::KernelFunctor<
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&
    > calculate_rho{ cl::Kernel() };

    cl::KernelFunctor<
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&
    > calculate_nonpressure_force{ cl::Kernel() };
    
    cl::KernelFunctor<
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&
    > calculate_pressure{ cl::Kernel() };
    cl::KernelFunctor<
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&
    > calculate_pressure_force{ cl::Kernel() };


    cl::KernelFunctor<
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&
    > advect_phase1{ cl::Kernel() };

    cl::KernelFunctor<
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&,
      cl::Buffer&
    > advect_phase2{ cl::Kernel() };


  } kernels;

#ifndef NDEBUG
  bool debug = true;
#else
  bool debug = false;
#endif

  engine_t(){}
  void load_opencl();
  void set( param_t &p );
  void log();
  void calculate_global_work_size()
  {
    int global_size_multiple = 16;
      //kernels.calculate_nonpressure.getKernel().getWorkGroupInfo<
      //CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE
      //>(device);
    global_work_size = (N/global_size_multiple);
    if( N % global_size_multiple ){ ++global_work_size; }
    global_work_size *= global_size_multiple;
  }

  void check_kernel_error( cl_int err, char const* str )
  {
    if( err != CL_SUCCESS )
    {
      std::cout << err << " : " << str << "\n";
      throw std::runtime_error( "kernel error" );
    }
  }

  struct {
    std::vector<ehfloat3> position;
    std::vector<ehfloat3> velocity;
    std::vector<ehfloat3> svelocity;
    std::vector<cl_int> flag;
    std::vector<cl_int> color;
    int max_list = 2048;
  } addparticle_waitlist;
  void add_waitlist()
  {
    if( addparticle_waitlist.position.size() == 0 ){ return; }
    if( N + addparticle_waitlist.position.size() > max_particle_count )
    {
      throw std::runtime_error( "particle count full error" );
    }
    int n = addparticle_waitlist.position.size();
    queue.enqueueWriteBuffer( position, CL_TRUE, sizeof(ehfloat3)*N, sizeof(ehfloat3)*n, addparticle_waitlist.position.data() );
    queue.enqueueWriteBuffer( velocity, CL_TRUE, sizeof(ehfloat3)*N, sizeof(ehfloat3)*n, addparticle_waitlist.velocity.data() );
    queue.enqueueWriteBuffer( svelocity, CL_TRUE, sizeof(ehfloat3)*N, sizeof(ehfloat3)*n, addparticle_waitlist.svelocity.data() );
    queue.enqueueWriteBuffer( flags, CL_TRUE, sizeof(cl_int)*N, sizeof(cl_int)*n, addparticle_waitlist.flag.data() );
    queue.enqueueWriteBuffer( color, CL_TRUE, sizeof(cl_int)*N, sizeof(cl_int)*n, addparticle_waitlist.color.data() );

    N += n;
    addparticle_waitlist.position.clear();
    addparticle_waitlist.velocity.clear();
    addparticle_waitlist.svelocity.clear();
    addparticle_waitlist.flag.clear();
    addparticle_waitlist.color.clear();
  }
  void add_particle( particle_info_t const& info )
  {
    addparticle_waitlist.position.push_back( info.position );
    addparticle_waitlist.velocity.push_back( info.velocity );
    addparticle_waitlist.svelocity.push_back( info.svelocity );
    addparticle_waitlist.flag.push_back( info.flag );
    addparticle_waitlist.color.push_back( info.color );

    if( addparticle_waitlist.position.size() == addparticle_waitlist.max_list )
    {
      add_waitlist();
    }
  }


  void upload_constants()
  {
    queue.enqueueWriteBuffer( constant_buffer, CL_TRUE, 0, sizeof(constant_t), &constants );
  }

  template < typename T >
  std::vector<T> get_buffer( cl::Buffer &buf )
  {
    std::vector<T> data( N );
    queue.enqueueReadBuffer(buf,CL_TRUE,0,sizeof(T)*N, data.data() );
    return data;
  }

  void prefix_sum( cl::Buffer &buf, int N );
  void grid_sort();
  void make_neighbors();
  void calculate_mass();
  void calculate_rho();
  void calculate_nonpressure_force();
  void calculate_pressure();
  void calculate_pressure_force();
  void advect_phase1();
  void advect_phase2();
  void advect();
  void step();
};
