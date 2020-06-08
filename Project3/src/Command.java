/**
 * @author Ahmet Uysal @ahmetuysal
 */
public class Command {
    private final String command;
    private final int argument1;
    private final int argument2;

    public Command(String command, int argument1, int argument2) {
        this.command = command;
        this.argument1 = argument1;
        this.argument2 = argument2;
    }

    public String getCommand() {
        return command;
    }

    public int getArgument1() {
        return argument1;
    }

    public int getArgument2() {
        return argument2;
    }

    @Override
    public String toString() {
        return "Command{" +
                "command='" + command + '\'' +
                ", argument1=" + argument1 +
                ", argument2=" + argument2 +
                '}';
    }
}
