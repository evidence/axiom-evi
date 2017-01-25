#ifndef nbody_h
#define nbody_h

#include <stddef.h>

#if defined (_OPENMP)
#  include <omp.h>
#  define threads() omp_get_max_threads()
#else
#  define threads()             1
#  define omp_get_num_threads() 1
#  define omp_get_max_threads() 1
#  define omp_get_thread_num()  0
#endif

#define BLOCK_SIZE 128

#define FLOAT double

typedef FLOAT * __restrict__ const float_ptr_t; 

typedef struct {
   FLOAT position_x[BLOCK_SIZE]; /* m   */
   FLOAT position_y[BLOCK_SIZE]; /* m   */
   FLOAT position_z[BLOCK_SIZE]; /* m   */
   FLOAT velocity_x[BLOCK_SIZE]; /* m/s */
   FLOAT velocity_y[BLOCK_SIZE]; /* m/s */
   FLOAT velocity_z[BLOCK_SIZE]; /* m/s */
   FLOAT mass[BLOCK_SIZE];       /* kg  */
   FLOAT weight[BLOCK_SIZE];
} particles_block_t;

typedef particles_block_t * __restrict__ const particle_ptr_t;

typedef struct {
   FLOAT x[BLOCK_SIZE]; /* x */
   FLOAT y[BLOCK_SIZE]; /* y */
   FLOAT z[BLOCK_SIZE]; /* z */
} force_block_t;

typedef struct {
   FLOAT x; /* x */
   FLOAT y; /* y */
   FLOAT z; /* z */
} single_force;

typedef force_block_t * __restrict__ const force_ptr_t;

static inline int log2i (int i) {
	return 31-__builtin_clz(i);
}

//#define PART 1024
#define PAGE_SIZE 4096
#define MIN_PARTICLES (4096*BLOCK_SIZE/sizeof(particles_block_t))

#define roundup(x, y) (                                 \
{                                                       \
        const typeof(y) __y = y;                        \
        (((x) + (__y - 1)) / __y) * __y;                \
}                                                       \
)

static const FLOAT default_domain_size   = 1.0e+16; /* m */

static const FLOAT default_domain_size_x = 1.0e+16; /* m  */
static const FLOAT default_domain_size_y = 1.0e+16; /* m  */
static const FLOAT default_domain_size_z = 1.0e+16; /* m  */
static const FLOAT default_mass_maximum  = 1.0e+29; /* kg */
//static const FLOAT default_time_interval = 1.0e+0;  /* s  */
static const FLOAT default_time_interval = 3600.0e+0;  /* s  */
static const int   default_seed          = 12345;
static const char* default_name          = "particles";
static const int   default_num_particles = 8192;
static const int   default_timesteps     = 10;

typedef struct {
   size_t total_size;
   size_t size;
   size_t offset;
   char name[1000];
} nbody_file_t;

typedef const struct {
   FLOAT domain_size_x;
   FLOAT domain_size_y;
   FLOAT domain_size_z;
   FLOAT mass_maximum;
   FLOAT time_interval;
   int   seed;
   const char* name;
   const int timesteps;
   const int   num_particles;
} nbody_conf_t;

typedef const struct {
   particle_ptr_t const local;
   particle_ptr_t const remote;
   force_ptr_t    const forces;
   const int num_particles;
   const int timesteps;
   nbody_file_t file;
} nbody_t;


/* coomon.c */
nbody_t nbody_setup(nbody_conf_t * const conf);
void nbody_save_particles(nbody_t *nbody, const int timesteps);
void nbody_free(nbody_t *nbody);
int nbody_check(nbody_t *nbody, const int timesteps);

double wall_time ( void );

void print_stats( double n_blocks, int timesteps, double elapsed_time );

void exchange_particles(particles_block_t * const sendbuf, particles_block_t * recvbuf, const int n_blocks,
                        const int rank, const int rank_size, const int i);
void solve_nbody(particles_block_t * particles, particles_block_t * const tmp, force_block_t * const forces,
                 const int num_particles, const int timesteps, const FLOAT time_interval, const nbody_t * nbody);

#endif /* #ifndef nbody_h */
