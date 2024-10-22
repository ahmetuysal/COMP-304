/**
 * @author Ahmet Uysal @ahmetuysal
 */
public class FileEntry {
    private final int fileId;
    private int startingBlockIndex;
    // file size in Blocks
    private int fileSize;

    public FileEntry(int fileId, int startingBlockIndex, int fileSize) {
        this.fileId = fileId;
        this.startingBlockIndex = startingBlockIndex;
        this.fileSize = fileSize;
    }

    public int getFileId() {
        return fileId;
    }

    public int getStartingBlockIndex() {
        return startingBlockIndex;
    }

    public void setStartingBlockIndex(int startingBlockIndex) {
        this.startingBlockIndex = startingBlockIndex;
    }

    public int getFileSize() {
        return fileSize;
    }

    public void setFileSize(int fileSize) {
        this.fileSize = fileSize;
    }


    @Override
    public String toString() {
        return "FileEntry{" +
                "fileId=" + fileId +
                ", startingBlockIndex=" + startingBlockIndex +
                ", fileSize=" + fileSize +
                '}';
    }
}
