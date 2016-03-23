
APPS := axiom_discovery_protocol_test
CLEANFILES = $(APPS) *.o
CFLAGS += -g -I../axiom-docs/include/
LDFLAGS += -pthread

.PHONY: all clean

all: $(APPS)

axiom_discovery_protocol_test: axiom_discovery_protocol_test.o axiom_simulator.o axiom_nic_emulator.o axiom_node_code.o  axiom_discovery_protocol.o axiom_route_compute.o axiom_route_delivery.o axiom_route_set.o

axiom_discovery_protocol_test.o: axiom_discovery_protocol_test.c axiom_simulator.h axiom_discovery_protocol.h

axiom_simulator.o: axiom_simulator.c axiom_simulator.h axiom_route_compute.h

axiom_node_code.o: axiom_node_code.c axiom_node_code.h axiom_route_compute.h axiom_route_delivery.h axiom_route_set.h axiom_discovery_protocol.h

axiom_discovery_protocol.o: axiom_discovery_protocol.c axiom_discovery_protocol.h

axiom_nic_emulator.o: axiom_nic_emulator.c axiom_simulator.h

axiom_route_compute.o: axiom_route_compute.c axiom_discovery_protocol.h axiom_route_compute.h

axiom_route_delivery.o: axiom_route_delivery.c axiom_route_delivery.h axiom_discovery_protocol.h axiom_route_compute.h

axiom_route_set.o: axiom_route_set.c axiom_discovery_protocol.h axiom_route_compute.h axiom_route_set.h

clean:
	rm -rf $(CLEANFILES)
