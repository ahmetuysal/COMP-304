/**
 * @author Ahmet Uysal @ahmetuysal
 */
public interface FileSystem {
    boolean createFile(int fileId, int fileLength);

    /**
     * Returns the location of the byte having the given offset in the directory, where byte offset is the offset of
     * that byte from the beginning of the file.
     *
     * @param fileId     id of the requested file
     * @param byteOffset file offset in bytes
     * @return the location (block) of the byte having the given offset in the directory
     */
    int access(int fileId, int byteOffset);

    boolean extend(int fileId, int extensionBlocks);

    boolean shrink(int fileId, int shrinkingBlocks);
}
