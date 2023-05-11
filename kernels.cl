#include "flags.h"

struct constant_t {
  ehfloat3 minbound;
  ehfloat3 maxbound;
  ehfloat3 gravity;
  int3 gridsize;
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
  int N;
};

int3 gridindex3_from_p3( constant struct constant_t *c, ehfloat3 p )
{
  return convert_int3_rtn( (p-c->minbound)*c->gridinvH );
}
int gridindex_from_index3( constant struct constant_t *c, int3 i3 )
{
  return (i3.z*c->gridsize.y + i3.y)*c->gridsize.x + i3.x;
}

// poly6 kernel
// W = W0 * ( 1 - (r/h)^2 )^3
ehfloat kernel_function( ehfloat invh, ehfloat3 x )
{
  ehfloat q = max( 1.0 - dot(x,x)*invh*invh, 0.0 );
  return 315.0/(64.0*EH_PI) * invh*invh*invh * q*q*q;
}
ehfloat3 kernel_gradient( ehfloat invh, ehfloat3 x )
{
  ehfloat q = max( 1.0 - dot(x,x)*invh*invh, 0.0 );
  return -945.0/(32.0*EH_PI) *invh*invh*invh*invh*invh * q*q * x;
}

kernel void assume_grid_count(
  constant struct constant_t *c,
  global int *gridcount,
  global int *grid_localindex,
  global const ehfloat3 *position,
  global int *gridindex
  )
{
  const int id = get_global_id(0);
  if( id >= c->N ){ return; }

  int3 index3 = gridindex3_from_p3( c, position[id] );
  int index1 = gridindex_from_index3( c, index3 );
  if( any(index3 < (int3)(0)) || any(index3 >= c->gridsize) )
  {
    index1 = c->gridsize.x*c->gridsize.y*c->gridsize.z;
  }
  gridindex[id] = index1;

  grid_localindex[id] = atomic_inc( gridcount+index1 );
}
kernel void prefix_sum_phase1(
  global int *A,
  global int *B,
  int N,
  int D
)
{
  int i = get_global_id(0);
  int x = 0;
  while( i <= N )
  {
    x += A[i];
    B[i] = x;
    i += D;
  }
}
kernel void prefix_sum_phase2(
  global int *A,
  global int *B,
  int N,
  int D
)
{
  int i = get_global_id(0);
  int x = 0;
  for( int j=max(0,i-D); j<i; ++j )
  {
    x += B[j];
  }
  A[i] = x;
}
kernel void move_to_new_grid(
  constant struct constant_t *c,
  global const int *grid_beginpoint,
  global const int *grid_localindex,
  global const int *gridindex,
  global const int *A,
  global int *newA,
  int type
)
{
  const int id = get_global_id(0);
  if( id >= c->N ){ return; }

  int to_id = grid_beginpoint[gridindex[id]] + grid_localindex[id];
  if( type == 0 )
  {
    newA[to_id] = A[id];
  }else if( type == 1 )
  {
    global ehfloat *newA_ = (global ehfloat*)newA;
    global const ehfloat *A_ = (global const ehfloat*)A;
    newA_[to_id] = A_[id];
  }else if( type == 2 )
  {
    global ehfloat2 *newA_ = (global ehfloat2*)newA;
    global const ehfloat2 *A_ = (global const ehfloat2*)A;
    newA_[to_id] = A_[id];
  }else if( type == 3 )
  {
    global ehfloat3 *newA_ = (global ehfloat3*)newA;
    global const ehfloat3 *A_ = (global const ehfloat3*)A;
    newA_[to_id] = A_[id];
  }
}
kernel void assume_neighbor_count(
  constant struct constant_t *c,
  global const int *grid_beginpoint,
  global const ehfloat3 *position,
  global const int *flags,
  global int *neighbor_count
)
{
  const int id = get_global_id(0);
  if( id >= c->N ){ return; }

  int count = 0;
  int3 index3 = gridindex3_from_p3( c, position[id] );
  int3 mingrid = max( index3-1, 0 );
  int3 maxgrid = min( index3+1, c->gridsize-1 );
  for( int gridz=mingrid.z; gridz<=maxgrid.z; ++gridz )
  {
    for( int gridy=mingrid.y; gridy<=maxgrid.y; ++gridy )
    {
      int begin = grid_beginpoint[ gridindex_from_index3(c,(int3)(mingrid.x,gridy,gridz)) ];
      int end = grid_beginpoint[ gridindex_from_index3(c,(int3)(maxgrid.x,gridy,gridz)) + 1 ];
      for( int j=begin; j<end; ++j )
      {
        ehfloat3 rij = position[id] - position[j];
        if( dot(rij,rij) > c->gridH*c->gridH ){ continue; }
        ++count;
      }
    }
  }
  if( count > 100 )
  {
    //printf( "%d %d %f %f %f\n", id, count, position[id].x, position[id].y, position[id].z );
  }
  neighbor_count[id] = count;
}
kernel void make_neighborlist(
  constant struct constant_t *c,
  global const int *grid_beginpoint,
  global const ehfloat3 *position,
  global const int *flags,
  global const int *neighbor_begin,
  global int *neighbors
)
{
  const int id = get_global_id(0);
  if( id >= c->N ){ return; }

  int count = 0;
  int3 index3 = gridindex3_from_p3( c, position[id] );
  int3 mingrid = max( index3-1, 0 );
  int3 maxgrid = min( index3+1, c->gridsize-1 );
  for( int gridz=mingrid.z; gridz<=maxgrid.z; ++gridz )
  {
    for( int gridy=mingrid.y; gridy<=maxgrid.y; ++gridy )
    {
      int begin = grid_beginpoint[ gridindex_from_index3(c,(int3)(mingrid.x,gridy,gridz)) ];
      int end = grid_beginpoint[ gridindex_from_index3(c,(int3)(maxgrid.x,gridy,gridz)) + 1 ];
      for( int j=begin; j<end; ++j )
      {
        ehfloat3 rij = position[id] - position[j];
        if( dot(rij,rij) > c->gridH*c->gridH ){ continue; }
        neighbors[ neighbor_begin[id]+count ] = j;
        ++count;
      }
    }
  }
}


kernel void calculate_rho(
  constant struct constant_t *c,
  global const int *neighbor_begin,
  global const int *neighbors,
  global const ehfloat3 *position,
  global ehfloat *rho,
  global ehfloat *V,
  global const int *flags
)
{
  const int id = get_global_id(0);
  if( id >= c->N ){ return; }
  ehfloat density = 0;
  ehfloat numdensity = 0;
  for( int jj=neighbor_begin[id]; jj<neighbor_begin[id+1]; ++jj )
  {
    int j = neighbors[jj];
    ehfloat3 rij = position[id] - position[j];
    ehfloat k = kernel_function( c->invH, rij );
    if( flags[j] & EH_PARTICLE_STATIC )
    {
      density += STATIC_MASS*c->mass*k;
    }else
    {
      density += c->mass*k;
    }
    numdensity += k;
  }
  density = max(density,c->rho0);
  rho[id] = density;
  if( flags[id] & EH_PARTICLE_STATIC )
  {
    V[id] = STATIC_MASS/numdensity;
  }else
  {
    V[id] = 1.0/numdensity;
  }
}
ehfloat16 gradient_tensor(
  constant struct constant_t *c,
  global const int *neighbor_begin,
  global const int *neighbors,

  global const ehfloat3 *position,
  global const ehfloat *rho,
  global const ehfloat *V,
  global const int *flags,
  int id
)
{
    return (ehfloat16)(
              (ehfloat4)(1,0,0,0),
              (ehfloat4)(0,1,0,0),
              (ehfloat4)(0,0,1,0),
              (ehfloat4)(0) );

  ehfloat3 invB[3] = { (ehfloat3)(0), (ehfloat3)(0), (ehfloat3)(0) };
  for( int jj=neighbor_begin[id]; jj<neighbor_begin[id+1]; ++jj )
  {
    int j = neighbors[jj];
    ehfloat3 rij = position[id] - position[j];
    ehfloat3 kdV = kernel_gradient( c->invH, rij ) * V[j];
    invB[0] += rij.x*kdV;
    invB[1] += rij.y*kdV;
    invB[2] += rij.z*kdV;
  }
  ehfloat det = dot(invB[0],cross(invB[1],invB[2]));
  if( fabs(det) < GRADIENT_TENSOR_EPS )
  {
    return (ehfloat16)(
              (ehfloat4)(1,0,0,0),
              (ehfloat4)(0,1,0,0),
              (ehfloat4)(0,0,1,0),
              (ehfloat4)(0) );
  }
  det = 1.0/det;
  ehfloat3 c1 = -det*cross(invB[1],invB[2]);
  ehfloat3 c2 = -det*cross(invB[2],invB[0]);
  ehfloat3 c3 = -det*cross(invB[0],invB[1]);
  return (ehfloat16)(
      (ehfloat4)(c1,0),
      (ehfloat4)(c2,0),
      (ehfloat4)(c3,0),
      (ehfloat4)(0) );
}

kernel void calculate_nonpressure_force(
  constant struct constant_t *c,
  global const int *neighbor_begin,
  global const int *neighbors,

  global const ehfloat3 *position,
  global const ehfloat *rho,
  global const ehfloat3 *velocity,
  global const int *flags,
  global ehfloat3 *nonpressure_force,
  global const ehfloat *V
)
{
  const int id = get_global_id(0);
  if( id >= c->N ){ return; }
  if( flags[id] & EH_PARTICLE_STATIC ){ return; }

  ehfloat3 gradvx = (ehfloat3)(0,0,0);
  ehfloat3 gradvy = (ehfloat3)(0,0,0);
  ehfloat3 gradvz = (ehfloat3)(0,0,0);
  ehfloat16 B = gradient_tensor(c,neighbor_begin,neighbors,position,rho,V,flags,id);
  for( int jj=neighbor_begin[id]; jj<neighbor_begin[id+1]; ++jj )
  {
    int j = neighbors[jj];
    if( flags[j] & EH_PARTICLE_STATIC ){ continue; }
    ehfloat3 rij = position[id] - position[j];
    ehfloat3 kdV = kernel_gradient(c->invH,rij)*V[j];
    ehfloat3 BkdV = kdV.x*B.s012 + kdV.y*B.s456 + kdV.z*B.s89a;
    ehfloat3 vji = velocity[j] - velocity[id];
    gradvx += vji.x*BkdV;
    gradvy += vji.y*BkdV;
    gradvz += vji.z*BkdV;
  }

  ehfloat3 lapv = (ehfloat3)(0,0,0);
  for( int jj=neighbor_begin[id]; jj<neighbor_begin[id+1]; ++jj )
  {
    int j = neighbors[jj];
    if( j == id ){ continue; }
    if( flags[j] & EH_PARTICLE_STATIC ){ continue; }
    ehfloat3 eij = position[id] - position[j];
    ehfloat3 kdV = kernel_gradient(c->invH,eij)*V[j];
    ehfloat3 vij = velocity[id] - velocity[j];
    if( dot(eij,eij) < 1e-10 ){ continue; }
    ehfloat invlen = 1.0/length(eij);
    eij = normalize(eij);
    ehfloat3 edgu = (ehfloat3)( dot(gradvx,eij), dot(gradvy,eij), dot(gradvz,eij) );
    lapv += 2*(vij*invlen - edgu)*dot(eij,kdV);
  }
  nonpressure_force[id] =
    rho[id]*c->gravity + c->mu*lapv;
}

kernel void calculate_pressure(
  constant struct constant_t *c,
  global const ehfloat *rho,
  global const int *flags,
  global ehfloat *pressure
  )
{
  int id = get_global_id(0);
  if( id >= c->N ){ return; }
  pressure[id] = c->Cs*c->Cs*c->rho0/c->gamma * (pow( rho[id]/c->rho0, c->gamma ) - 1.0);
  if( flags[id] & EH_PARTICLE_STATIC )
  {
    pressure[id] = c->Cs*c->Cs*c->rho0/c->gamma * (pow( c->static_rho, c->gamma ) - 1.0);
  }
}
kernel void calculate_pressure_force(
  constant struct constant_t *c,
  global const int *neighbor_begin,
  global const int *neighbors,

  global const ehfloat3 *position,
  global const ehfloat *rho,
  global const ehfloat *pressure,
  global const int *flags,
  global ehfloat3 *pressure_force,
  global const ehfloat *V
)
{
  int id = get_global_id(0);
  if( id >= c->N ){ return; }
  if( flags[id] & EH_PARTICLE_STATIC ){ return; }

/*
  ehfloat16 B = gradient_tensor(c,neighbor_begin,neighbors,position,rho,V,flags,id);
  ehfloat3 force = (ehfloat3)(0,0,0);
  for( int jj=neighbor_begin[id]; jj<neighbor_begin[id+1]; ++jj )
  {
    int j = neighbors[jj];
    ehfloat3 rij = position[id] - position[j];
    ehfloat3 kdV = kernel_gradient(c->invH,rij)*V[j];
    ehfloat3 BkdV = kdV.x*B.s012 + kdV.y*B.s456 + kdV.z*B.s89a;
    force -= (pressure[j]-pressure[id])*BkdV;
  }
*/
  ehfloat3 accel = (ehfloat3)(0,0,0);
  for( int jj=neighbor_begin[id]; jj<neighbor_begin[id+1]; ++jj )
  {
    int j = neighbors[jj];
    ehfloat3 rij = position[id] - position[j];
    ehfloat3 acc = -kernel_gradient(c->invH,rij)*c->mass*( pressure[id]/(rho[id]*rho[id]) + pressure[j]/(rho[j]*rho[j]) );
    if( flags[j] & EH_PARTICLE_STATIC ){ acc *= STATIC_MASS; }

    accel += acc;
  }

  pressure_force[id] = accel * rho[id];
}

kernel void advect_phase1(
  constant struct constant_t *c,
  global const int *flags,
  global const ehfloat3 *svelocity,
  global ehfloat3 *position,
  global ehfloat3 *velocity,
  global const ehfloat *rho,
  global const ehfloat3 *nonpressure_force
)
{
  int id = get_global_id(0);
  if( id >= c->N ){ return; }
  if( flags[id] & EH_PARTICLE_STATIC ){ return; }
  ehfloat3 accel = nonpressure_force[id]/rho[id];
  position[id] += c->dt * velocity[id] + 0.5*c->dt*c->dt*accel;
  velocity[id] += c->dt*accel;
}
kernel void advect_phase2(
  constant struct constant_t *c,
  global const int *flags,
  global const ehfloat3 *svelocity,
  global ehfloat3 *position,
  global ehfloat3 *velocity,
  global const ehfloat *rho,
  global const ehfloat3 *pressure_force
)
{
  int id = get_global_id(0);
  if( id >= c->N ){ return; }
  if( flags[id] & EH_PARTICLE_STATIC ){ return; }
  const ehfloat3 accel = pressure_force[id]/rho[id];
  position[id] += 0.5*c->dt*c->dt*accel;
  velocity[id] += c->dt*accel;
}

ehfloat calculate_rho_at(
  constant struct constant_t *c,
  global const int *grid_beginpoint,

  global const ehfloat3 *position,
  global ehfloat *rho,
  global ehfloat *V,
  global const int *flags,
  ehfloat3 point,
  int except_flag
)
{
  int3 index3 = gridindex3_from_p3( c, point );
  int3 mingrid = max( index3-1, 0 );
  int3 maxgrid = min( index3+1, c->gridsize-1 );

  ehfloat density = 0;
  for( int gridz=mingrid.z; gridz<=maxgrid.z; ++gridz )
  {
    for( int gridy=mingrid.y; gridy<=maxgrid.y; ++gridy )
    {
      int begin = grid_beginpoint[ gridindex_from_index3(c,(int3)(mingrid.x,gridy,gridz)) ];
      int end = grid_beginpoint[ gridindex_from_index3(c,(int3)(maxgrid.x,gridy,gridz)) + 1 ];
      for( int j=begin; j<end; ++j )
      {
        ehfloat3 rij = point - position[j];
        if( dot(rij,rij) > c->H*c->H ){ continue; }
        if( flags[j] & except_flag ){ continue; }
        ehfloat k = kernel_function(c->invH,rij);
        density += k*c->mass;
      }
    }
  }
  return density;
}
kernel void get_image(
  constant struct constant_t *c,
  global const int *grid_beginpoint,
  global ehfloat3 *position,
  global const ehfloat *rho,
  global ehfloat *V,
  global const int *flags,
  global ehfloat *image,
  ehfloat3 r0,
  ehfloat3 r1,
  int X,
  int Y,
  int Z
)
{
  if( get_global_id(0) >= X || get_global_id(1) >= Y || get_global_id(2) >= Z ){ return; }

  const ehfloat3 gap = (r1-r0)/(ehfloat3)(X,Y,Z);
  ehfloat3 p0 = (ehfloat3)( get_global_id(0), get_global_id(1), get_global_id(2) );
  p0 *= gap;
  p0 += r0;

  ehfloat density = calculate_rho_at( c, grid_beginpoint, position, rho, V, flags, p0, EH_PARTICLE_STATIC );

  image[ get_global_id(2)*Y*X + get_global_id(1)*X + get_global_id(0) ] = density;
}
