# AXIOM Software Stack

----

**This repository contains the following git sub-modules where the Intellectual
Property (IP) is completly owned by Evidence SRL with the licence (GPLv2)
described in the COPYING file:**

 * **axiom-allocator**
    + Implementation of the 3-level AXIOM Memory Allocator
 * **axiom-evi-allocator-drv**
    + Implementation of the Axiom Memory Device Driver to handle virtual to physical memory mapping
 * **axiom-evi-allocator-lib**
    + Implementation of 3rd level software Memory Allocator based on LMM
 * **axiom-evi-apps**
    + Implementation of AXIOM application and daemons (axiom-init, axiom-run, etc.)
 * **axiom-evi-nic**
    + Implementation of AXIOM NIC device driver and User Space libraries

This repository contains also the following git sub-modules with their
respective licenses described in each sub-module:

 * axiom-evi-extrae
    + Modified version of Extrae to support IOCTL and AXIOM api
 * axiom-evi-gasnet
    + Modified version of GASNet that includes the new AXIOM conduit
 * axiom-evi-mcxx
    + Modified version of mcxx to support AXIOM GASNet conduit and cross-compilation
 * axiom-evi-nanox
    + Modified version of nanox to support AXIOM GASNet conduit and cross-compilation

## 1. Introduction

The main components of the AXIOM infrastructure are the following
(see the figure belowe):

* **The AXIOM NIC**:
    + Implemented in the FPGA by partner FORTH. The AXIOM NIC exports a set of
      registers to the computing part of the FPGA. These registers are described
      in the Datasheet, that can be found in the [[#Documentation]].
* **Axiom NIC Device driver**:
    + Implements the kernel-related part of the infrastructure, which is
      responsible for a proper handling of the AXIOM NIC registers, and which is
      responsible for providing abstractions for communication ports, an IOCTL
      interface to the user space, and for extending the HW buffering space in
      memory thanks to the usage of kernel threads.
* **Axiom User Library**:
    + The AXIOM User Library is responsible for providing a comprehensive C API,
      which can be used by the AXIOM applications to interact with the network
      interface in a simpler way.

<img src="https://github.com/evidence/axiom-evi-nic/raw/master/axiom_docs/documents/axiom_api.png" width="600px">

The main features of the AXIOM NIC are the following:

* **Interrupt moderation**:
    + The AXIOM NIC only generates interrupts when the queue status has changed.
* **Link status monitoring**:
    + The AXIOM NIC is able to provide information about the fact that a
      specific interface of the board is connected or not to another board.
* **Routing decision based on a Routing Table**:
    + The routing of messages in the AXIOM NIC is based on a store & forward
      technique, where each node maintains a routing table in the FPGA, holding
      the information about the interface to use to reach another node.
* **Multiple type of messages**:
    + The network interface is able to concurrently send two kind of messages,
      small and big messages.
* **Multiple queues available**:
    + The AXIOM NIC provides 2 set of queues for reasons of efficiency to
      separate message (one for small and one for big messages).

During the design of the network infrastructure, we took special care about
defining a complete system that could be efficiently mapped on the parallel
programming libraries running in user space. For this reason, we followed these
guidelines during the implementation of the AXIOM Drivers and during the design
of the AXIOM NIC registers:

* **Network management in user space**:
    + Compared to the standard TCP/IP Linux stack, we tried to move various
      features and network support in user space, limiting the AXIOM NIC driver
      to its main task of handling the delivery of the packets between the user
      space applications and the AXIOM NIC registers.
* **Possibility to support for memory mapped registers exposed to the user space**:
    + We left open the possibility to use memory mapped registers to avoid the
      use of IOCTL on each packet transfer, and to reduce memory copies. On the
      other hand, this requires the availability of separate per-process network
      queues to limit the overhead that is linked to mutual exclusion on the NIC
      register sets.
* **Possibility to use the NIC as tunnel for ethernet packets**:
    + The idea is that the long messages will be used as datagrams, giving the
      possibility for TCP/IP packet to be sent on top of the AXIOM NIC layers.
      This has the advantage to reduce cabling between the boards (only the
      AXIOM Link is needed), allowing legacy applications based on TCP/IP to run
      unmodified on top of the AXIOM-link network.

## 2. Download the code

### 2.1 Clone the repository
```
$ git clone https://github.com/evidence/axiom-evi.git
$ cd axiom-evi
```

### 2.2 Init the sub-modules (may take awhile)
```
$ ./scripts/submodules_update.sh
```

## 3. AXIOM Software Stack compilation for the AXIOM Board (Xilinx UltraScale+)

In the [README\_AXIOM\_Board.md](README_AXIOM_Board.md) you can find more
details about the compilation of AXIOM Software Stack for the AXIOM Board.

