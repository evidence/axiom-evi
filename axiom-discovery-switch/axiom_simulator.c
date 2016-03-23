/*
 * axiom_simulator.c
 *
 * Version:     v0.3
 * Last update: 2016-03-23
 *
 * This file implements AXIOM node network simulation
 *
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "axiom_simulator.h"
#include "axiom_route_compute.h"

#define AXSW_PORT_START         33300   /* first port to listen */

pthread_key_t node_info_key;      /* thread private data */


void print_topology(axiom_sim_topology_t *tpl)
{
    int i, j;

    printf("Node");
    for (i = 0; i < tpl->num_interfaces; i++) {
        printf("\tIF%d", i);
    }
    printf("\n");

    for (i = 0; i < tpl->num_nodes; i++) {
        printf("%d", i);
        for (j = 0; j < tpl->num_interfaces; j++) {
            printf("\t%u", tpl->topology[i][j]);
        }
        printf("\n");
    }
}

void print_local_routing_table(axiom_sim_node_args_t *nodes, int num_nodes)
{
    int index, i, j;

    for (index = 0; index < num_nodes; index++)
    {
        printf("\nNode %d LOCAL ROUTING TABLE\n", nodes[index].node_id);
        printf("Node");
        for (i = 0; i < nodes[index].num_interfaces; i++) {
            printf("\tIF%d", i);
        }
        printf("\n");

        for (i = 0; i < num_nodes; i++)
        {
            printf("%d", i);
            for (j = 0; j < nodes[index].num_interfaces; j++)
            {
                printf("\t%u", nodes[index].local_routing[i][j]);
            }
            printf("\n");
        }
    }
}

void print_routing_tables (axiom_if_id_t rt[][AXIOM_NUM_NODES])
{
    int i, j;
    int comma = 0;


    for (i = 0; i < AXIOM_NUM_NODES; i++)
    {
        printf("\nNode %d ROUTING TABLE\n", i);
        for (j = 0; j < AXIOM_NUM_NODES; j++) {
            printf("\tNode%d", j);
        }

        printf("\n");
        printf("IF");
        for (j = 0; j < AXIOM_NUM_NODES; j++)
        {
            printf ("\t(");
            if (rt[i][j] & AXIOM_IF_0)
            {
                printf ("%d", 0);
                comma = 1;
            }
            if (rt[i][j] & AXIOM_IF_1)
            {
                if (comma == 1)
                    printf(",");
                else
                    comma = 1;
                printf ("%d", 1);
            }
            if (rt[i][j] & AXIOM_IF_2)
            {
                if (comma == 1)
                    printf(",");
                else
                    comma = 1;
                printf ("%d", 2);
            }
            if (rt[i][j] & AXIOM_IF_3)
            {
                if (comma == 1)
                    printf(",");
                else
                    comma = 1;
                printf ("%d", 3);
            }
            comma = 0;
            printf (")");
        }
        printf("\n");
    }
}

print_received_routing_table(axiom_sim_node_args_t *nodes, int num_nodes)
{
	int index, j;
	int comma = 0;
	
	for (index = 0; index < num_nodes; index++)
    {
		printf("\nNode %d RECEIVED ROUTING TABLE\n", nodes[index].node_id);
        for (j = 0; j < AXIOM_NUM_NODES; j++) {
            printf("\tNode%d", j);
        }
		
		printf("\n");
        printf("IF");
        for (j = 0; j < num_nodes; j++)
        {
			printf ("\t(");
			if (nodes[index].final_routing_table[j] & AXIOM_IF_0)
			{
				printf ("%d", 0);
				comma = 1;
			}
			if (nodes[index].final_routing_table[j] & AXIOM_IF_1)
			{
				if (comma == 1)
					printf(",");
				else
					comma = 1;
				printf ("%d", 1);
			}
			if (nodes[index].final_routing_table[j] & AXIOM_IF_2)
			{
				if (comma == 1)
					printf(",");
				else
					comma = 1;
				printf ("%d", 2);
			}
			if (nodes[index].final_routing_table[j] & AXIOM_IF_3)
			{
				if (comma == 1)
					printf(",");
				else
					comma = 1;
				printf ("%d", 3);
			}
			comma = 0;
			printf (")");
        }
        printf("\n");
		
	}
	
}

int allocate_socket_node(uint8_t nodeA, axiom_sim_node_args_t *nodes, axiom_sim_topology_t *tpl)
{
    uint8_t found_flag = 0;
    int i, socket_d, ret;
    struct sockaddr_in addr;

    for (i = 0; i < tpl->num_interfaces; i++) {
        if (tpl->topology[nodeA][i] != AXIOM_NULL_NODE) {
            nodes[nodeA].connected_if[i] = AXIOM_IF_CONNECTED;
            found_flag = 1;
        }
        else {
            nodes[nodeA].connected_if[i] = AXIOM_NULL_INTERFACE;
        }
    }
    if (found_flag == 0) {
        DPRINTF(" node %d disconnected from the network", nodeA);
        return -1;
    }

    /* Create a socket for nodeA */
    socket_d = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_d  < 0) {
        perror("opening stream socket ");
        return -1;
    }
    nodes[nodeA].node_fd = socket_d;
    DPRINTF("Node %d - Socket %d", nodeA, socket_d);

    /* Connect to the Virtual Machine ports */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
#if 0
    addr.sin_addr.s_addr = htonl(AXIOM_ETH_IP); // XXX: collegarsi all'IP della virtual machine
    addr.sin_port        = htons(AXSW_PORT_START + nodeA) ;
#endif
    //addr.sin_addr.s_addr = htonl(0xC0A80A7E); // XXX: collegarsi all'IP della virtual machine
    //addr.sin_port        = htons(AXSW_PORT_START);
    addr.sin_addr.s_addr = htonl(0xC0A80A86); // XXX: collegarsi all'IP della virtual machine
    addr.sin_port        = htons(AXSW_PORT_START + nodeA);

    ret = connect(socket_d, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("connect failed");
        return -1;
    }
    DPRINTF("Node %d connected", nodeA);

    return 0;
}

int setup_nodes(axiom_sim_node_args_t *nodes, axiom_sim_topology_t *tpl)
{
    int i, err;

    for (i = 0; i < tpl->num_nodes; i++) {
        err = allocate_socket_node(i, nodes, tpl);
        if (err) {
            return err;
        }
    }

    return 0;
}

#if 0
int allocate_link(uint8_t nodeA, uint8_t ifA, axiom_sim_node_args_t *nodes, axiom_sim_topology_t *tpl)
{
    uint8_t nodeB, ifB = AXIOM_NULL_NODE;
    int i, sockets[2];

    nodeB = tpl->topology[nodeA][ifA];

    /* The ifA of nodeA is not connected to any node */
    if (nodeB == AXIOM_NULL_NODE) {
        NDPRINTF("interface not conntected - nodeA=%d ifA=%d nodeB=%d ifB=%d",
                nodeA, ifA, nodeB, ifB);
        return 0;
    }

    /* Link already allocated */
    if (nodes[nodeA].node_if_fd[ifA] != 0) {
        NDPRINTF("link already allocated - nodeA=%d ifA=%d nodeB=%d ifB=%d",
                nodeA, ifA, nodeB, ifB);
        return 0;
    }

    /* Find the ifB connected to ifA */
    for (i = 0; i < tpl->num_interfaces; i++) {
        if (tpl->topology[nodeB][i] == nodeA) {
            ifB = i;
            break;
        }
    }

    if (ifB == AXIOM_NULL_INTERFACE) {
        NEPRINTF("link not found - nodeA=%d ifA=%d nodeB=%d ifB=%d",
                nodeA, ifA, nodeB, ifB);
        return -1;
    }

    /* Create a socket pair (link) between the interfaces */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
        perror("opening stream socket pair");
        return -1;
    }

    nodes[nodeA].node_if_fd[ifA] = sockets[0];
    nodes[nodeB].node_if_fd[ifB] = sockets[1];
    DPRINTF("Socket pair: [%d,%d]\n\r", sockets[0], sockets[1]);

    return 0;
}

int setup_links(axiom_sim_node_args_t *nodes, axiom_sim_topology_t *tpl)
{
    int i, j, err;

    for (i = 0; i < tpl->num_nodes; i++) {
       for (j = 0; j < tpl->num_interfaces; j++) {
            err = allocate_link(i, j, nodes, tpl);
            if (err) {
                return err;
            }
       }
    }

    return 0;
}
#endif

int start_nodes(axiom_sim_node_args_t *nodes, int num_nodes, int master_node,
        void *(*master_body)(void *), void *(*slave_body)(void *))
{
    int i, j, h, err;

    pthread_key_create(&node_info_key, NULL);

    for (i = 0; i < num_nodes; i++) {
        void *(*t_body)(void *);
        if (i != master_node) {
            t_body = slave_body;
        } else {
            t_body = master_body;
        }


        /* init the number of node interfaces */
        nodes[i].num_interfaces = AXIOM_NUM_INTERFACES;
        /* init the node local routing table */
        for (j = 0; j < num_nodes; j++)
        {
            for (h = 0; h < nodes[i].num_interfaces; h++)
            {
                nodes[i].local_routing[j][h] = 0;
            }
        }

        err = pthread_create(&nodes[i].tid, NULL, t_body, (void *)&nodes[i]);
        if (err) {
            EPRINTF("unable to create pthread err=%d", err);
            return err;
        }
    }

    return 0;
}

void wait_nodes(axiom_sim_node_args_t *nodes, int num_nodes)
{
    int i, err;

    for (i = 0; i < num_nodes; i++) {
        pthread_join(nodes[i].tid, NULL);
    }

    pthread_key_delete(node_info_key);
}


void set_node_info(axiom_sim_node_args_t *args)
{
    pthread_setspecific(node_info_key, (void *)args);
}

axiom_sim_node_args_t *get_node_info()
{
    return (axiom_sim_node_args_t *)pthread_getspecific(node_info_key);
}

