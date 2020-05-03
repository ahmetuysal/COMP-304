# COMP 304 - Project II

**Author**: Ahmet Uysal (AUYSAL16)

All parts of my implementation work as described in the project description.

## Overall Program Structure

I implemented my program on top of ideas provided in lectures, PS slides and provided pseudocode.

In order to ensure first plane awakes air traffic control thread, I created a mutex named `first_plane_mutex` which is initially locked. Air traffic control thread tries to lock this mutex before doing anything. This mutex is unlocked at the first thread of first plane (landing plane with id 0). Therefore, it is guaranteed that air traffic control thread will wait for the creation of first plane before executing.

Another challenge was to signal plane thread once they are popped from queues. To accomplish this I used a shared variable named `current_plane_id` that stores id of the plane that is last popped from a queue. Air controller thread sends a broadcast using `iteration_lock` and `iteration_cond`. Every plane thread waits until their id is equal to `current_plane_id` using implementation given below.

```c++
// wait for air traffic controller
pthread_mutex_lock(&iteration_lock);
while (current_plane_id != my_id) {
    pthread_cond_wait(&iteration_cond, &iteration_lock);
}
pthread_mutex_unlock(&iteration_lock);
```

### `ConcurrentPlaneQueue` Class

In order to abstract away the details of concurrent queue access, I created a class that encapsulates queue structure with an additional lock.

## Part II: How to solve Starvation

The given algorithm for part II is favoring the landing planes until one the below conditions hold:

- The `landing_queue` is empty
- There are at least 5 planes in the `departing_queue`

This algorithm causes starvation since it is possible that this algorithm only allows planes to depart and wait landing planes indefinitely. We can observe this problem by running the simulation with a low `landing_plane_prob` (and therefore a high departing plane probability) like -p=0.01.

To solve this problem, I added an extra constraint to algorithm.

Landing planes are favored unless one of the conditions hold:

- The `landing_queue` is empty.
- There is at least 5 planes in the `departing_queue`and the first plane at the `departing_queue` has a lower `arrival_time` than the first plane at the `landing_queue`.

## Output files

Log output files for the program run with flags `-s 60 -p 0.5 -r 60780 -n 0` are provided in `planes.log` and `queues.log` files.
