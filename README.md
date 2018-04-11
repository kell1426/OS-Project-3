/*kell1426
*04/11/18
*Daniel Kelly
*4718021*/

CSCI 4061 Programming Project 3
Daniel Kelly
x500: kell1426
ID: 4718021

Lecture Section: 10

I did not implement the extra credit.

The purpose of this program is to simulate a votecounting application with threading. It takes in encrypted files, decrypts them, and counts up the votes in each leaf node. These votes are then aggregated up to the top node, and the winner is declared.

To compile this program, just run the command "make" using the included makefile. To remove the executable, run the command "make clean".

To execute this program type: ./votecounter <DAG.txt> <input_dir> <output_dir>
Where <DAG.txt> is the text file containing the information about the nodes.
<input_dir> is the input directory containing the input files.
<output_dir> is the output directory where all of the output will be stored.

This program takes in a text file that describes how the nodes are connected together (parent/child connections) and an input directory containing the files that have the votes for each leaf node. A data structure is created to model this, and all the info is loaded into the data structure. The program then implements the model of the data structure into respective files. To improve the speed of this program, a thread is created for each leaf node that will aggregate its votes upwards to the top node. The threads decrypt their input file, save the candidate info, and aggregate the votes up the data structure. Synchronization methods using semaphores have been implemented so that only one thread can operate on one node at a time. After all the threads have finished, the main thread declares the winner and appends it to the root nodes output aggregate file.

I worked on this program myself.


