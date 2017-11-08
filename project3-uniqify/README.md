# description

Write a C program called uniqify. Uniqify is to read a text file and output the unique words in the files, sorted in alphabetic order. 
The input is from stdin and the output is to stdout.

Internally, the program would be organized into 3 processes. A single process reads the input parsing the lines into words. 
Another process does the sorting, and a single process supresses duplicate words and writes the output. 
You should organize the processes so that every parent waits for its children. 

You must use the system sort command (`/bin/sort`) with no arguments to do the actual sorting, 
and your program must arrange to start the processes and plumb the pipes. 

The I/O to and from the pipes should be done using the stream functions `fgets` and `fputs`, with `fdopen` for attaching to the pipes, 
and `fdclose` for flushing the streams. 

In this assignment words are all alphabetic and case insensitive, with the parser converting all alphabetic characters to lower case. Any non-alphabetic characters delimit words and are discarded. Any word shorter than 5 characters is discarded and any word over 35 words is truncated to 35. The output is the unique and sorted words preceded by a count of its multiplicity. For uniformity use %5d for the multiplicities. 

Optional extension for extra credit: use two sorting processes with the parsing process distributing alternate words to each sort and supporessing process merging as well as suppressing. 
