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
    private int emptyBlockCount;

    public LinkedAllocationFileSystem(int directorySize) {
        this.directoryTable = new HashMap<>();
        this.directory = new int[directorySize];
        this.emptyBlockCount = directorySize;
        this.randomGenerator = new Random();
    }

    @Override
    public boolean createFile(int fileId, int fileLength) {
        // +2 because we will create at least 1 chunk and every chunk requires 2 additional indices
        if (emptyBlockCount < fileLength + 2)
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
        while (i < directory.length && blocksStored < fileLength) {
            if (directory[i] == 0) {
                // starting a new chunk
                if (chunkStartIndex == -1) {
                    if (numberOfChunks == 0) {
                        // this is the first chunk, create the file entry
                        // this entry will also be used to rollback if file creation fails
                        fileInfo = new FileEntry(fileId, i, fileLength);
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
                // we can finish current chuck and jump to the end of this chunk we just hit
                lastIndexOfTheLatestChunk = i - 1;
                chunkSize--;
                blocksStored--;
                directory[chunkStartIndex] = chunkSize;
                chunkStartIndex = -1;
                i += directory[i] + 2;
            }
        }

        if (blocksStored == fileLength && i < directory.length) {
            // file is successfully stored
            // put chunk size to last chunk
            directory[chunkStartIndex] = chunkSize;
            // put -1 to the last index of last chunk (which is i)
            directory[i] = -1;
            emptyBlockCount -= (fileLength + 2 * (numberOfChunks));
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
        return 0;
    }

    @Override
    public boolean extend(int fileId, int extensionBlocks) {
        if (emptyBlockCount < extensionBlocks)
            return false;

        // allocate
        emptyBlockCount -= extensionBlocks;
        return true;
    }

    @Override
    public void shrink(int fileId, int shrinkingBlocks) {
        emptyBlockCount += shrinkingBlocks;
    }
}
