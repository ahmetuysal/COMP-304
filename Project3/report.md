# COMP 304 Project 3: Space Allocation Methods

> Ahmet Uysal, 60780

## Implementation

I used Java to implement this project. Allocation systems in question are implemented as two classes: `ContiguousAllocationFileSystem` and `LinkedAllocationFileSystem`.

`ContiguousAllocationFileSystem` implementation is straight forward and easy to understand. In `LinkedAllocationFileSystem` we needed to also store _File Allocation Table (FAT)_ in the directory. I implemented this by dividing files into parts that I referred in the code as _chunks_. Every file consists of one or more chunks.
Every chunk consist of three parts:

1. Index 0 stores amount of data blocks stored in the chunk (size)
2. Indices 1-size (inclusive) store data blocks
3. Index size+1 stores the first index of next chunk (or -1 if this is the last chunk of the file)

Therefore, every file is stored using **data size (in blocks) + 2 \* number of chunks** blocks. My implementation required some edge cases such as roll back of changes in the case of failure (due to insufficient space). I tried to leave helpful comments to make it easier to understand but it might require some thought (especially at indexing and boundary checking parts) to understand fully. Feel free to contact me if you have any questions.

## Results

### `input_8_600_5_5_0.txt`

#### Linked Allocation: Average Runtime = 353.1796 μs, Average Running Time of an Operation = 203.3273 ns

Result{numberOfOperations=1737, rejectedCreations=599, rejectedExtensions=551, rejectedShrinks=0, runtime=358299}
Result{numberOfOperations=1737, rejectedCreations=599, rejectedExtensions=551, rejectedShrinks=0, runtime=373100}
Result{numberOfOperations=1737, rejectedCreations=599, rejectedExtensions=551, rejectedShrinks=0, runtime=349099}
Result{numberOfOperations=1737, rejectedCreations=599, rejectedExtensions=551, rejectedShrinks=0, runtime=370500}
Result{numberOfOperations=1737, rejectedCreations=599, rejectedExtensions=551, rejectedShrinks=0, runtime=314900}

#### Contiguous Allocation: Average Runtime = 18551.8404 μs, Average Running Time of an Operation = 10680.3917 ns

Result{numberOfOperations=1737, rejectedCreations=213, rejectedExtensions=201, rejectedShrinks=0, runtime=33803900}
Result{numberOfOperations=1737, rejectedCreations=213, rejectedExtensions=201, rejectedShrinks=0, runtime=27435500}
Result{numberOfOperations=1737, rejectedCreations=213, rejectedExtensions=201, rejectedShrinks=0, runtime=11457701}
Result{numberOfOperations=1737, rejectedCreations=213, rejectedExtensions=201, rejectedShrinks=0, runtime=10157700}
Result{numberOfOperations=1737, rejectedCreations=213, rejectedExtensions=201, rejectedShrinks=0, runtime=9904401}

### `input_1024_200_5_9_9.txt`

#### Linked Allocation: Average Runtime = 938.8198 μs, Average Running Time of an Operation = 281.0837 ns

Result{numberOfOperations=3340, rejectedCreations=199, rejectedExtensions=1361, rejectedShrinks=1539, runtime=1246299}
Result{numberOfOperations=3340, rejectedCreations=199, rejectedExtensions=1361, rejectedShrinks=1539, runtime=745200}
Result{numberOfOperations=3340, rejectedCreations=199, rejectedExtensions=1361, rejectedShrinks=1539, runtime=634300}
Result{numberOfOperations=3340, rejectedCreations=199, rejectedExtensions=1361, rejectedShrinks=1539, runtime=1017200}
Result{numberOfOperations=3340, rejectedCreations=199, rejectedExtensions=1361, rejectedShrinks=1539, runtime=1051100}

#### Contiguous Allocation: Average Runtime = 19554.4602 μs, Average Running Time of an Operation = 5854.6288 ns

Result{numberOfOperations=3340, rejectedCreations=34, rejectedExtensions=510, rejectedShrinks=432, runtime=20360900}
Result{numberOfOperations=3340, rejectedCreations=34, rejectedExtensions=510, rejectedShrinks=432, runtime=19206600}
Result{numberOfOperations=3340, rejectedCreations=34, rejectedExtensions=510, rejectedShrinks=432, runtime=19721400}
Result{numberOfOperations=3340, rejectedCreations=34, rejectedExtensions=510, rejectedShrinks=432, runtime=18479101}
Result{numberOfOperations=3340, rejectedCreations=34, rejectedExtensions=510, rejectedShrinks=432, runtime=20004300}

### `input_1024_200_9_0_0.txt`

#### Linked Allocation: Average Runtime = 23838.9802 μs, Average Running Time of an Operation = 58.2414 ns

Result{numberOfOperations=409313, rejectedCreations=199, rejectedExtensions=0, rejectedShrinks=0, runtime=16149800}
Result{numberOfOperations=409313, rejectedCreations=199, rejectedExtensions=0, rejectedShrinks=0, runtime=21202900}
Result{numberOfOperations=409313, rejectedCreations=199, rejectedExtensions=0, rejectedShrinks=0, runtime=20105300}
Result{numberOfOperations=409313, rejectedCreations=199, rejectedExtensions=0, rejectedShrinks=0, runtime=45522801}
Result{numberOfOperations=409313, rejectedCreations=199, rejectedExtensions=0, rejectedShrinks=0, runtime=16214100}

#### Contiguous Allocation: Average Runtime = 10828.2000 μs, Average Running Time of an Operation = 26.4545 ns

Result{numberOfOperations=409313, rejectedCreations=80, rejectedExtensions=0, rejectedShrinks=0, runtime=13916999}
Result{numberOfOperations=409313, rejectedCreations=80, rejectedExtensions=0, rejectedShrinks=0, runtime=9861600}
Result{numberOfOperations=409313, rejectedCreations=80, rejectedExtensions=0, rejectedShrinks=0, runtime=9567700}
Result{numberOfOperations=409313, rejectedCreations=80, rejectedExtensions=0, rejectedShrinks=0, runtime=9800900}
Result{numberOfOperations=409313, rejectedCreations=80, rejectedExtensions=0, rejectedShrinks=0, runtime=10993801}

### `input_1024_200_9_0_9.txt`

#### Linked Allocation: Average Runtime = 864.9002 μs, Average Running Time of an Operation = 327.8620 ns

Result{numberOfOperations=2638, rejectedCreations=199, rejectedExtensions=0, rejectedShrinks=824, runtime=1138600}
Result{numberOfOperations=2638, rejectedCreations=199, rejectedExtensions=0, rejectedShrinks=824, runtime=771400}
Result{numberOfOperations=2638, rejectedCreations=199, rejectedExtensions=0, rejectedShrinks=824, runtime=775101}
Result{numberOfOperations=2638, rejectedCreations=199, rejectedExtensions=0, rejectedShrinks=824, runtime=847500}
Result{numberOfOperations=2638, rejectedCreations=199, rejectedExtensions=0, rejectedShrinks=824, runtime=791900}

#### Contiguous Allocation: Average Runtime = 2864.2002 μs, Average Running Time of an Operation = 1085.7468 ns

Result{numberOfOperations=2638, rejectedCreations=0, rejectedExtensions=0, rejectedShrinks=0, runtime=3394600},
Result{numberOfOperations=2638, rejectedCreations=0, rejectedExtensions=0, rejectedShrinks=0, runtime=3308800},
Result{numberOfOperations=2638, rejectedCreations=0, rejectedExtensions=0, rejectedShrinks=0, runtime=3759800},
Result{numberOfOperations=2638, rejectedCreations=0, rejectedExtensions=0, rejectedShrinks=0, runtime=2079400},
Result{numberOfOperations=2638, rejectedCreations=0, rejectedExtensions=0, rejectedShrinks=0, runtime=1778401}

### `input_2048_600_5_5_0.txt`

#### Linked Allocation: Average Runtime = 506.6804 μs, Average Running Time of an Operation = 292.7096 ns

Result{numberOfOperations=1731, rejectedCreations=599, rejectedExtensions=531, rejectedShrinks=0, runtime=657400}
Result{numberOfOperations=1731, rejectedCreations=599, rejectedExtensions=531, rejectedShrinks=0, runtime=400201}
Result{numberOfOperations=1731, rejectedCreations=599, rejectedExtensions=531, rejectedShrinks=0, runtime=577900}
Result{numberOfOperations=1731, rejectedCreations=599, rejectedExtensions=531, rejectedShrinks=0, runtime=326000}
Result{numberOfOperations=1731, rejectedCreations=599, rejectedExtensions=531, rejectedShrinks=0, runtime=571901}

#### Contiguous Allocation: Average Runtime = 18886.9398 μs, Average Running Time of an Operation = 10910.9993 ns

Result{numberOfOperations=1731, rejectedCreations=213, rejectedExtensions=181, rejectedShrinks=0, runtime=31426300}
Result{numberOfOperations=1731, rejectedCreations=213, rejectedExtensions=181, rejectedShrinks=0, runtime=26003299}
Result{numberOfOperations=1731, rejectedCreations=213, rejectedExtensions=181, rejectedShrinks=0, runtime=16272301}
Result{numberOfOperations=1731, rejectedCreations=213, rejectedExtensions=181, rejectedShrinks=0, runtime=11285299}
Result{numberOfOperations=1731, rejectedCreations=213, rejectedExtensions=181, rejectedShrinks=0, runtime=9447500}

## Questions

1- With test instances having a block size of 1024, in which cases (inputs) contiguous allocation has a shorter average operation time? Why? What are the dominating operations in these cases? In which linked is better, why?

In the case `input_1024_200_9_0_0.txt` contiguous allocation (26.4545 ns) is more than two times faster than linked allocation (58.2414 ns). This test case almost completely consist of _read_ operations. Contiguous allocation can directly read the given offset in constant time whereas linked allocation need to travel to the corresponding _chunk_ to read this data.

In the cases `input_1024_200_5_9_9` and `input_1024_200_9_0_9.txt` linked allocation is significantly faster than contiguous allocation. _shrink_ and _extend_ are the dominating operations in `input_1024_200_5_9_9`. _shrink_, _access_, and _create_ are the dominating operations in `input_1024_200_9_0_9.txt`. _create_ and _extend_ operations take more time in contiguous allocation since they might require **compaction** and this is a costly operation.

2- Comparing the difference between the creation rejection ratios with block size 2048 and 32, what can you conclude? How did dealing with smaller block sizes affect the FAT memory utilization?

In my implementation I directly used blocks (which are represented as integers) to store FAT related entries. Therefore, I don't think it should have a significant impact for my implementation since we represented each block with an integer. However, normally this would impact how many of the blocks are filled with FAT entries. Since the size of FAT entries are always 4 byte, they would take much less space relative to data blocks in the case of block size of 2048.

3- FAT is a popular way to implement linked allocation strategy. This is because it permits faster access compared to the case where the pointer to the next block is stored as a part of the concerned block. Explain why this provides better space utilization.

Because FAT requires much less overhead compared to storing a pointer at each block. Instead of storing a pointer for each data entry, we can use much smaller amount of pointers to indicate jumps between data chunks. 

4- If you have extra memory available of a size equal to the size of the DT, how can this improve the performance of your defragmentation?

We implemented defragmentation by moving every file to lower indices. If we continue to naively move files upwards block by block having extra space will not improve performance. However, we can design more sophisticated algorithms with the usage of this extra space.

5- How much, at minimum, extra memory do you need to guarantee reduction in the number of rejected extensions in the case of contiguous allocations?

In some cases, we might reject the extension even when directory has sufficient space. My compaction algorithm moves files one by one to lower indices in ascending order by starting indices. Without extra space we cannot move the file we extend to end since it will override other files during the process. If we have extra space equal to the **file's initial size before extension**, we can copy the file there, remove file from `directoryTable`, apply compaction, add file back to the, extend the file, and add file information back to `directoryTable`.
