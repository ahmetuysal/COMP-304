import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;

/**
 * @author Ahmet Uysal @ahmetuysal
 */
public class ContiguousAllocationFileSystem implements FileSystem {

    private final Map<Integer, FileEntry> directoryTable;
    private final int[] directory;
    private final Random randomGenerator;
    private final int blockSize;
    private int emptyBlockCount;

    public ContiguousAllocationFileSystem(int directorySize, int blockSize) {
        this.directoryTable = new HashMap<>();
        this.directory = new int[directorySize];
        this.emptyBlockCount = directorySize;
        this.blockSize = blockSize;
        this.randomGenerator = new Random();
    }

    @Override
    public boolean createFile(int fileId, int fileLength) {
        int correspondingBlockSize = (int) Math.ceil((double) fileLength / blockSize);
        if (emptyBlockCount < correspondingBlockSize)
            return false;
        // after this point, we are sure that there is enough empty blocks to store the file

        int firstFitIndex = getFirstFitForSize(correspondingBlockSize);
        // if there is no holes big enough to fit the file, we need to apply compaction
        if (firstFitIndex == -1) {
            applyCompaction();
            // after the compaction, all files will be located at range [0 - (size-empty block count))
            // we can directly conclude that first fit index is this value
            firstFitIndex = directory.length - emptyBlockCount;
        }

        // allocate
        for (int i = firstFitIndex; i < firstFitIndex + correspondingBlockSize; i++) {
            directory[i] = 1 + randomGenerator.nextInt(Integer.MAX_VALUE);
        }
        emptyBlockCount -= correspondingBlockSize;

        // add file to directory table
        directoryTable.put(fileId, new FileEntry(fileId, firstFitIndex, correspondingBlockSize));

        return true;
    }

    @Override
    public int access(int fileId, int byteOffset) {
        int correspondingBlockSize = (int) Math.floor((double) byteOffset / blockSize);
        FileEntry fileInfo = directoryTable.get(fileId);
        // check for file size boundary
        if (fileInfo != null && correspondingBlockSize < fileInfo.getFileSize()) {
            return fileInfo.getStartingBlockIndex() + correspondingBlockSize;
        } else {
            return -1;
        }
    }

    @Override
    public boolean extend(int fileId, int extensionBlocks) {
        if (extensionBlocks < 0 || emptyBlockCount < extensionBlocks)
            return false;

        FileEntry fileInfo = directoryTable.get(fileId);

        if (fileInfo == null) {
            return false;
        }

        // directly extend the file if area right after file is already available
        if (isAreaAvailable(fileInfo.getStartingBlockIndex() + fileInfo.getFileSize(), extensionBlocks)) {
            for (int i = fileInfo.getStartingBlockIndex() + fileInfo.getFileSize();
                 i < fileInfo.getStartingBlockIndex() + fileInfo.getFileSize() + extensionBlocks; i++) {
                directory[i] = 1 + randomGenerator.nextInt(Integer.MAX_VALUE);
            }
        }
        // if area before the file is available, move the file to lower indices and extend later
        else if (isAreaAvailable(fileInfo.getStartingBlockIndex() - extensionBlocks, extensionBlocks)) {
            for (int i = fileInfo.getStartingBlockIndex() - extensionBlocks;
                 i < fileInfo.getStartingBlockIndex() - extensionBlocks + fileInfo.getFileSize(); i++) {
                directory[i] = directory[i + extensionBlocks];
            }

            for (int i = fileInfo.getStartingBlockIndex() - extensionBlocks + fileInfo.getFileSize();
                 i < fileInfo.getStartingBlockIndex() + fileInfo.getFileSize(); i++) {
                directory[i] = 1 + randomGenerator.nextInt(Integer.MAX_VALUE);
            }
            // update directory table
            fileInfo.setStartingBlockIndex(fileInfo.getStartingBlockIndex() - extensionBlocks);
        }
        // compaction is required
        else {
            // if number of empty blocks before starting index + number of empty blocks directly after the current file
            // is bigger than the extensionBlocks we can get away with directly updating file length info,
            // doing compaction, and then extend later
            // otherwise we will override other files' contents by error while doing compaction

            int emptyBlockCount = 0;
            for (int i = 0; i < fileInfo.getStartingBlockIndex() && emptyBlockCount < extensionBlocks; i++) {
                if (directory[i] == 0)
                    emptyBlockCount++;
            }

            int indexAfterFile = fileInfo.getStartingBlockIndex() + fileInfo.getFileSize();
            while (emptyBlockCount < extensionBlocks) {
                if (directory[indexAfterFile] == 0) {
                    emptyBlockCount++;
                    indexAfterFile++;
                } else {
                    break;
                }
            }

            if (emptyBlockCount >= extensionBlocks) {
                int oldFileSize = fileInfo.getFileSize();
                fileInfo.setFileSize(oldFileSize + extensionBlocks);
                applyCompaction();
                for (int i = fileInfo.getStartingBlockIndex() + oldFileSize;
                     i < fileInfo.getStartingBlockIndex() + fileInfo.getFileSize(); i++) {
                    directory[i] = 1 + randomGenerator.nextInt(Integer.MAX_VALUE);
                }
            }
            // otherwise
            else {
                // we can try to move the file to another place if there is room
                int firstFitIndex = getFirstFitForSize(fileInfo.getFileSize() + extensionBlocks);
                if (firstFitIndex != -1) {
                    for (int i = 0; i < fileInfo.getFileSize(); i++) {
                        directory[firstFitIndex + i] = directory[fileInfo.getStartingBlockIndex() + i];
                        directory[fileInfo.getStartingBlockIndex() + i] = 0;
                    }
                    for (int i = fileInfo.getStartingBlockIndex() + fileInfo.getFileSize();
                         i < fileInfo.getStartingBlockIndex() + fileInfo.getFileSize() + extensionBlocks; i++) {
                        directory[i] = 1 + randomGenerator.nextInt(Integer.MAX_VALUE);
                    }
                    fileInfo.setStartingBlockIndex(firstFitIndex);
                    fileInfo.setFileSize(fileInfo.getFileSize() + extensionBlocks);
                }
                // TODO: I am out of ideas
                else {
                    return false;
                }
            }
        }
        emptyBlockCount -= extensionBlocks;
        return true;
    }

    @Override
    public boolean shrink(int fileId, int shrinkingBlocks) {
        FileEntry fileInfo = directoryTable.get(fileId);

        if (shrinkingBlocks <= 0 || fileInfo == null || fileInfo.getFileSize() <= shrinkingBlocks) {
            return false;
        }

        for (int i = fileInfo.getStartingBlockIndex() + fileInfo.getFileSize() - shrinkingBlocks;
             i < fileInfo.getStartingBlockIndex() + fileInfo.getFileSize(); i++) {
            directory[i] = 0;
        }
        fileInfo.setFileSize(fileInfo.getFileSize() - shrinkingBlocks);
        emptyBlockCount += shrinkingBlocks;
        return true;
    }

    /**
     * Applies compaction from high to low indices.
     */
    private void applyCompaction() {
        final int[] currentIndex = {0};
        directoryTable.values().stream()
                // iterate files in ascending starting block index order
                .sorted(Comparator.comparingInt(FileEntry::getStartingBlockIndex))
                .forEachOrdered(fe -> {
                    for (int i = 0; i < fe.getFileSize(); i++) {
                        // move the file block by block
                        directory[currentIndex[0] + i] = directory[fe.getStartingBlockIndex() + i];
                    }
                    fe.setStartingBlockIndex(currentIndex[0]);
                    currentIndex[0] += fe.getFileSize();
                });
    }

    /**
     * Returns the first fit index in directory for the given file size.
     *
     * @param size File size
     * @return first fit hole starting index (or -1 if no hole is found)
     */
    private int getFirstFitForSize(int size) {
        int holeStartIndex = 0;
        int holeCurrentSize = 0;
        int i = 0;
        while (i < directory.length && holeCurrentSize < size) {
            if (directory[i] == 0) {
                if (holeCurrentSize == 0)
                    holeStartIndex = i;
                holeCurrentSize++;
            } else {
                holeCurrentSize = 0;
            }
            i++;
        }

        return holeCurrentSize == size ? holeStartIndex : -1;
    }

    /**
     * Checks whether given area with starting index and length is completely empty.
     *
     * @param startingIndex starting index of area
     * @param length        length of are
     * @return <code>true</code> if area is available (empty), <code>false</code> otherwise.
     */
    private boolean isAreaAvailable(int startingIndex, int length) {
        if (startingIndex < 0 || startingIndex + length > directory.length) {
            return false;
        }
        for (int i = startingIndex; i < startingIndex + length; i++) {
            if (directory[i] != 0)
                return false;
        }
        return true;
    }


}
