/*
 * axiom_node_code.c
 *
 * Version:     v0.2
 * Last update: 2016-03-22
 *
 * This file implements AXIOM NIC init phase of
 * Master and Slaves nodes
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "axiom_node_code.h"
#include "axiom_nic_discovery.h"
#include "axiom_route_compute.h"
#include "axiom_route_delivery.h"
#include "axiom_route_set.h"

/* Master node code */
void axiom_master_node_code(axiom_dev_t *dev, axiom_node_id_t topology[][AXIOM_NUM_INTERFACES],
                      axiom_if_id_t routing_tables[][AXIOM_NUM_NODES],
                      axiom_if_id_t final_routing_table[AXIOM_NUM_NODES])
{
    axiom_msg_id_t ret;

    /* Discovery phase: discover the global topology */
    axiom_master_node_discovery(dev, topology);

    /* compute each node routing table */
    axiom_compute_routing_tables(topology, routing_tables);

    /* copy its routing table */
    memcpy(final_routing_table, routing_tables[0], sizeof(axiom_if_id_t)*AXIOM_NUM_NODES);

    /* delivery of each node routing tables */
    ret = axiom_delivery_routing_tables(dev, routing_tables);
    if (ret == AXIOM_RET_ERROR)
    {
        EPRINTF("MASTER: axiom_delivery_routing_tables failed");
    }

    /* Say to all nodes to set the received routing table */
    sleep(2);
    ret = axiom_set_routing_table(dev, final_routing_table);
    if (ret == AXIOM_RET_ERROR)
    {
        EPRINTF("Master: axiom_set_routing_table failed");
    }

}

/* Slave node code*/
void axiom_slave_node_code(axiom_dev_t *dev, axiom_node_id_t topology[][AXIOM_NUM_INTERFACES],
                     axiom_if_id_t final_routing_table[AXIOM_NUM_NODES])
{
    axiom_node_id_t my_node_id;
    axiom_msg_id_t ret;

    /* Discovery phase: discover the intermediate topology */
    axiom_slave_node_discovery (dev, topology, &my_node_id);

    /* Receive my routing table */
    ret = axiom_receive_routing_tables(dev, my_node_id, final_routing_table);
    if (ret == AXIOM_RET_ERROR)
    {
        EPRINTF("Slave %d: axiom_receive_routing_tables failed", my_node_id);
    }

    /* Set my final routing table*/
    ret = axiom_set_routing_table(dev, final_routing_table);
    if (ret == AXIOM_RET_ERROR)
    {
        EPRINTF("Slave %d: axiom_set_routing_table failed", my_node_id);
    }
}
