import java.util.HashMap;
import java.util.Map;

/**
 * @author Ahmet Uysal @ahmetuysal
 */
public class LinkedAllocationFileSystem implements FileSystem {

    private final Map<Integer, FileEntry> directoryTable;
    private final int[] directory;
    private int emptyBlockCount;

    public LinkedAllocationFileSystem(int directorySize) {
        this.directoryTable = new HashMap<>();
        this.directory = new int[directorySize];
        this.emptyBlockCount = directorySize;
    }

    @Override
    public boolean createFile(int fileId, int fileLength) {
        if (emptyBlockCount < fileLength)
            return false;

        // allocate
        emptyBlockCount -= fileLength;
        return true;
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
