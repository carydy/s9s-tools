#! /bin/bash
MYNAME=$(basename $0)
MYBASENAME=$(basename $0 .sh)
MYDIR=$(dirname $0)
VERBOSE=""
VERSION="0.0.3"
LOG_OPTION="--wait"
CONTAINER_SERVER=""
CONTAINER_IP=""
CLUSTER_NAME="${MYBASENAME}_$$"
LAST_CONTAINER_NAME=""

cd $MYDIR
source ./include.sh
source ./include_lxc.sh

#
# Prints usage information and exits.
#
function printHelpAndExit()
{
cat << EOF
Usage: $MYNAME [OPTION]... [TESTNAME]
 Test script for s9s to check various error conditions.

 -h, --help       Print this help and exit.
 --verbose        Print more messages.
 --print-json     Print the JSON messages sent and received.
 --log            Print the logs while waiting for the job to be ended.
 --print-commands Do not print unit test info, print the executed commands.
 --install        Just install the server and exit.
 --reset-config   Remove and re-generate the ~/.s9s directory.
 --server=SERVER  Use the given server to create containers.

SUPPORTED TESTS:
  o registerServer   Registers a new container server. No software installed.
  o checkServerTree  Checks how the server looks like in the tree.
  o checkState       Checks the server state device.

EOF
    exit 1
}

ARGS=$(\
    getopt -o h \
        -l "help,verbose,print-json,log,print-commands,install,reset-config,\
server:" \
        -- "$@")

if [ $? -ne 0 ]; then
    exit 6
fi

eval set -- "$ARGS"
while true; do
    case "$1" in
        -h|--help)
            shift
            printHelpAndExit
            ;;

        --verbose)
            shift
            VERBOSE="true"
            OPTION_VERBOSE="--verbose"
            ;;

        --log)
            shift
            LOG_OPTION="--log"
            ;;

        --print-json)
            shift
            OPTION_PRINT_JSON="--print-json"
            ;;

        --print-commands)
            shift
            DONT_PRINT_TEST_MESSAGES="true"
            PRINT_COMMANDS="true"
            ;;

        --install)
            shift
            OPTION_INSTALL="--install"
            ;;

        --reset-config)
            shift
            OPTION_RESET_CONFIG="true"
            ;;

        --server)
            shift
            CONTAINER_SERVER="$1"
            shift
            ;;

        --)
            shift
            break
            ;;
    esac
done

if [ -z "$OPTION_RESET_CONFIG" ]; then
    printError "This script must remove the s9s config files."
    printError "Make a copy of ~/.s9s and pass the --reset-config option."
    exit 6
fi

if [ -z "$CONTAINER_SERVER" ]; then
    printError "No container server specified."
    printError "Use the --server command line option to set the server."
    exit 6
fi

function checkServerTree()
{
    local lines

    print_title "Checking Server in Tree"

    mys9s tree --tree --all "/$CONTAINER_SERVER"
    lines=$(s9s tree --tree --all "/$CONTAINER_SERVER")

    if ! echo "$lines" | grep --quiet "/$CONTAINER_SERVER"; then
        failure "8742 Tree check failed"
    fi

    if ! echo "$lines" | grep --quiet ".runtime"; then
        failure "8743 Tree check failed"
    fi
    
    if ! echo "$lines" | grep --quiet "containers"; then
        failure "8744 Tree check failed"
    fi
}

function checkState()
{
    local lines

    print_title "Checking Device Files"
   
    mys9s tree \
        --cmon-user=system \
        --password=secret \
        --cat /.runtime/server_manager

    #
    # Checking the state... TBD
    #
    mys9s tree --cat /$CONTAINER_SERVER/.runtime/state 

    lines=$(s9s tree --cat /$CONTAINER_SERVER/.runtime/state)
    if ! echo "$lines" | grep --quiet "CmonLxcServer"; then
        failure "The device file seems to be missing class name"
    fi
    
    if ! echo "$lines" | grep --quiet "server_name"; then
        failure "The device file seems to be missing host name"
    fi
    
    if ! echo "$lines" | grep --quiet "number_of_processors"; then
        failure "The device file seems to be missing the number of CPUs"
    fi
}

#
#
#
function unregisterServer()
{
    print_title "Unregistering Server"
    mys9s server --unregister --servers="lxc://$CONTAINER_SERVER"

    check_exit_code_no_job $?    

    mys9s tree --tree --all
}

#
# Running the requested tests.
#
startTests
reset_config
grant_user

if [ "$OPTION_INSTALL" ]; then
    runFunctionalTest registerServer
elif [ "$1" ]; then
    for testName in $*; do
        runFunctionalTest "$testName"
    done
else
    runFunctionalTest registerServer
    runFunctionalTest checkServerTree
    runFunctionalTest checkState

    runFunctionalTest unregisterServer

    runFunctionalTest registerServer
    runFunctionalTest checkState
fi

endTests
