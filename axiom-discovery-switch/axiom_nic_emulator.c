/*
 * axiom_nic_emulator.c
 *
 * Version:     v0.3
 * Last update: 2016-03-23
 *
 * This file contains the AXIOM NIC API simulation
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "axiom_nic_api_user.h"
#include "axiom_nic_packets.h"
#include "axiom_nic_discovery.h"
#include "axiom_nic_routing.h"
#include "axiom_simulator.h"

#include "axiom_switch_packets.h"

/* static function prototypes */
static axiom_msg_id_t
axiom_send_small_neighbour(axiom_dev_t *dev, axiom_if_id_t src_interface,
        axiom_port_t port, axiom_flag_t flag, axiom_payload_t *payload);

static axiom_msg_id_t
axiom_send_small_no_neighbour(axiom_dev_t *dev, axiom_if_id_t dest_node_id,
                              axiom_port_t port, axiom_flag_t flag,
                              axiom_payload_t *payload);
static axiom_msg_id_t
axiom_recv_small_neighbour(axiom_dev_t *dev, axiom_node_id_t *src_interface,
        axiom_port_t *port, axiom_flag_t *flag, axiom_payload_t *payload);

static axiom_msg_id_t
axiom_recv_small_no_neighbour(axiom_dev_t *dev, axiom_node_id_t *src_node_id,
        axiom_port_t *port, axiom_flag_t *flag, axiom_payload_t *payload);


/* Exported functions */

/*
 * @brief This function returns the local node id.
 * @param dev The axiom devive private data pointer
 * return Returns the local node id.
 */
axiom_node_id_t
axiom_get_node_id(axiom_dev_t *dev)
{
    return ((axiom_sim_node_args_t*)dev)->node_id;
}

/*
 * @brief This function sets the id of a local node.
 * @param dev The axiom devive private data pointer
 * @param node_id The _id assigned to this node.
 */
void
axiom_set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id)
{
    ((axiom_sim_node_args_t*)dev)->node_id = node_id;
}

/*
 * @brief This function reads the number of interfaces which are present on a
 * node.
 * @param dev The axiom devive private data pointer
 * @param if_number The number of the node interfaces
 * return Returns ...
 */
axiom_err_t
axiom_get_if_number(axiom_dev_t *dev, axiom_if_id_t* if_number)
{
    *if_number =  ((axiom_sim_node_args_t*)dev)->num_interfaces;

    return AXIOM_RET_OK; /* always return OK */
}

/*
 * @brief This function reads the features of an interface.
 * @param dev The axiom devive private data pointer
 * @param if_number The number of the node interface
 * @param if_features The features of the node interface
 * return Returns ...
 */
axiom_err_t
axiom_get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number, uint8_t* if_features)
{
    if (if_number >= AXIOM_NUM_INTERFACES)
    {
        return AXIOM_RET_ERROR;
    }

    *if_features = 0;

    if ( ((axiom_sim_node_args_t*)dev)->connected_if[if_number] != AXIOM_NULL_INTERFACE)
    {
        /* Exist a connection (a socket descriptor) on the 'if_number' interface of the
         *  node managed by the running thread */
        *if_features = AXIOM_IF_CONNECTED;
    }

    return AXIOM_RET_OK;
}

/*
 * @brief This function sets the routing table of a local node.
 * @param dev The axiom devive private data pointer
 * @param node_id Remote connected node id
 * @param enabled_mask bit mask interface
 */
axiom_err_t
axiom_set_routing(axiom_dev_t *dev, axiom_node_id_t node_id, uint8_t enabled_mask)
{
    uint8_t i;

    for (i = 0; i < AXIOM_NUM_INTERFACES; i++)
    {
        if (enabled_mask & (uint8_t)(1 << i))
        {
            ((axiom_sim_node_args_t*)dev)->local_routing[node_id][i] = 1;
        }
        else
        {
            ((axiom_sim_node_args_t*)dev)->local_routing[node_id][i] = 0;
        }
    }

    return AXIOM_RET_OK;
}

/*
 * @brief This function gets the routing table of a local node.
 * @param dev The axiom devive private data pointer
 * @param node_id Remote connected node id
 * @param enabled_mask bit mask interface
 */
axiom_err_t axiom_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id, uint8_t *enabled_mask)
{
    uint8_t i;

    *enabled_mask = 0;

    for (i = 0; i < AXIOM_NUM_INTERFACES; i++)
    {
        if (((axiom_sim_node_args_t*)dev)->local_routing[node_id][i] == 1)
        {
            *enabled_mask = (uint8_t)(1 << i);
        }
    }

    return AXIOM_RET_OK;
}


/*
 * @brief This function sends small data to a remote node.
 * @param dev The axiom devive private data pointer
 * @param dst_id The remote node id that will receive the small data or local
 *               interface that will send the small data
 * @param port port of the small message
 * @param flag flags of the small message
 * @param payload data to be sent
 * @return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
 */
axiom_msg_id_t
axiom_send_small(axiom_dev_t *dev, axiom_node_id_t dst_id,
        axiom_port_t port, axiom_flag_t flag, axiom_payload_t *payload)
{
    axiom_msg_id_t ret = AXIOM_RET_ERROR;

    if ((port == AXIOM_SMALL_PORT_DISCOVERY) && (flag == AXIOM_SMALL_FLAG_NEIGHBOUR))
    {
        ret = axiom_send_small_neighbour(dev, (axiom_if_id_t)dst_id,
                                   AXIOM_SMALL_PORT_DISCOVERY,
                                   AXIOM_SMALL_FLAG_NEIGHBOUR, payload);
    }
    if ((port == AXIOM_SMALL_PORT_ROUTING) && (flag == 0))
    {
        ret = axiom_send_small_no_neighbour(dev, dst_id,
                                            AXIOM_SMALL_PORT_DISCOVERY,
                                            0, payload);
    }

    return ret;
}

/*
 * @brief This function sends a raw message to a neighbour on a specific
 *        interface.
 * @param dev The axiom devive private data pointer
 * @param src_interface Sender interface identification
 * @param type type of the raw message
 * @param data Data to send
 * return Returns ...
 */
static axiom_msg_id_t
axiom_send_small_neighbour(axiom_dev_t *dev, axiom_if_id_t src_interface,
        axiom_port_t port, axiom_flag_t flag, axiom_payload_t *payload)
{
    axiom_small_msg_t message;
    axiom_small_eth_t small_eth;
    uint32_t axiom_msg_length = sizeof(axiom_small_eth_t);
    int ret;

    /* Header */
    message.header.tx.port_flag.port = port;
    message.header.tx.port_flag.flag = flag;
    message.header.tx.dst = src_interface;
    /* Payload */
    message.payload = *payload;


    /* ethernet packet management */
    ret = send(((axiom_sim_node_args_t*)dev)->node_fd, &axiom_msg_length, sizeof(axiom_msg_length), 0);
    if (ret != sizeof(axiom_msg_length)) {
        EPRINTF("axiom_msg_length send error - return: %d", ret);
        return AXIOM_RET_ERROR;
    }
    small_eth.eth_hdr.type = AXIOM_ETH_TYPE_SMALL;
    memcpy(&small_eth.small_msg, &message, sizeof(message));

    /* send the ethernet packet */
    ret = send( ((axiom_sim_node_args_t*)dev)->node_fd, &small_eth, axiom_msg_length, 0);

    if ((ret == -1) || (ret == 0))
    {
        return AXIOM_RET_ERROR;
    }
    return AXIOM_RET_OK;
}

/*
 * @brief This function sends a rwa message to a node
 * @param dev The axiom devive private data pointer
 * @param src_node_id Sender node identification
 * @param dest_node_id Recipient node identification
 * @param type type of the raw message
 * @param data Data to send
 * return Returns ...
 */
static axiom_msg_id_t
axiom_send_small_no_neighbour(axiom_dev_t *dev, axiom_if_id_t dest_node_id,
                              axiom_port_t port, axiom_flag_t flag,
                              axiom_payload_t *payload)
{
    axiom_small_msg_t message;
    axiom_small_eth_t small_eth;
    uint32_t axiom_msg_length = sizeof(axiom_small_eth_t);
    int ret;

    if (port == AXIOM_SMALL_PORT_DISCOVERY)
    {
        /* Header message */
        message.header.tx.port_flag.port = port;
        message.header.tx.port_flag.flag = flag;
        message.header.tx.dst = dest_node_id;
        /* Payload */
        message.payload= *payload;

        /* Ethernet packet management */
        ret = send(((axiom_sim_node_args_t*)dev)->node_fd, &axiom_msg_length, sizeof(axiom_msg_length), 0);
        if (ret != sizeof(axiom_msg_length)) {
            EPRINTF("axiom_msg_length send error - return: %d", ret);
            return AXIOM_RET_ERROR;
        }
        small_eth.eth_hdr.type = AXIOM_ETH_TYPE_SMALL;
        memcpy(&small_eth.small_msg, &message, sizeof(message));

        /* Send the ethernet packet */
        ret = send( ((axiom_sim_node_args_t*)dev)->node_fd, &small_eth, axiom_msg_length, 0);
        if ((ret == -1) || (ret == 0))
        {
            return AXIOM_RET_ERROR;
        }
        else
        {
            DPRINTF("routing: send on socket = %d to node %d: (%d,%d)",
               ((axiom_sim_node_args_t*)dev)->node_fd, message.header.tx.dst,
                (uint8_t)((message.payload & 0x00FF0000) >> 16), (uint8_t)((message.payload & 0x0000FF00) >> 8));
        }
    }

    return AXIOM_RET_OK;
}

/*
 * @brief This function receives small data.
 * @param dev The axiom devive private data pointer
 * @param src_id The source node id that sent the small data or local
 *               interface that received the small data
 * @param port port of the small message
 * @param flag flags of the small message
 * @param payload data received
 * @return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
 */
axiom_msg_id_t
axiom_recv_small(axiom_dev_t *dev, axiom_node_id_t *src_id,
        axiom_port_t *port, axiom_flag_t *flag, axiom_payload_t *payload)
{
    axiom_msg_id_t ret = AXIOM_RET_ERROR;

    if (*flag == AXIOM_SMALL_FLAG_NEIGHBOUR)
    {
        /* discovery and set routing neighbour messages management */
        ret = axiom_recv_small_neighbour(dev, src_id, port, flag, payload);
    }

    if ((*port == AXIOM_SMALL_PORT_ROUTING) && (*flag == 0))
    {
        /* delivery small messages management */
        ret = axiom_recv_small_no_neighbour(dev, src_id, port, flag, payload);
    }

    return ret;

}

/*
 * @brief This function receives small neighbour data.
 * @param dev The axiom devive private data pointer
 * @param src_id The source node id that sent the small data or local
 *               interface that received the small data
 * @param port port of the small message
 * @param flag flags of the small message
 * @param payload data received
 * @return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
 */
static axiom_msg_id_t
axiom_recv_small_neighbour(axiom_dev_t *dev, axiom_node_id_t *src_interface,
        axiom_port_t *port, axiom_flag_t *flag, axiom_payload_t *payload)
{
    axiom_small_eth_t small_eth;
    uint32_t axiom_msg_length;
    int ret;

    /* receive the length of the ethernet packet */
    ret = recv(((axiom_sim_node_args_t*)dev)->node_fd, &axiom_msg_length, sizeof(axiom_msg_length), MSG_WAITALL);
    if (ret < 0)
    {
        return AXIOM_RET_ERROR;
    }

    axiom_msg_length = ntohl(axiom_msg_length);
    if (axiom_msg_length > sizeof(small_eth))
    {
        EPRINTF("too long message - len: %d", axiom_msg_length);
        return AXIOM_RET_ERROR;
    }

    /* receive ethernet packet */
    ret = recv(((axiom_sim_node_args_t*)dev)->node_fd, &small_eth, axiom_msg_length, MSG_WAITALL);
    if (ret != axiom_msg_length)
    {
        EPRINTF("unexpected length - expected: %d received: %d", axiom_msg_length, ret);
        return AXIOM_RET_ERROR;
    }

    NDPRINTF("Receive from socket number = %d", ((axiom_sim_node_args_t*)dev)->node_fd);

    /* Header */
    //*src_interface = i; /* index of the interface from which I have received */
    *src_interface = small_eth.small_msg.header.rx.src; /* the switch puts the interface from which I have to receive */
    *port = small_eth.small_msg.header.rx.port_flag.port;
    *flag = small_eth.small_msg.header.rx.port_flag.flag;
    /* payload */
    *payload = small_eth.small_msg.payload;

    return AXIOM_RET_OK;
}

static axiom_msg_id_t
axiom_recv_small_no_neighbour(axiom_dev_t *dev, axiom_node_id_t *src_node_id,
        axiom_port_t *port, axiom_flag_t *flag, axiom_payload_t *payload)
{
    axiom_routing_payload_t rt_message;
    axiom_node_id_t my_id, node_id;
    axiom_if_id_t if_id;
    axiom_small_eth_t small_eth;
    uint32_t axiom_msg_length;
    int ret;

    my_id = axiom_get_node_id(dev);

    /* receive the length of the ethernet packet */
    ret = recv(((axiom_sim_node_args_t*)dev)->node_fd, &axiom_msg_length, sizeof(axiom_msg_length), MSG_WAITALL);
    if (ret < 0)
    {
        return AXIOM_RET_ERROR;
    }

    axiom_msg_length = ntohl(axiom_msg_length);
    if (axiom_msg_length > sizeof(small_eth))
    {
        EPRINTF("too long message - len: %d", axiom_msg_length);
        return AXIOM_RET_ERROR;
    }

    /* receive ethernet packet */
    ret = recv(((axiom_sim_node_args_t*)dev)->node_fd, &small_eth, axiom_msg_length, MSG_WAITALL);
    if (ret != axiom_msg_length)
    {
        EPRINTF("unexpected length - expected: %d received: %d", axiom_msg_length, ret);
        return AXIOM_RET_ERROR;
    }

    if (small_eth.small_msg.header.rx.port_flag.port == AXIOM_SMALL_PORT_ROUTING)
    {
        memcpy(&rt_message, &small_eth.small_msg.payload, sizeof(rt_message));

        *port = small_eth.small_msg.header.rx.port_flag.port;
        node_id = rt_message.node_id;
        if_id = rt_message.if_id;
        DPRINTF("routing: received on socket = %d for node %d: (%d,%d) [I'm node %d]",
                   ((axiom_sim_node_args_t*)dev)->node_fd, small_eth.small_msg.header.tx.dst,
                     node_id, if_id, my_id);

        if (rt_message.command ==  AXIOM_RT_CMD_INFO)
        {
            if (small_eth.small_msg.header.tx.dst == my_id)
            {
                /* it is my routing table, return the info to the caller */
                *src_node_id = small_eth.small_msg.header.tx.dst;
                *payload = small_eth.small_msg.payload;
                return AXIOM_RET_OK;
            }
            else
            {
                return AXIOM_RET_ERROR;
            }
        }
        else if (rt_message.command == AXIOM_RT_CMD_END_INFO)
        {
            if (small_eth.small_msg.header.tx.dst == my_id)
            {
                return AXIOM_RET_OK;
            }
            else
            {
                return AXIOM_RET_ERROR;
            }
        }
    }
}
