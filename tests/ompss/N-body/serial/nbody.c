#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <ieee754.h>
#include <time.h>
#include "nbody.h"
#include "kernel.h"

int silent;

const static FLOAT gravitational_constant =  6.6726e-11; /* N(m/kg)2 */

void particle_init (nbody_conf_t * const conf, particles_block_t* const part)
{
   int i;
   for (i=0;i < BLOCK_SIZE;i++) {
      part->position_x[i] = conf->domain_size_x * ((FLOAT) random() / ((FLOAT)RAND_MAX + 1.0));
      part->position_y[i] = conf->domain_size_y * ((FLOAT) random() / ((FLOAT)RAND_MAX + 1.0));
      part->position_z[i] = conf->domain_size_z * ((FLOAT) random() / ((FLOAT)RAND_MAX + 1.0));
      part->mass[i]       = conf->mass_maximum  * ((FLOAT) random() / ((FLOAT)RAND_MAX + 1.0));
      part->weight[i]     = gravitational_constant * part->mass[i];

      printf ("%d: %lf %lf %lf  %lf\n", i, (double) part->position_x[i],
               (double) part->position_y[i], (double) part->position_z[i],
               (double) part->mass[i]);
   }
}

void nbody_generate_particles (nbody_conf_t * conf, nbody_file_t * file)
{
   int i;
   char fname[1024];
   sprintf(fname, "%s.in", file->name);

   if( access( fname, F_OK ) == 0 ) return;

   const int fd = open (fname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IRGRP | S_IROTH);
   assert(fd >= 0);

   assert(file->total_size % PAGE_SIZE == 0);
   assert(ftruncate(fd, file->total_size) == 0);
   printf ("File truncated total_size %d\n",  file->total_size);

   particles_block_t * const particles = mmap(NULL, file->total_size, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);

   const int total_num_particles = file->total_size / sizeof(particles_block_t);
   printf("Working with input file %s total_num_particles %d\n", fname,
           total_num_particles);
   for(i=0; i<total_num_particles; i++){
      particle_init(conf, particles+i);
   }

   assert(munmap(particles, file->total_size) == 0);
   close(fd);
}

int nbody_check (const nbody_t *nbody, const int timesteps)
{
   char fname[1024];
   sprintf(fname, "%s.ref", nbody->file.name);

   if ( access( fname, F_OK ) != 0 ) return 0;

   const int fd = open (fname, O_RDONLY, 0);
   assert(fd >= 0);

   particle_ptr_t particles = mmap(NULL, nbody->file.total_size, PROT_READ, MAP_SHARED, fd, nbody->file.offset);

   double error = 0.0;
   int i, e, count = 0;

   for(i=0; i< nbody->num_particles; i++){
#ifdef DEBUG
       printf ("Checking particles block %d num_particles %d fileoffset %d nbody->file.total_size %d\n", i,
             nbody->num_particles, nbody->file.offset, nbody->file.total_size);
#endif
      for (e=0;e < BLOCK_SIZE;e++){

         if((nbody->local[i].position_x[e]  != particles[i].position_x[e]) ||
               (nbody->local[i].position_y[e]  != particles[i].position_y[e]) ||
               (nbody->local[i].position_z[e] != particles[i].position_z[e])) {
            error += fabs(((nbody->local[i].position_x[e] - particles[i].position_x[e])*100.0)/particles[i].position_x[e]) +
               fabs(((nbody->local[i].position_y[e] - particles[i].position_y[e])*100.0)/particles[i].position_y[e]) +
               fabs(((nbody->local[i].position_z[e] - particles[i].position_z[e])*100.0)/particles[i].position_z[e]);
            printf ("%d: %lf %lf     %lf %lf    %lf %lf\n", 
                    e, (double) nbody->local[i].position_x[e],
                       (double) particles[i].position_x[e],
                       (double) nbody->local[i].position_y[e],
                       (double) particles[i].position_y[e],
                       (double) nbody->local[i].position_z[e],
                       (double) particles[i].position_z[e]);
            count++;
         }
      }
   }
   double relative_error = error/(3.0*count);

   if((count*100.0)/(nbody->num_particles*BLOCK_SIZE) > 0.6 || (relative_error >  0.000008)) {
      silent?:printf("> Relative error[%d]: %f\n", count, relative_error);
      return -1;
   } else {
      silent?:printf("> Result validation: OK\n");
      return 1;
   }
}



particles_block_t * nbody_load_particles (nbody_conf_t * conf, nbody_file_t * file)
{

   char fname[1024];
   sprintf(fname, "%s.in", file->name);

   const int fd = open (fname, O_RDONLY, 0);
   assert(fd >= 0);

   void * const ptr = mmap(NULL, file->size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, file->offset);
   assert(ptr != MAP_FAILED);

   assert(close(fd) == 0);

   return ptr;
}

void * nbody_alloc (const size_t size)
{
   void * const space = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
   assert(space != MAP_FAILED);
   return space;
}

force_ptr_t nbody_alloc_forces (nbody_conf_t * const conf)
{
   return nbody_alloc(conf->num_particles*sizeof(force_block_t));
}

particle_ptr_t nbody_alloc_particles (nbody_conf_t * const conf)
{
   return nbody_alloc(conf->num_particles*sizeof(particles_block_t));
}

nbody_file_t nbody_setup_file (nbody_conf_t * const conf)
{
#if 0
   int rank, rank_size;
   assert(MPI_Comm_size(MPI_COMM_WORLD, &rank_size) == MPI_SUCCESS);
   assert(MPI_Comm_rank(MPI_COMM_WORLD, &rank) == MPI_SUCCESS);
#else
   int rank = 0, rank_size = 1;
#endif

   nbody_file_t file;

   const int total_num_particles = conf->num_particles*rank_size;

   file.total_size = total_num_particles*sizeof(particles_block_t);
   file.size   = file.total_size/rank_size;
   file.offset = file.size*rank;

   sprintf(file.name, "%s-%d-%d-%d", conf->name, BLOCK_SIZE*total_num_particles, BLOCK_SIZE, conf->timesteps);

   return file;
}

nbody_t nbody_setup (nbody_conf_t * const conf)
{

   nbody_file_t file = nbody_setup_file(conf);

   if (file.offset == 0) nbody_generate_particles(conf, &file);

   printf("file offset %d\n", file.offset);

   nbody_t nbody = {
      nbody_load_particles(conf, &file),
      nbody_alloc_particles(conf),
      nbody_alloc_forces(conf),
      conf->num_particles,
      conf->timesteps,
      file
   };

   return nbody;
}


void nbody_save_particles(nbody_t * const nbody, const int timesteps)
{
   char fname[1024];
   sprintf(fname, "%s.out", nbody->file.name);

   const int fd = open (fname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
   assert(fd >= 0);

   write(fd, nbody->local, nbody->file.total_size);

   assert(close(fd) == 0);

}

#if 0
void nbody_save_particles2(particle_ptr_t const local, size_t total_size)
{
   char fname[1024];
   sprintf(fname, "%s.out", nbody->file.name);

   const int fd = open (fname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
   assert(fd >= 0);

   write(fd, local, total_size);

   assert(close(fd) == 0);
}
#endif
void nbody_free(nbody_t * const nbody)
{
   {
      const size_t size = nbody->num_particles*sizeof(particles_block_t);
      assert(munmap(nbody->local, size) == 0);
   }
   {
      const size_t size = nbody->num_particles*sizeof(force_block_t);
      assert(munmap(nbody->forces, size) == 0);
   }
}

double wall_time ( void )
{
   struct timespec ts;
   clock_gettime(CLOCK_MONOTONIC,&ts);
   return (double) (ts.tv_sec)  + (double) ts.tv_nsec * 1.0e-9;
}

void solve_nbody ( particles_block_t * __restrict__ particles, particles_block_t * __restrict__ tmp,
                   force_block_t * __restrict__ forces, const int n_blocks, const int timesteps, const FLOAT time_interval, const nbody_t * nbody )
{
   assert(timesteps > 0);

   int t;
   for(t = 0; t < timesteps; t++) {
   //   #pragma omp task in([n_blocks] particles) inout([n_blocks] forces)
      calculate_forces(forces, particles, particles, n_blocks);
      //#pragma omp task inout([n_blocks] particles) inout([n_blocks] forces)
      update_particles(n_blocks, particles, forces, time_interval);
#pragma omp taskwait 
      printf ("Saving particles timestep %d... ", t);
      nbody_save_particles(nbody, timesteps);
      printf ("done.\n");
#ifdef DEBUG
#endif
      printf (" %.1lf %.1lf %.1lf   ", forces[0].x[1], forces[0].y[1],
                          forces[0].z[1]);
      printf (" %.1lf %.1lf %.1lf    %.1lf %.1lf %.1lf\n", particles[0].velocity_x[1],
                                particles[0].velocity_y[1],
                                particles[0].velocity_z[1],
                                particles[0].position_x[1],
                                particles[0].position_y[1],
                                particles[0].position_z[1]);
   }

#pragma omp taskwait 
}


int main (int argc, char** argv)
{

   int num_particles   = argc <= 1 ? default_num_particles : roundup(atoi(argv[1]), MIN_PARTICLES);
   const int timesteps = argc <= 2 ? default_timesteps : atoi(argv[2]);
   silent = argc >= 4 ? atoi(argv[3]) : 0;

   assert(timesteps > 0);
   assert(num_particles >= 128);

   silent?:fprintf(stdout,"> Number of particles: %d\n", num_particles );
   silent?:fprintf(stdout,"> Time steps: %d\n", timesteps );

   num_particles = num_particles/BLOCK_SIZE;
   printf ("num_particles_blocks %d, BLOCK_SIZE %d\n", num_particles, BLOCK_SIZE);

   nbody_conf_t conf = { default_domain_size_x, default_domain_size_y, default_domain_size_z,
                         default_mass_maximum, default_time_interval, default_seed, default_name,
                         timesteps /* arg */, num_particles /* arg */ };

   nbody_t nbody = nbody_setup( &conf );

   const double start = wall_time();
   solve_nbody(nbody.local, nbody.remote, nbody.forces, num_particles, timesteps, conf.time_interval, &nbody);
   const double end = wall_time ();

   silent?:fprintf(stdout,"> Total execution time: %g s\n", end - start);

   nbody_save_particles(&nbody, timesteps);
   int result = nbody_check(&nbody, timesteps);
   nbody_free(&nbody);


   double tmp = (num_particles * BLOCK_SIZE);

   double throughput  = ( tmp * tmp * timesteps / (end-start) ) * ( 20 / 1.0E9 );

   const char * check[] = {"fail","n/a","pass"};

   int check_idx;

   if ( result <  0 ) check_idx = 0;
   if ( result == 0 ) check_idx = 1;
   if ( result >  0 ) check_idx = 2;

   // Print results
   if ( silent ) {
      printf( "%s (%s) with %d thread/s: %s\n", "N-Body", "OmpSs",omp_get_num_threads(), check[check_idx]);
   } else {
   printf( "================ N-BODY RESULTS ================== \n" );
   printf( ":Benchmark: %s (%s)\n", "N-Body", "OmpSs");
   printf( ":Total particles: %d\n", num_particles * BLOCK_SIZE );
   printf( ":Verification: %s\n", check[check_idx] );
   printf( ":Threads: %d\n", omp_get_num_threads());
   printf( ":Execution time (secs): %f\n", end-start);
   printf( ":Throughput (gflops): %f\t\n", throughput);
   printf( "================================================== \n" );
   }

   return 0;
}
