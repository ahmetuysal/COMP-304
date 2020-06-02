/**
 * @author Ahmet Uysal @ahmetuysal
 */
public interface FileSystem {
    boolean createFile(int fileId, int fileLength);

    int access(int fileId, int byteOffset);

    boolean extend(int fileId, int extensionBlocks);

    void shrink(int fileId, int shrinkingBlocks);
}
