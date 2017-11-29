////////////////////////////////////////////////////////////////
/////////////SORTER.C README FILE AND DOCUMENTATION/////////////
//////////////////PETER LAMBE AND UMAR RABBANI//////////////////
////////////////////////////////////////////////////////////////


HOW TO RUN THE CODE:

Had to include mergesort.c in the compile command because C didn't know the two files were linked (this can be solved with a makefile, but we weren't sure
if that was part of the requirements). So here is the command you must run in order to compile our code!

Compile Command:
gcc -pthread -o sorter_thread mergesort.c sorter_thread.c

Execution Command:
./sorter_thread -c COLUMNNAME

Optional parameters
-d DIR NAME (starts from the given directory)
-o DIR NAME (outputs to the given directory)
[Parameters can be entered in any order]

Output:
If successful, one singled sorted CSV containing the contents from every discovered CSV file will be placed into the starting directory, or the output directory of -o was used
Stdout will show process id's and total number of child processes created.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DESIGN DECISIONS:

Work-Flow:
The programs work flow is as follows:

Run the logic to determine which parameters were used and if they were valid
Call populateStructTitles() to populate the Title columns in the output CSV
Call a method called traverseDirectory() that runs through the given directory item by item and determines if it is a CSV or if it is a directory.

If CSV - it will run beginSort which will add all of the contents to a global data structure
If Directory - it will call traverseDirectory() again on the found directory

Then, mergesort is run on the global data structure and exported into a CSV
    
Header File:
In our header file, we have 3 important things (aside from the includes and the definitions). First is the struct that 
is the backbone for our code. This is necessary because it is the only feasible way to achieve to results of this program
without writing some really ugly code. We have commented each field in the struct describing what they are for. 

Additionally, we have defined the function definition for the mergesort() function along with our isString() function.

    
    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DIFFICULTIES/ASSUMPTIONS:
-This was also hard!
-Most of the work was figuring out the thread logic and making it robust
-Had to do some tricky string manipulation for appending/modifying directory paths in order to maintain robustness

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TESTING PROCEDURE:
-Ran various testing using nested directories of length 0-16, and various different kinds of CSV sizes and types in different locations
-Various testing for the flag parameters to be entered in any order, and to make sure they work under all conditions
-Ran run-time analysises to compare this with project 1

