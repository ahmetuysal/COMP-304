import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class Main {

    private static final int DIRECTORY_SIZE = 32768;


    public static void main(String[] args) {
        // write your code here
        String fileName = "input_1024_200_5_9_9.txt";
        // "input_8_600_5_5_0.txt";
        // "input_1024_200_5_9_9.txt";
        // "input_1024_200_9_0_0.txt";
        // "input_1024_200_9_0_9.txt";
        // "input_2048_600_5_5_0.txt";
        int blockSize = 1024;

        List<Command> commandList = new ArrayList<>();
        try {
            BufferedReader fileReader = new BufferedReader(new FileReader("src/io/" + fileName));
            String line;
            while ((line = fileReader.readLine()) != null) {
                if (!line.isBlank()) {
                    String[] arguments = line.split(":");
                    commandList.add(new Command(arguments[0], Integer.parseInt(arguments[1]), arguments.length > 2 ? Integer.parseInt(arguments[2]) : -1));
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        List<Result> results = new ArrayList<>();

        for (int i = -5; i < 10; i++) {
            FileSystem fs;

            if (i < 5) {
                fs = new LinkedAllocationFileSystem(DIRECTORY_SIZE, blockSize);
            } else {
                fs = new ContiguousAllocationFileSystem(DIRECTORY_SIZE, blockSize);
            }

            int fileId = 0;
            int rejectedCreations = 0;
            int rejectedExtensions = 0;
            int rejectedShrinks = 0;
            final long startTime = System.nanoTime();
            for (Command cmd : commandList) {
                System.out.println(cmd);
                switch (cmd.getCommand()) {
                    case "c":
                        if (fs.createFile(fileId, cmd.getArgument1())) {
                            fileId++;
                        } else {
                            rejectedCreations++;
                            System.out.println("Rejected create");
                        }
                        break;
                    case "a":
                        fs.access(cmd.getArgument1(), cmd.getArgument2());
                        break;
                    case "e":
                        if (!fs.extend(cmd.getArgument1(), cmd.getArgument2())) {
                            rejectedExtensions++;
                            System.out.println("Rejected extension");
                        }
                        break;
                    case "sh":
                        if (!fs.shrink(cmd.getArgument1(), cmd.getArgument2())) {
                            rejectedShrinks++;
                        }
                        break;
                    default:
                        System.err.println("Unknown command " + cmd.getCommand());
                }
                if (fs instanceof LinkedAllocationFileSystem) {
                    ((LinkedAllocationFileSystem) fs).checkAllChunks();
                }
            }
            final long duration = System.nanoTime() - startTime;
            if (i >= 0)
                results.add(new Result(commandList.size(), rejectedCreations, rejectedExtensions, rejectedShrinks, duration));
        }

        System.out.println(results.toString());
    }
}
