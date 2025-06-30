# Basic-Shell-For-CLI
A basic UNIX shell to interpret command line prompts

This repository contains the C code making up a basic UNIX shell that will be able to interpret user input commands through the command line. Like a linux terminal, this shell will simply wait for user input and will parse the given command once one is submitted. Based on the user commmand, it will execute the command and pause the shell input until command action is successfully completed. 

This shell will be able to handle a number of basic user commands such as the following...
cd, ls, local, export and exit. It can also handle a few smaller commands but these are the main functions.

In addition to being able to handle these commands, it is able to handle all major error conditions that could occur durring the shells operation. It also carefully frees all memory once its no longer needed to prevent memory leaks and to optimize system performance.
