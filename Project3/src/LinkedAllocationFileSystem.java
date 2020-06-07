import java.util.HashMap;
import java.util.Map;
import java.util.Random;

/**
 * @author Ahmet Uysal @ahmetuysal
 */
public class LinkedAllocationFileSystem implements FileSystem {

    private final Map<Integer, FileEntry> directoryTable;
    private final int[] directory;
    private final Random randomGenerator;
    private final int blockSize;
    private int emptyBlockCount;

    public LinkedAllocationFileSystem(int directorySize, int blockSize) {
        this.directoryTable = new HashMap<>();
        this.directory = new int[directorySize];
        this.emptyBlockCount = directorySize;
        this.blockSize = blockSize;
        this.randomGenerator = new Random();
    }

    @Override
    public boolean createFile(int fileId, int fileLength) {
        int correspondingBlockSize = (int) Math.ceil((double) fileLength / blockSize);
        // +2 because we will create at least 1 chunk and every chunk requires 2 additional indices
        if (emptyBlockCount < correspondingBlockSize + 2)
            return false;

        // each chunk consists of
        // index 0: size of data contained in chunk
        // indices 1-size (both inclusive): data
        // index size + 1: starting index of the next chunk or -1 if this is the last chunk

        FileEntry fileInfo = null;
        int blocksStored = 0;
        int chunkStartIndex = -1;
        int chunkSize = 0;
        int numberOfChunks = 0;
        // this is required since we will need to link each chunk to consequent chunk
        int lastIndexOfTheLatestChunk = -1;
        int i = 0;
        while (i < directory.length && blocksStored < correspondingBlockSize) {
            if (directory[i] == 0) {
                // starting a new chunk
                if (chunkStartIndex == -1) {
                    if (numberOfChunks == 0) {
                        // this is the first chunk, create the file entry
                        // this entry will also be used to rollback if file creation fails
                        fileInfo = new FileEntry(fileId, i, correspondingBlockSize);
                    } else {
                        // link latest chunk to this new chunk
                        directory[lastIndexOfTheLatestChunk] = i;
                    }
                    // this index (directory[i]) is left as it is for now
                    // we will write amount of data stored in the chunk to this index when the chunk ends
                    numberOfChunks++;
                    chunkStartIndex = i;
                }
                // continue writing current chunk
                else {
                    directory[i] = 1 + randomGenerator.nextInt(Integer.MAX_VALUE);
                    blocksStored++;
                    chunkSize++;
                }
                i++;
            } else {
                // we hit a chunk
                // we can finish current chunk and jump to the end of this chunk we just hit
                lastIndexOfTheLatestChunk = i - 1;
                chunkSize--;
                blocksStored--;
                directory[chunkStartIndex] = chunkSize;
                chunkStartIndex = -1;
                i += directory[i] + 2;
            }
        }

        if (blocksStored == correspondingBlockSize && i < directory.length) {
            // file is successfully stored
            // put chunk size to last chunk
            directory[chunkStartIndex] = chunkSize;
            // put -1 to the last index of last chunk (which is i)
            directory[i] = -1;
            emptyBlockCount -= (correspondingBlockSize + 2 * (numberOfChunks));
            directoryTable.put(fileId, fileInfo);
            return true;
        } else {
            // rollback changes in the directory

            int chunkToRemoveStartIndex = fileInfo.getStartingBlockIndex();
            // remove everything up to last chunk (which is interrupted and does not carry right size info
            while (chunkToRemoveStartIndex != chunkStartIndex) {
                // 1 (block indicating chunk size) + chunk size + 1 (block indicating next chunk)
                // next chunk block will be deleted after we retrieve location of the next chunk
                int size = directory[chunkToRemoveStartIndex] + 1;
                // we will remove chunk size amount of blocks
                for (int j = chunkToRemoveStartIndex; j < chunkToRemoveStartIndex + size; j++) {
                    directory[j] = 0;
                }
                int nextChunkToRemoveStart = directory[chunkToRemoveStartIndex + size];
                directory[chunkToRemoveStartIndex + size] = 0;
                chunkToRemoveStartIndex = nextChunkToRemoveStart;
            }
            // remove last chunk
            for (int j = chunkStartIndex; j <= chunkStartIndex + chunkSize; j++) {
                directory[j] = 0;
            }

            return false;
        }
    }

    @Override
    public int access(int fileId, int byteOffset) {
        FileEntry fileInfo = directoryTable.get(fileId);

        int correspondingBlockSize = (int) Math.floor((double) byteOffset / blockSize);

        // check for file size boundary
        if (fileInfo == null || correspondingBlockSize >= fileInfo.getFileSize()) {
            return -1;
        }

        int currentOffset = 0;
        int currentIndex = fileInfo.getStartingBlockIndex();

        // jump to next chunk until we are in the correct chunk
        while (currentOffset + directory[currentIndex] < correspondingBlockSize) {
            currentOffset += directory[currentIndex];
            currentIndex = directory[currentIndex + directory[currentIndex] + 1];
        }

        // current index stores the first index of the chunk we are looking for
        // current offset stores number of data blocks we have jumped so far
        return currentIndex + 1 + (correspondingBlockSize - currentOffset);
    }

    @Override
    public boolean extend(int fileId, int extensionBlocks) {
        if (emptyBlockCount < extensionBlocks)
            return false;

        FileEntry fileInfo = directoryTable.get(fileId);

        if (fileInfo == null) {
            return false;
        }

        int currentIndex = fileInfo.getStartingBlockIndex();
        // jump to latest chunk
        while (directory[currentIndex + directory[currentIndex] + 1] != -1) {
            currentIndex = directory[currentIndex + directory[currentIndex] + 1];
        }

        int blocksExtended = 0;
        int lastIndexOfTheLatestChunk = -1;

        // these two variables will be used in the case of rollback (extension fails due to storage)
        int chunkToRemoveStartingIndex = -1;
        // this will only be updated if we do an extension to latest block, otherwise it will left as -1
        int latestChunkInitialSize = -1;

        // try to extend the last block if there is room
        if (directory[currentIndex + directory[currentIndex] + 2] == 0) {
            int chunkSize = directory[currentIndex];
            // we can start from last index of the chunk since it was storing -1
            chunkToRemoveStartingIndex = currentIndex;
            latestChunkInitialSize = chunkSize;
            int i = currentIndex + chunkSize + 1;
            directory[i] = 0;
            while (i < (directory.length - 1) && blocksExtended < extensionBlocks && directory[i + 1] == 0) {
                directory[i] = 1 + randomGenerator.nextInt(Integer.MAX_VALUE);
                blocksExtended++;
                chunkSize++;
                i++;
            }
            directory[currentIndex] = chunkSize;
            lastIndexOfTheLatestChunk = i;
        }

        int numberOfNewChunks = 0;
        // store the remaining part of extension in new chunks (if required)
        // this part of extension algorithm is almost identical to create
        int i = 0;
        int chunkStartIndex = -1;
        int chunkSize = 0;

        while (i < directory.length && blocksExtended < extensionBlocks) {
            if (directory[i] == 0) {
                // starting a new chunk
                if (chunkStartIndex == -1) {
                    // link latest chunk to this new chunk
                    directory[lastIndexOfTheLatestChunk] = i;
                    // this index (directory[i]) is left as it is for now
                    // we will write amount of data stored in the chunk to this index when the chunk ends
                    numberOfNewChunks++;
                    chunkStartIndex = i;
                    // if latest chunk is not modified and this is the first new chunk, store its starting index to use in the case of rollback
                    if (chunkToRemoveStartingIndex == -1 && latestChunkInitialSize == -1) {
                        chunkToRemoveStartingIndex = i;
                    }
                }
                // continue writing current chunk
                else {
                    directory[i] = 1 + randomGenerator.nextInt(Integer.MAX_VALUE);
                    blocksExtended++;
                    chunkSize++;
                }
                i++;
            } else {
                // we hit a chunk
                // we can finish current chunk and jump to the end of this chunk we just hit
                lastIndexOfTheLatestChunk = i - 1;
                chunkSize--;
                blocksExtended--;
                directory[chunkStartIndex] = chunkSize;
                chunkStartIndex = -1;
                i += directory[i] + 2;
            }
        }

        if (blocksExtended == extensionBlocks && i < directory.length) {
            // file is successfully stored
            // put chunk size to last chunk (if any new blocks are created)
            if (numberOfNewChunks > 0) {
                directory[chunkStartIndex] = chunkSize;
            }
            // put -1 to the last index of last chunk
            directory[i > 0 ? i : lastIndexOfTheLatestChunk] = -1;
            emptyBlockCount -= extensionBlocks + 2 * numberOfNewChunks;
            fileInfo.setFileSize(fileInfo.getFileSize() + extensionBlocks);
            return true;
        } else {
            // rollback changes in the directory

            if (latestChunkInitialSize != -1) {
                // we made changes to former last chunk, we need to restore the old version
                int updatedSize = directory[chunkToRemoveStartingIndex];
                directory[chunkToRemoveStartingIndex] = latestChunkInitialSize;
                directory[chunkToRemoveStartingIndex + latestChunkInitialSize + 1] = -1;
                for (int j = chunkToRemoveStartingIndex + latestChunkInitialSize + 2; j < chunkToRemoveStartingIndex + updatedSize + 1; j++) {
                    directory[j] = 0;
                }

                chunkToRemoveStartingIndex = directory[chunkToRemoveStartingIndex + updatedSize + 2];
                directory[chunkToRemoveStartingIndex + updatedSize + 2] = 0;
            }

            // remove everything up to last chunk (which is interrupted and does not carry right size info
            while (chunkToRemoveStartingIndex != chunkStartIndex) {
                // 1 (block indicating chunk size) + chunk size + 1 (block indicating next chunk)
                // next chunk block will be deleted after we retrieve location of the next chunk
                int size = directory[chunkToRemoveStartingIndex] + 1;
                // we will remove chunk size amount of blocks
                for (int j = chunkToRemoveStartingIndex; j < chunkToRemoveStartingIndex + size; j++) {
                    directory[j] = 0;
                }
                int nextChunkToRemoveStart = directory[chunkToRemoveStartingIndex + size];
                directory[chunkToRemoveStartingIndex + size] = 0;
                chunkToRemoveStartingIndex = nextChunkToRemoveStart;
            }
            // remove last chunk
            for (int j = chunkStartIndex; j <= chunkStartIndex + chunkSize; j++) {
                directory[j] = 0;
            }
            return false;
        }
    }

    @Override
    public boolean shrink(int fileId, int shrinkingBlocks) {
        FileEntry fileInfo = directoryTable.get(fileId);

        if (fileInfo == null) {
            return false;
        }

        int newSizeInBlocks = fileInfo.getFileSize() - shrinkingBlocks;
        int currentIndex = fileInfo.getStartingBlockIndex();
        // jump to the chunk that will be the new last chunk
        int blocksJumped = 0;
        while (blocksJumped + directory[currentIndex] < newSizeInBlocks) {
            blocksJumped = directory[currentIndex];
            currentIndex = directory[currentIndex + directory[currentIndex] + 1];
        }

        int lastChunkNewSize = newSizeInBlocks - blocksJumped;
        int lastChunkOldSize = directory[currentIndex];
        int nextChunk = directory[currentIndex + lastChunkOldSize + 1];
        directory[currentIndex] = lastChunkNewSize;
        directory[currentIndex + lastChunkNewSize + 1] = -1;
        for (int i = currentIndex + lastChunkNewSize + 2; i < currentIndex + lastChunkOldSize + 2; i++) {
            directory[i] = 0;
        }

        // delete all next chunks
        while (nextChunk != -1) {
            int size = directory[nextChunk] + 1;
            // we will remove chunk size amount of blocks
            for (int j = nextChunk; j < nextChunk + size; j++) {
                directory[j] = 0;
            }
            int nextChunkToRemoveStart = directory[nextChunk + size];
            directory[nextChunk + size] = 0;
            nextChunk = nextChunkToRemoveStart;
        }

        emptyBlockCount += shrinkingBlocks;
        fileInfo.setFileSize(newSizeInBlocks);
        return true;
    }
}
