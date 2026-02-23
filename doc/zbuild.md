
# zbuild 

## Operation order 

## Design Specification  

### 1. Parsing for build file  

#### 1.1 State machine  

### 2. Efficient Variable management  

#### 2.1 Hash table

#### 2.2 Kinds of variables

### 3. Rule Scheduling  

Core Algorithm which 

1. Resolve Dependencies (inspect children nodes)  

1.1. Edge

if this is file, it means this is an edge of the dependency tree.
-> check modification timestamp with ``` stat ```.

1.2. Node

If this is not a file, it must be a target name, otherwise error.

All target should be searched on a hash table by its hash index. 

Current implementation is not on a hash table unlike variable management.


The hash table should be seperated from the hash table for variable as its member string can be overrapped. 

If the hash index is matched with 

* There are 3 different types of representation for target. 

  * perfectly-matched target  
  
  * partially-matched target (use implicit rule)
    e.g. %abc, a%bc, abc%

    Num of % must be single, and it can be put anywhere.
    
    in this case, you should compute hash value omitting % in advance.

    For instance, if a%bc, you should compute hash value of abc.
    Note that you must pick up a hash function on which the value computed from the member is independent from previous or next string and its index.

    The first M string and the last N string should be recorded and kept tracked for finding its member.
    For instance, given a%bc, [M,N] = [1,2] should be recorded. Then, assume you find another target ; abc%de.
    Then, [M,N] = [3,2] will be recorded on somewhere else.

    When you find a string, given abcfghde, you should start from [M,N] = [3,2]. This means you compute hash of first 3 which is ```abc```, then compute the last 2 which is ``` de ```.
    If the hash value is matched with something on a hash table, validate it with strcmp but only the part without %.
    If none of them matched, short M to 1, now [M,N] = [1,2].
    you do not need to recompute simply negate the difference.
    

  * partially-matched target (use suffix rule)
    e.g. .c.o 

* else -> error  

* perfect

reference to source code 

* Command Execution

1. put task on a queue or multiple queues.

* 


