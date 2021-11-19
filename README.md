# README
<h1>Defragmenting a Disk</h1>
<h2>To Compile and Run</h2>
Library: <br >
make <br  >

Tests: <br >
After compiling and linking the library together <br >
./defrag "-h" will print a man page
./defrag [fragmented disk] will unfragment the disk

<h2>Introduction</h2>
This program defragments a fragmented disk.

<h3>Data Structures Implemented</h3>
The primary data structures implemented are just the structs for the super block and inode as well as traversing arrays a pointers.


<h2>Features</h2>
Puts the inodes files in order.


<h3>Not Implemented</h3>
Sometimes there is a segmentation fault when filling the free blocks. 

<h3>Tests Performed</h3>
Compared to Haosong's results (his were the only ones in the github atm). Our's are different so I am scared.
