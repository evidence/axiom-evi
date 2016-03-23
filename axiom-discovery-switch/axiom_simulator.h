/*
 * axiom_simulator.h
 *
 * Version:     v0.3
 * Last update: 2016-03-23
 *
 * This file contains AXIOM node network simulation structure
 * and prototypes
 *
 */
#include "axiom_discovery_protocol.h"

typedef struct node_args {  
    pthread_t tid;
    int num_interfaces;
    int connected_if[AXIOM_NUM_INTERFACES];
    int	node_fd;
    axiom_node_id_t node_id;
    axiom_node_id_t node_topology[AXIOM_NUM_NODES][AXIOM_NUM_INTERFACES];
    int local_routing[AXIOM_NUM_NODES][AXIOM_NUM_INTERFACES];
    axiom_if_id_t routing_tables[AXIOM_NUM_NODES][AXIOM_NUM_NODES];
    axiom_if_id_t final_routing_table[AXIOM_NUM_NODES];
} axiom_sim_node_args_t;

typedef struct topology {
    uint8_t topology[AXIOM_NUM_NODES][AXIOM_NUM_INTERFACES];
    int num_nodes;
    int num_interfaces;
} axiom_sim_topology_t;


void print_topology(axiom_sim_topology_t *tpl);
void print_local_routing_table(axiom_sim_node_args_t *nodes, int num_nodes);
int allocate_socket_node(uint8_t nodeA, axiom_sim_node_args_t *nodes, axiom_sim_topology_t *tpl);
int setup_nodes(axiom_sim_node_args_t *nodes, axiom_sim_topology_t *tpl);
//int allocate_link(uint8_t nodeA, uint8_t ifA, axiom_sim_node_args_t *nodes, axiom_sim_topology_t *tpl);
//int setup_links(axiom_sim_node_args_t *nodes, axiom_sim_topology_t *tpl);
int start_nodes(axiom_sim_node_args_t *nodes, int num_nodes, int master_node,
        void *(*master_body)(void *), void *(*slave_body)(void *));
void wait_nodes(axiom_sim_node_args_t *nodes, int num_nodes);
void set_node_info(axiom_sim_node_args_t * node_args);
axiom_sim_node_args_t *get_node_info();
