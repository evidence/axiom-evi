#!/bin/bash

CHRT=/usr/bin/axiom-chrt
TASKSET=/usr/bin/taskset
RENICE=/usr/bin/renice
PS=/bin/ps

KTHREAD=
SCHED=
NICE=
PRIORITY=1
RUNTIME=0
PERIOD=0
DEADLINE=0

function usage () {
    echo -e "\nUsage: $0 -k KTHREAD [options]\n"
    echo -e "Set and get scheduler parameters and CPU affinity for AXIOM Kthreads"
    echo -e "Example: $0 -k raw   # get parameters for RAW kthread"
    echo -e "Example: $0 -k rdma -s rr -p 10  # set RR scheduler with priority 10 for RDMA kthread"
    echo -e "\noptions:\n"
    echo -e "    -k  KTHREAD   kthread to use [raw, rdma, wtd]"
    echo -e "    -a  AFFINITY  affinity mask to set (see taskset man page for details)"
    echo -e "    -s  SCHED     scheduler to set [other, idle, batch, rr, fifo, deadline]"
    echo -e "    -n  NICE      nice value [-20,19] to set, used only with other, idle and batch"
    echo -e "    -p  PRIORITY  priority value [1,99] to set, used only with rr and fifo"
    echo -e "    -T  RUNTIME   runtime value <ns> to set, used only with deadline (see chrt man page for details)"
    echo -e "    -P  PERIOD    period value <ns> to set, used only with deadline"
    echo -e "    -D  DEADLINE  period value <ns> to set, used only with deadline"
    echo -e "    -h            print this help"

}

while getopts "k:a:s:n:p:T:P:D:h" opt; do
    case $opt in
        k)
            KTHREAD=$OPTARG
            ;;
        a)
            AFFINITY=$OPTARG
            ;;
        s)
            SCHED=$OPTARG
            ;;
        n)
            NICE=$OPTARG
            ;;
        p)
            PRIORITY=$OPTARG
            ;;
        T)
            RUNTIME=$OPTARG
            ;;
        P)
            PERIOD=$OPTARG
            ;;
        D)
            DEADLINE=$OPTARG
            ;;
        h)
            usage
            exit 0
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            usage
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument." >&2
            usage
            exit 1
            ;;
    esac

done
shift $((OPTIND-1))

# Make sure only root can run this script
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root" 1>&2
    exit 1
fi

if [ x$KTHREAD == x ]; then
    echo "-k parameter is mandatory" 1>&2
    usage
    exit -1
fi

PID=$(cat /sys/devices/axiom/kthread_$KTHREAD/pid)

if [ x$PID == x ]; then
    echo "kthread not recognized" 1>&2
    exit -2
fi

# get affinity and scheduler parameters
if [ x$SCHED == x ] && [ x$AFFINITY == x ]; then
    eval "$TASKSET -p $PID"
    eval "$CHRT -p $PID"
    eval "$PS -o pid,comm,pri,nice -p $PID"
    exit 0
fi

# set affinity mask
if [ x$AFFINITY != x ]; then
    eval "$TASKSET -p $AFFINITY $PID"
fi

# set scheduler parameters
if [ x$SCHED != x ]; then
    case $SCHED in
        other)
            eval "$CHRT -v -o -p 0 $PID"
            if [ x$NICE != x ]; then
                eval "$RENICE $NICE -p $PID"
            fi
            ;;
        idle)
            eval "$CHRT -v -i -p 0 $PID"
            if [ x$NICE != x ]; then
                eval "$RENICE $NICE -p $PID"
            fi
            ;;
        batch)
            eval "$CHRT -v -b -p 0 $PID"
            if [ x$NICE != x ]; then
                eval "$RENICE $NICE -p $PID"
            fi
            ;;
        rr)
            eval "$CHRT -v -r -p $PRIORITY $PID"
            ;;
        fifo)
            eval "$CHRT -v -f -p $PRIORITY $PID"
            ;;
        deadline)
            eval "$CHRT -v -d -P $PERIOD -D $DEADLINE -T $RUNTIME -p 0 $PID"
            ;;
        *)
            echo "scheduler $SCHED not recognized" 1>&2
            exit -3
            ;;
    esac
fi
