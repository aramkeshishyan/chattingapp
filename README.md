# Chatting App

## Table of contents
* [General info](#general-info)
* [Prerequisites](#prerequisites)
* [Technologies](#technologies)
* [Setup](#setup)
* [Functionality](#functionality)
* [Input file format and examples](#input-file-format-and-examples)
* [Available Commands](#available-commands)

## General info
This project is a peer-to-peer application

## PREREQUISITES:
If running on a windows machine use WSL(Windows Subsystem for Linux)

## Technologies
Project is created with:
* C environment
* Linux Socket API
* POSIX TREADS(Pthreads) library

## Setup
To run this project use any Linux Terminal
```
$ cd ../chattingapp
```
```
$ gcc chat.c -o chat -lpthread
```
```
$ ./chat portNumber
```

## Functionality
* Run the program using up to 10 instances of terminals to immitate up to 10 group members


## Available Commands
The following commands can be invoked at any point 
* **help** : Display available commands
* **myip**: Display IP address of this process
* **myport**: Display the listening port
* **connect destinationIP prot#**: Initiate a TCP connection with the specified destination 
* **list**: Display a list of connections this process is part of
* **terminate connectionID**: terminate a connection using its ID displayed by the **list** command
* **send connectionID message**: Send a message to a host using their ID displayed by the **list** command
* **exit**: Close all connections and terminate the process
