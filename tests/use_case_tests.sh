#!/bin/bash

OUT_DIR="../out/"
VALGRIND="valgrind --leak-check=full --show-reachable=yes --quiet"
TESTS_NUMBER=15

function tests()
{
    case "$1" in

    "test1")
        echo "$(tput setf 1)${1^^}     unexisting flag$(tput sgr0)"	
        echo "$(tput setf 2)EXPECTED  help print$(tput sgr0)"	
        command="${VALGRIND} ${OUT_DIR}redis_executor -v"
        echo "COMMAND   $command"
        $command
        echo 
        ;;
    "test2")
        echo "$(tput setf 1)${1^^}      sentinel with bad port$(tput sgr0)"	
        echo "$(tput setf 2)EXPECTED   \"cann't resolve\" message $(tput sgr0)"	
        command="${VALGRIND} ${OUT_DIR}redis_executor -s 12799.0.0.1:26379"
        echo "COMMAND   $command"
        $command
        echo 
        ;;
    "test3")
        echo "$(tput setf 1)${1^^}      several sentinels, one with bad port$(tput sgr0)"	
        echo "$(tput setf 2)EXPECTED    failed to connect to...., than master obtained from well specified redis and default PING command issued on it $(tput sgr0)"	
        command="${VALGRIND} ${OUT_DIR}redis_executor -s 127.0.0.1:22,127.0.0.1:26379"
        echo "COMMAND   $command"
        $command
        echo 
        ;;
    "test4")
        echo "$(tput setf 1)${1^^}      one correct sentinel, no command$(tput sgr0)"	
        echo "$(tput setf 2)EXPECTED  redis master obtained from sentinel and default PING command issued $(tput sgr0)"	
        command="${VALGRIND} ${OUT_DIR}redis_executor -s 127.0.0.1:22"
        echo "COMMAND   $command"
        $command
        echo 
        ;;
    "test5")
        echo "$(tput setf 1)${1^^}      several sentinels, one bad, several commands all correct$(tput sgr0)"	
        echo "$(tput setf 2)EXPECTED  failed to connect to bad sentinel, than outputs from issued commands$(tput sgr0)"	
        command="${VALGRIND} ${OUT_DIR}redis_executor -s 127.0.0.1:22,127.0.0.1:26379 GET mykey;GET foobar; SET mykey zium; GET mykey;"
        echo "COMMAND   $command"
        $command
        echo 
        ;;
    "test6")
        echo "$(tput setf 1)${1^^}      several sentinels, one bad, several commands all correct, one string, one space between command separator and command$(tput sgr0)"	
        echo "$(tput setf 2)EXPECTED  failed to connect to bad sentinel, than outputs from issued commands$(tput sgr0)"	
        command="${VALGRIND} ${OUT_DIR}redis_executor -s 127.0.0.1:22,127.0.0.1:26379 GET mykey;GET foobar; SET mykey zium;GET mykey;"
        echo "COMMAND   $command"
        $command
        echo 
        ;;
    "test7")
        #
        echo "$(tput setf 1)${1^^}      several sentinels, one bad, several commands, one incorrect$(tput sgr0)"	
        echo "$(tput setf 2)EXPECTED  failed to connect to bad sentinel, than output from correct command and info that bad command does not exist$(tput sgr0)"	
        commmand="${VALGRIND} ${OUT_DIR}redis_executor -s 127.0.0.1:22,127.0.0.1:26379 GET mykey;GETT foobar;SET mykey zium;GET mykey;"
        echo "COMMAND   $command"
        $command
        echo 
        ;;
    "test8")
        #
        echo "$(tput setf 1)${1^^}     several sentinels, one bad, several commands, one unexisting key$(tput sgr0)"	
        echo "$(tput setf 2)EXPECTED  failed to connect to bad sentinel, than output from correct command and info that bad command does not exist$(tput sgr0)"	
        command="${VALGRIND} ${OUT_DIR}redis_executor -s 127.0.0.1:22,127.0.0.1:26379 GET mykey;GET foobar;SET mykey zium;GET mykea;"
        echo "COMMAND   $command"
        $command
        echo 
        ;;
    "test9")
        #
        echo "$(tput setf 1)${1^^}    input with unexpected (unpaired) \"$(tput sgr0)"	
        echo "$(tput setf 2)EXPECTED  message that there was malformed command$(tput sgr0)"	
        command="${VALGRIND} ${OUT_DIR}redis_executor -s 127.0.0.1:26379 GET mykey;GET \"foobar;SET mykey zium;GET mykea;"
        echo "COMMAND   $command"
        $command
        echo 
        ;;
    "test10")
        #
        echo "$(tput setf 1)${1^^}    badly used redis command, too many arguments$(tput sgr0)"	
        echo "$(tput setf 2)EXPECTED  message that there was badly used command$(tput sgr0)"	
        command="${VALGRIND} ${OUT_DIR}redis_executor -s 127.0.0.1:26379 GET bambam bimbim"
        echo "COMMAND   $command"
        $command
        echo 
        ;;
    "test11")
        #
        echo "$(tput setf 1)${1^^}    reading commands form file, file badly specified ( at the end of commands )$(tput sgr0)"	
        echo "$(tput setf 2)EXPECTED  message that there was badly used command$(tput sgr0)"	
        command="${VALGRIND} ${OUT_DIR}redis_executor -s 127.0.0.1:26379 GET bambam bimbim -c foo"
        echo "COMMAND   $command"
        $command
        echo 
        ;;
    "test12")
        #
        echo "$(tput setf 1)${1^^}    reading commands from nonexisting file$(tput sgr0)"	
        echo "$(tput setf 2)EXPECTED  message that specified file with commands doesn't exists$(tput sgr0)"	
        command="${VALGRIND} ${OUT_DIR}redis_executor -c foo -s 127.0.0.1:26379"
        echo "COMMAND   $command"
        $command
        echo 
        ;;
    "test13")
        #
        echo "$(tput setf 1)${1^^}    reading commands from file$(tput sgr0)"	
        echo "$(tput setf 2)EXPECTED  usual output for each command, with newlines at the end of line removed$(tput sgr0)"	
        command="${VALGRIND} ${OUT_DIR}redis_executor -c commands.txt -s 127.0.0.1:26379"
        echo "COMMAND   $command"
        $command
        echo 
        ;;
    "test14")
        echo "$(tput setf 1)${1^^}      more substrings than max allowed$(tput sgr0)"    
        echo "$(tput setf 2)EXPECTED  terminating application with info about substrings limit exceeded$(tput sgr0)"   
        command="${VALGRIND} ${OUT_DIR}redis_executor -c commands2.txt -s 127.0.0.1:26379"
        echo "COMMAND   $command"
        $command
        echo
        ;;   
    "test15")
        echo "$(tput setf 1)${1^^}      LRANGE command executed$(tput sgr0)"    
        echo "$(tput setf 2)EXPECTED    commands with their list responses$(tput sgr0)"   
        command="${VALGRIND} ${OUT_DIR}redis_executor -s 127.0.0.1:26379 LPUSH testlist item1 item2 item3; LRANGE testlist 0 -1"
        echo "COMMAND   $command"
        $command
        echo
        ;;
    *)  echo "no test specified"
        ;;
    esac
}

trap '' 2

if [ $# -ne 1 ]
then
    echo "Usage: ./tests.sh testname"
    echo "where testname test1..test${TESTS_NUMBER} or \"all\" to run all tests"
    exit
fi

if [ "$1" == "all" ]
then
    for i in $(seq 1 ${TESTS_NUMBER})
    do
        tests "test$i"
        echo "---------------------"
    done
else
    tests $1
fi

