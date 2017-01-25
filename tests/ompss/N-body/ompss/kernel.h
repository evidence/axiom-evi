#include "nbody.h"

void calculate_forces_part(FLOAT pos_x1, FLOAT pos_y1, FLOAT pos_z1, FLOAT mass1, FLOAT pos_x2, FLOAT pos_y2, FLOAT pos_z2,
      FLOAT mass2, float_ptr_t fx, float_ptr_t fy, float_ptr_t fz, const int safe);

void calculate_forces_BLOCK (force_ptr_t forces, particle_ptr_t block1, particle_ptr_t block2);

void calculate_forces ( force_block_t * forces, particles_block_t * block1, particles_block_t * block2, const int size );

void update_particle (particle_ptr_t part, const force_ptr_t force,  const FLOAT time_interval);

void update_particles (const int n_blocks, particles_block_t * block, force_block_t * forces, const FLOAT time_interval);
