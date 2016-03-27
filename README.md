# Disk-based-merge-sort

Performs merge sort on a huge input file that does not fit into the main memory.
Makes use of disk based merging where part of file is sorted in chunks and merged finally using heap algorithm.

Merge sort methods:
  (i) Basic merge sort
  (ii) Multistep merge sort
  (iii) Replacement selection merge sort

Compilation command: g++ assn_3.cpp -o assn_3
Executing the code: ./assn_3 --merge-method --input-file --output-file

--merge-method is one of the merge methods given above.
--input-file is a huge file containing unsorted integers.
--output-file is the file to which the sorted output will be written in chunks. 
If the file does not exist, a new file in the name will be created and written.
