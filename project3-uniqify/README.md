# description

Write a C program called uniqify. Uniqify is to read a text file and output the unique words in the files, sorted in alphabetic order. 
The input is from stdin and the output is to stdout.

Internally, the program would be organized into 3 processes. A single process reads the input parsing the lines into words. 
Another process does the sorting, and a single process supresses duplicate words and writes the output. 
You should organize the processes so that every parent waits for its children. 

You must use the system sort command (/bin/sort) with no arguments to do the actual sorting, 
and your program must arrange to start the processes and plumb the pipes. 
