/*
 * axiom_discovery_protocol_test.c
 *
 * Version:     v0.3
 * Last update: 2016-03-23
 *
 * This file tests the AXIOM INIT implementation
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "axiom_simulator.h"

axiom_sim_node_args_t axiom_nodes[AXIOM_NUM_NODES];

/* Matrix memorizing the topolgy of the network
 * (a row for each node, a column for each interface)
 - topology[id_node1][id_iface1] = id_node2, id_iface means that
   id_node1 node, on its id_iface_1 is connected with id_node12 node
*/
/* axiom_node_id_t topology[NUMBER_OF_NODES][AXIOM_NUM_INTERFACES]; */
#ifdef EXAMPLE1
axiom_sim_topology_t start_topology = {
    .topology = {
        { 1, 2, 3, AXIOM_NULL_NODE},
        { 0, 6, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 0, 3, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 0, 4, 2, AXIOM_NULL_NODE},
        { 3, 5, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 6, 4, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 1, 5, 7, AXIOM_NULL_NODE},
        { 6, AXIOM_NULL_NODE, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
    },
    .num_nodes = AXIOM_NUM_NODES,
    .num_interfaces = AXIOM_NUM_INTERFACES
};
#endif
#ifdef EXAMPLE2
axiom_sim_topology_t start_topology = {
    .topology = {
        { 1, 2, 3, AXIOM_NULL_NODE},
        { 0, 4, 8, AXIOM_NULL_NODE},
        { 5, 0, 4, AXIOM_NULL_NODE},
        { 0, AXIOM_NULL_NODE, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 2, 1, 5, AXIOM_NULL_NODE},
        { 2, 4, 6, 7},
        { 8, 5, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 5, 9, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 6, 1, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 7, AXIOM_NULL_NODE, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
    },
    .num_nodes = AXIOM_NUM_NODES,
    .num_interfaces = AXIOM_NUM_INTERFACES
};
#endif
#ifdef EXAMPLE3
axiom_sim_topology_t start_topology = {
    .topology = {
        { 1, 2, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 0, 3, 3, 4},
        { 0, 4, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 1, 1, 4, 5},
        { 3, 1, 2, AXIOM_NULL_NODE},
        { 3, AXIOM_NULL_NODE, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
    },
    .num_nodes = AXIOM_NUM_NODES,
    .num_interfaces = AXIOM_NUM_INTERFACES
};
#endif
#ifdef EXAMPLE4
axiom_sim_topology_t start_topology = {
    .topology = {
        { 1, 2, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 0, 4, 3, 3},
        { 4, 0, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 4, 1, 1, 5},
        { 3, 1, 2, AXIOM_NULL_NODE},
        { 3, AXIOM_NULL_NODE, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
    },
    .num_nodes = AXIOM_NUM_NODES,
    .num_interfaces = AXIOM_NUM_INTERFACES
};
#endif
#ifdef EXAMPLE5
axiom_sim_topology_t start_topology = {
    .topology = {
        { 1, 5, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 0, 2, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 1, 3, 4, AXIOM_NULL_NODE},
        { 2, AXIOM_NULL_NODE, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 2, 5, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 4, 0, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
    },
    .num_nodes = AXIOM_NUM_NODES,
    .num_interfaces = AXIOM_NUM_INTERFACES
};
#endif

axiom_sim_topology_t final_topology = {
    .num_nodes = AXIOM_NUM_NODES,
    .num_interfaces = AXIOM_NUM_INTERFACES
};

axiom_if_id_t all_routing_tables[AXIOM_NUM_NODES][AXIOM_NUM_NODES];

extern axiom_node_id_t topology[AXIOM_NUM_NODES][AXIOM_NUM_INTERFACES];

void *master(void *args)
{
    axiom_sim_node_args_t *axiom_args = (axiom_sim_node_args_t *) args;
    axiom_dev_t *p_node_info;
 #if 0
    int i;
    char buf[1024];
#endif

    set_node_info(axiom_args);
    sleep(1);
    NDPRINTF("axiom_args %p get_node_info %p", axiom_args, get_node_info());

    /* get the pointer to the running thread 'axiom_nodes' entry */
    p_node_info = (axiom_dev_t *)get_node_info();

    axiom_master_node_code(p_node_info, axiom_args->node_topology, axiom_args->routing_tables,
					axiom_args->final_routing_table);

    memcpy(final_topology.topology, axiom_args->node_topology, sizeof(axiom_args->node_topology));

    memcpy(all_routing_tables, axiom_args->routing_tables, sizeof(axiom_args->routing_tables));

    DPRINTF("END");

#if 0
    for (i = 0; i < axiom_args->num_interfaces; i++) {
        NDPRINTF("Master node [%lu] if%dfd = %d", axiom_args->tid, i, axiom_args->node_if_fd[i]);
        strncpy(buf, "Message from master", 20);
        if (axiom_args->node_if_fd[i] != 0)
            write(axiom_args->node_if_fd[i], buf, 20);
            DPRINTF("Master node [%lu] sent %s", axiom_args->tid, buf);
    }
#endif
}

void *slave(void *args)
{
    axiom_sim_node_args_t *axiom_args = (axiom_sim_node_args_t *) args;
    axiom_dev_t *p_node_info;
 #if 0
    int i;
    char buf[1024];
#endif
    set_node_info(axiom_args);
    sleep(1);
    NDPRINTF("axiom_args %p get_node_info %p", axiom_args, get_node_info());

    /* get the pointer to the running thread 'axiom_nodes' entry */
    p_node_info = (axiom_dev_t *)get_node_info();
    axiom_slave_node_code(p_node_info, axiom_args->node_topology, axiom_args->final_routing_table);

#if 0
    for (i = 0; i < axiom_args->num_interfaces; i++) {
        NDPRINTF("Slave node [%lu] if%dfd = %d", axiom_args->tid, i, axiom_args->node_if_fd[i]);
        if (axiom_args->node_if_fd[i] != 0) {
            read(axiom_args->node_if_fd[i], buf, 20);
            DPRINTF("Slave node [%lu] received %s", axiom_args->tid, buf);
        }
    }
#endif
}

int main(void *arg)
{
    int i, err;

    printf("Start Topology\n");
    print_topology(&start_topology);

    /* initilize links between nodes */
#if 0
    err = setup_links(axiom_nodes, &start_topology);
#endif
    err = setup_nodes(axiom_nodes, &start_topology);
    if (err) {
        return err;
    }

    /* start nodes threads */
    start_nodes(axiom_nodes, AXIOM_NUM_NODES, AXIOM_MASTER_ID, master, slave);


    /* wait nodes */
    wait_nodes(axiom_nodes, AXIOM_NUM_NODES);


    printf("\n************* Final Topology *******************\n");
    print_topology(&final_topology);
    
	/* print local routing table after discovery */
	printf("\n************* Local routing table *******************\n");
    print_local_routing_table(axiom_nodes, AXIOM_NUM_NODES);
    
	/* print all routing tables computed by Master*/
	printf("\n************* MASTER GLOBAL routing Table *******************\n");
    print_routing_tables(all_routing_tables);
	
	/* print each node received routing tables after delivery */
	printf("\n************* Slave received routing table *******************\n");
    print_received_routing_table(axiom_nodes, AXIOM_NUM_NODES);

	
    return 0;
}
