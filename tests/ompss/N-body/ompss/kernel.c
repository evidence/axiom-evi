#include <math.h>
#include <ieee754.h>
#include <stdbool.h>
#include <string.h>

#include "nbody.h"

const static FLOAT gravitational_constant =  6.6726e-11; /* N(m/kg)2 */


static void calculate_forces_part (
      FLOAT pos_x1, FLOAT pos_y1, FLOAT pos_z1, FLOAT mass1,
      FLOAT pos_x2, FLOAT pos_y2, FLOAT pos_z2, FLOAT mass2,
      float_ptr_t fx, float_ptr_t fy, float_ptr_t fz, const bool safe)
{
   const FLOAT diff_x = pos_x2 - pos_x1;
   const FLOAT diff_y = pos_y2 - pos_y1;
   const FLOAT diff_z = pos_z2 - pos_z1;

   const FLOAT distance_squared = diff_x * diff_x + diff_y * diff_y + diff_z * diff_z;

   const FLOAT distance = sqrt(distance_squared); // (Optimization) Intel compiler detects and emits rsqrt for this sqrt

   FLOAT force = ((mass1/(distance_squared*distance))*(mass2*gravitational_constant));  //correct as later we multiply by each distance in x, y, and z

   // This "if" MUST be after calculating force
   if (safe && distance_squared == 0.0f) force = 0.0f;

   *fx += force*diff_x;
   *fy += force*diff_y;
   *fz += force*diff_z;
}


static void calculate_forces_BLOCK (force_ptr_t forces, particle_ptr_t block1, particle_ptr_t block2)
{
   float_ptr_t x      = forces->x;
   float_ptr_t y      = forces->y;
   float_ptr_t z      = forces->z;

   float_ptr_t pos_x1 = block1->position_x;
   float_ptr_t pos_y1 = block1->position_y;
   float_ptr_t pos_z1 = block1->position_z;

   float_ptr_t mass1  = block1->mass ;

   float_ptr_t pos_x2 = block2->position_x;
   float_ptr_t pos_y2 = block2->position_y;
   float_ptr_t pos_z2 = block2->position_z;

   float_ptr_t mass2  = block2->mass;

   int i, j;
   for (i = 0; i < BLOCK_SIZE; i++) {
      FLOAT fx=x[i], fy=y[i], fz=z[i];  // correct adding forces from all part
      for (j = 0; j < BLOCK_SIZE; j++) {
         // (Optimization) If we are on not in the same block, do not use safe mode which checks if
         // p1 is the same particle than p2 and corrects NaN forces. This "if" with allows intel
         // compiler to remove branch "if (safe && distance_squared...)" at compile time in
         // calculate_forces_part whenever needed (Do not change it to a single call)
         calculate_forces_part(
               pos_x1[i], pos_y1[i], pos_z1[i], mass1[i],
               pos_x2[j], pos_y2[j], pos_z2[j], mass2[j],
               &fx, &fy, &fz, (block1 == block2) /*safe*/
               );
      }
      x[i]=fx;    	  
      y[i]=fy;       
      z[i]=fz;
   }
}


void calculate_forces ( force_block_t * forces, particles_block_t * block1, particles_block_t * block2, const int size )
{
   int i,j;
//printf ("calculate_forces size %d\n", size);
   for (i = 0; i < size; i++) {
      for (j = 0; j < size; j++) {
         #pragma omp target device(smp) copy_deps
         #pragma omp task inout( ([size] forces)[i;1] ) in( ([size] block1)[i;1], ([size] block2)[j;1] )
         {
             calculate_forces_BLOCK(forces+i, block1+i, block2+j);	
         }
      }
   }
//#pragma omp taskwait
}


void update_particle (particle_ptr_t part, const force_ptr_t force,  const FLOAT time_interval)
{
   int e;
	for (e=0; e < BLOCK_SIZE; e++) {
	   const FLOAT mass       = part->mass[e];
	   const FLOAT velocity_x = part->velocity_x[e];
	   const FLOAT velocity_y = part->velocity_y[e];
	   const FLOAT velocity_z = part->velocity_z[e];

	   const FLOAT position_x = part->position_x[e];
	   const FLOAT position_y = part->position_y[e];
	   const FLOAT position_z = part->position_z[e];

	   const FLOAT time_by_mass       = time_interval / mass;
	   const FLOAT half_time_interval = /*0.5f * */time_interval;

	   const FLOAT velocity_change_x = force->x[e] * time_by_mass;
	   const FLOAT velocity_change_y = force->y[e] * time_by_mass;
	   const FLOAT velocity_change_z = force->z[e] * time_by_mass;

	   const FLOAT position_change_x = (velocity_x + velocity_change_x) * half_time_interval;
	   const FLOAT position_change_y = (velocity_y + velocity_change_y) * half_time_interval;
	   const FLOAT position_change_z = (velocity_z + velocity_change_z) * half_time_interval;

	   part->velocity_x[e] = velocity_x + velocity_change_x;
	   part->velocity_y[e] = velocity_y + velocity_change_y;
	   part->velocity_z[e] = velocity_z + velocity_change_z;

	   part->position_x[e] = position_x + position_change_x;
	   part->position_y[e] = position_y + position_change_y;
	   part->position_z[e] = position_z + position_change_z;
   }
}


void update_particles (const int n_blocks, particles_block_t* __restrict__ block,
      force_block_t* __restrict__ forces, const FLOAT time_interval)
{
   int i;
//printf ("n_blocks %d\n", n_blocks);
   for (i = 0; i < n_blocks; i++) {
      #pragma omp target device(smp) copy_deps
      #pragma omp task inout( ([n_blocks] forces)[i;1], ([n_blocks] block)[i;1] )
      {
         update_particle(block+i, forces+i, time_interval);
         memset( forces+i, 0, sizeof(force_block_t));
      }
   }
   #pragma omp taskwait
}

