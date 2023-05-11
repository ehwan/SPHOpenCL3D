#include "engine.hpp"
#include <fstream>
#include "MC33.h"

engine_t engine;

// Marching-Cubes Density-Field Resolution
int X = 60;
int Y = 45;
int Z = 45;
// density field image buffer
cl::Buffer image_buffer;

int main()
{
  // Init Engine
  param_t param;
  param.minbound = { -1.0, -1.0, -1.0 };
  param.maxbound = { 3.0, 2.0, 2.0 };
  param.Cs = 10;
  param.gravity = { 0, -4.0, 0 };
  param.h = 0.08;
  param.eta = 2.5;
  param.mu = 0.02;
  param.gamma = 7.0;
  param.max_particle_count = 1800000;
  param.courant_dt_factor = 0.8;
  param.diffusion_dt_factor = 0.8;
  param.rho0 = 1;
  engine.set( param );
  engine.load_opencl();
  //engine.dt = 1.0/1500.0;


  // Adding Particles
  ehfloat static_gap = 1.1;
  for( ehfloat z=-static_gap*engine.H; z<1.0+static_gap*engine.H; z+=engine.gap )
  {
    for( ehfloat y=-static_gap*engine.H; y<1.0+static_gap*engine.H; y+=engine.gap )
    {
      for( ehfloat x=-static_gap*engine.H; x<2.0+static_gap*engine.H; x+=engine.gap )
      {
        if( x<0 || y<0 || z<0 || x>2 || y>1 || z>1 )
        {
          particle_info_t info;
          info.position = { x, y, z };
          info.velocity = { 0, 0, 0 };
          info.svelocity = { 0, 0, 0 };
          info.flag = EH_PARTICLE_STATIC|EH_PARTICLE_STATICMOVE|EH_PARTICLE_NOFORCE;
          info.color = 0;
          engine.add_particle( info );
        }else if( 
            x < 0.8
        )
        {
          particle_info_t info;
          info.position = { x, y, z };
          info.velocity = { 0, 0, 0 };
          info.svelocity = { 0, 0, 0 };
          info.flag = 0;
          info.color = 1;
          engine.add_particle( info );
        }
        /*
        else if( y < 0.1 )
        {
          particle_info_t info;
          info.position = { x, y, z };
          info.velocity = { 0, 0, 0 };
          info.svelocity = { 0, 0, 0 };
          info.flag = 0;
          info.color = 1;
          engine.add_particle( info );
        }
        */
      }
    }
  }

  engine.calculate_mass();
  

  // Kernels calculate density field
  cl::KernelFunctor<
    cl::Buffer&,
    cl::Buffer&,
    cl::Buffer&,
    cl::Buffer&,
    cl::Buffer&,
    cl::Buffer&,
    cl::Buffer&,
    cl_float3,
    cl_float3,
    cl_int,cl_int,cl_int
  > get_image_kernel( engine.program, "get_image" );
  image_buffer = cl::Buffer( engine.context, CL_MEM_READ_WRITE, sizeof(ehfloat)*X*Y*Z );

  ehfloat t = 0;
  int renderstep0 = 0.02/engine.dt;
  int renderstep = 0;
  std::ofstream file( "out.txt.nosync" );
  grid3d grid;
  std::vector<ehfloat> image( X*Y*Z );
  MC33 mc33;
  surface surf;
  while( t < 20 )
  {
    engine.step();
    t += engine.dt;

    if( renderstep == 0 )
    {
      renderstep = renderstep0;

      // print marching cubes
      int err;
      get_image_kernel(
          cl::EnqueueArgs(engine.queue,cl::NDRange(X,Y,Z)),
          engine.constant_buffer,
          engine.grid_particlecount,
          engine.position,
          engine.rho,
          engine.V,
          engine.flags,
          image_buffer,
          engine.minbound,
          engine.maxbound,
          //cl_float3{-0.1f,-0.1f,-0.1f},
          //cl_float3{2.1f,1.1f,1.1f},
          X, Y, Z, err ).wait();
      engine.check_kernel_error( err, "get_image_kernel" );
      engine.queue.enqueueReadBuffer( image_buffer, CL_TRUE, 0, sizeof(ehfloat)*X*Y*Z, image.data() );
      grid.set_data_pointer( X, Y, Z, image.data() );
      grid.set_ratio_aspect( 
          (engine.maxbound.s[0]-engine.minbound.s[0])/(float)X, 
          (engine.maxbound.s[1]-engine.minbound.s[1])/(float)Y, 
          (engine.maxbound.s[2]-engine.minbound.s[2])/(float)Z
      );
      //grid.set_ratio_aspect( 2.0/(float)X, 1.0/(float)Y, 1.0/(float)Z );
      grid.set_r0( engine.minbound.s[0], engine.minbound.s[1], engine.minbound.s[2] );
      mc33.set_grid3d( grid );
      mc33.calculate_isosurface( surf, 0.6 );

      int nverts = surf.get_num_vertices();
      const float *vs = surf.getVertex(0);
      const float *ns = surf.getNormal(0);
      int ntri = surf.get_num_triangles();
      const unsigned int *ts = surf.getTriangle(0);
      file.write( (char*)&t, sizeof(float) );
      file.write( (char*)&nverts, sizeof(int) );
      file.write( (char*)&ntri, sizeof(int) );
      file.write( (char*)vs, sizeof(float)*3*nverts );
      file.write( (char*)ns, sizeof(float)*3*nverts );
      file.write( (char*)ts, sizeof(unsigned int)*3*ntri );
      file.flush();
      std::cout << t << " " << engine.N << " " << nverts << " " << ntri << "\n";

      // print particle position & velocity
      /*
      auto p = engine.get_buffer<ehfloat3>( engine.position );
      auto v = engine.get_buffer<ehfloat3>( engine.velocity );
      auto flags = engine.get_buffer<cl_int>( engine.flags );
      auto nc = engine.get_buffer<cl_int>( engine.neighbor_count );
      decltype(p) pos, vel;
      int N = 0;
      for( int i=0; i<engine.N; ++i )
      {
        if( flags[i] == 0 )
        {
          ++N; pos.push_back(p[i]); vel.push_back(v[i]);
        }
      }

      std::cout << engine.N << " " << N << " " << t << "\n";

      file.write( (char*)&t, sizeof(ehfloat) );
      file.write( (char*)&N, sizeof(N) );
      file.write( (char*)pos.data(), sizeof(ehfloat3)*N );
      file.write( (char*)vel.data(), sizeof(ehfloat3)*N );
      */

    }
    --renderstep;
  }

  engine.step();
}
