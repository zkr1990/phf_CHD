## PHF_CHD: (minimum) perfect hash function using CHD algorithm

## Description

This is a naive C++ implementation of CHD algorithm (practical version) for perfect hash function.
Use Jenkins hash for g(x).

All-in-one naive C++ coding, naive Chinese comments.

## Run
Generate keys for test input. (input.txt)
Input and output of phf are 64 bits integer numbers.

~~~
cd input_file
python test1_generation.py
~~~

Modify line 222 in chd.cpp according to the number of keys generated in input.txt.

~~~
cd ..
g++ chd.cpp -o chd
./chd
~~~



## Reference
Original paper:

F. C. Botelho, D. Belazzougui and M. Dietzfelbinger. Compress, hash and displace. In Proceedings of the 17th European Symposium on Algorithms (ESAâ€™09). Springer LNCS, 2009.
http://cmph.sourceforge.net/chd.html
