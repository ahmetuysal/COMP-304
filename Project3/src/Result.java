/**
 * @author Ahmet Uysal @ahmetuysal
 */
public class Result {
    final int numberOfOperations;
    final int rejectedCreations;
    final int rejectedExtensions;
    final int rejectedShrinks;
    // in nanoseconds
    final long runtime;

    public Result(int numberOfOperations, int rejectedCreations, int rejectedExtensions, int rejectedShrinks, long runtime) {
        this.numberOfOperations = numberOfOperations;
        this.rejectedCreations = rejectedCreations;
        this.rejectedExtensions = rejectedExtensions;
        this.rejectedShrinks = rejectedShrinks;
        this.runtime = runtime;
    }

    public int getNumberOfOperations() {
        return numberOfOperations;
    }

    public int getRejectedCreations() {
        return rejectedCreations;
    }

    public int getRejectedExtensions() {
        return rejectedExtensions;
    }

    public int getRejectedShrinks() {
        return rejectedShrinks;
    }

    public long getRuntime() {
        return runtime;
    }

    @Override
    public String toString() {
        return "Result{" +
                "numberOfOperations=" + numberOfOperations +
                ", rejectedCreations=" + rejectedCreations +
                ", rejectedExtensions=" + rejectedExtensions +
                ", rejectedShrinks=" + rejectedShrinks +
                ", runtime=" + runtime +
                '}';
    }
}
