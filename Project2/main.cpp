#include <cstdlib>     /* srand, rand */
#include <ctime>       /* time */
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <string>
#include <sys/time.h>
#include <queue>
#include <pthread.h>

// type definitions
class Plane {
public:
    unsigned int id;
    time_t arrival_time;

    Plane(unsigned int _id, time_t _arrival_time) {
        id = _id;
        arrival_time = _arrival_time;
    }

    // lock
    // condition
};

class ConcurrentPlaneQueue {
private:
    std::queue<Plane> queue;
    pthread_mutex_t lock{};

    std::string queue_to_string(std::queue<Plane> q) {
        std::string str_rep;
        while (!q.empty()) {
            str_rep += std::to_string(q.front().id) + " ";
            q.pop();
        }
        return str_rep;
    }

public:
    void push(const Plane &plane) {
        pthread_mutex_lock(&lock);
        queue.push(plane);
        pthread_mutex_unlock(&lock);
    }

    Plane &front() {
        pthread_mutex_lock(&lock);
        Plane &return_val = queue.front();
        pthread_mutex_unlock(&lock);
        return return_val;
    }

    Plane &front_and_pop() {
        pthread_mutex_lock(&lock);
        Plane &return_val = queue.front();
        queue.pop();
        pthread_mutex_unlock(&lock);
        return return_val;
    }

    void pop() {
        pthread_mutex_lock(&lock);
        queue.pop();
        pthread_mutex_unlock(&lock);
    }

    bool empty() {
        pthread_mutex_lock(&lock);
        bool return_val = queue.empty();
        pthread_mutex_unlock(&lock);
        return return_val;
    }

    int size() {
        pthread_mutex_lock(&lock);
        int return_val = queue.size();
        pthread_mutex_unlock(&lock);
        return return_val;
    }

    std::string to_string() {
        pthread_mutex_lock(&lock);
        std::string str_rep = queue_to_string(queue);
        pthread_mutex_unlock(&lock);
        return str_rep;
    }

    ConcurrentPlaneQueue() {
        lock = PTHREAD_MUTEX_INITIALIZER;
    }
};

enum PlaneType {
    LANDING, DEPARTING, EMERGENCY
};


// function declarations
int pthread_sleep(int seconds);

void parse_command_line_arguments(int argc, char *argv[], int *total_simulation_time, double *landing_plane_prob,
                                  unsigned int *random_seed, int *queue_log_start_time);

void create_plane(PlaneType planeType, time_t currentTime);

void *air_traffic_control_main(void *);

void *landing_plane_main(void *id);

void *departing_plane_main(void *id);

unsigned int generate_unique_plane_id(PlaneType planeType);

double random_double();

// global variables
ConcurrentPlaneQueue landing_queue;
ConcurrentPlaneQueue departing_queue;
ConcurrentPlaneQueue emergency_queue;
time_t simulation_end_time;
time_t simulation_start_time;
pthread_mutex_t first_plane_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t iteration_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t iteration_cond = PTHREAD_COND_INITIALIZER;
unsigned int current_plane_id = -1;
FILE *plane_log_file;
FILE *queue_log_file;

/**
 * This program simulates an air traffic controller.
 * @author Ahmet Uysal (@ahmetuysal)
 * @param argc argument count
 * @param argv command line arguments
 * @return
 */
int main(int argc, char *argv[]) {
    int total_simulation_time;
    double landing_plane_prob;
    unsigned int random_seed;
    int queue_log_start_time;
    parse_command_line_arguments(argc, argv, &total_simulation_time, &landing_plane_prob, &random_seed,
                                 &queue_log_start_time);
    struct timeval tp{};
    gettimeofday(&tp, nullptr);
    simulation_start_time = tp.tv_sec;
    simulation_end_time = tp.tv_sec + total_simulation_time;
    time_t next_emergency_time = tp.tv_sec + 40;
    plane_log_file = fopen("./planes.log", "w+");
    queue_log_file = fopen("./queues.log", "w+");
    fprintf(plane_log_file, "PlaneID\tStatus\tRequest Time\tRunway Time\tTurnaround Time\n");
    srand(random_seed);

    pthread_mutex_lock(&first_plane_mutex);

    create_plane(LANDING, tp.tv_sec);
    create_plane(DEPARTING, tp.tv_sec);
    pthread_t air_controller_thread;
    pthread_create(&air_controller_thread, nullptr, air_traffic_control_main, nullptr);

    while (tp.tv_sec < simulation_end_time) {
        if (tp.tv_sec >= queue_log_start_time + simulation_start_time) {
            fprintf(queue_log_file, "At %ld sec departing queue: %s\n",
                    tp.tv_sec - simulation_start_time,
                    departing_queue.to_string().c_str());
            fprintf(queue_log_file, "At %ld sec landing queue: %s\n",
                    tp.tv_sec - simulation_start_time,
                    landing_queue.to_string().c_str());
            fprintf(queue_log_file, "At %ld sec emergency queue: %s\n",
                    tp.tv_sec - simulation_start_time,
                    emergency_queue.to_string().c_str());
        }
        pthread_sleep(1);
        gettimeofday(&tp, nullptr);

        double random_ = random_double();
        if (random_ <= landing_plane_prob) {
            create_plane(LANDING, tp.tv_sec);
        }
        if (random_ <= 1 - landing_plane_prob) {
            create_plane(DEPARTING, tp.tv_sec);
        }
        if (tp.tv_sec >= next_emergency_time) {
            create_plane(EMERGENCY, tp.tv_sec);
            next_emergency_time += 40;
        }

    }

    fclose(plane_log_file);
    fclose(queue_log_file);
    return 0;
}

// function implementations

/******************************************************************************
 pthread_sleep takes an integer number of seconds to pause the current thread
 original by Yingwu Zhu
 updated by Muhammed Nufail Farooqi
 *****************************************************************************/
int pthread_sleep(int seconds) {
    pthread_mutex_t mutex;
    pthread_cond_t condition_var;
    struct timespec time_to_expire{};
    if (pthread_mutex_init(&mutex, nullptr)) {
        return -1;
    }
    if (pthread_cond_init(&condition_var, nullptr)) {
        return -1;
    }
    struct timeval tp{};
    //When to expire is an absolute time, so get the current time and add //it to our delay time
    gettimeofday(&tp, nullptr);
    time_to_expire.tv_sec = tp.tv_sec + seconds;
    time_to_expire.tv_nsec = tp.tv_usec * 1000;

    pthread_mutex_lock(&mutex);
    int res = pthread_cond_timedwait(&condition_var, &mutex, &time_to_expire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition_var);

    //Upon successful completion, a value of zero shall be returned
    return res;
}

/**
 * Parses given command line arguments. Accepts only three different arguments and discards other arguments.
 * Allowed arguments:
 * -s (total simulation time): a positive integer value that sets the total simulation time. 1000 is used by default if
 * this argument is omitted or illegal
 * -p (probability): a double value between [0-1] (inclusive) that sets probability of a landing plane arriving. This
 * value is also used for determining the probability of a plane getting ready for take-off with probability 1-p.
 * 0.5 is used by default if this argument is omitted or illegal
 * -r (random seed): an integer value to set a seed to random generator. current calendar time is used by default if
 * this argument is omitted or illegal
 * -n (queue log start time): an integer value that controls the second at which program starts to log content of queues
 * to a text file
 * @param argc argument count
 * @param argv arguments array
 * @param total_simulation_time pointer to total_simulation_time variable
 * @param landing_plane_prob pointer to landing_plane_prob variable
 * @param random_seed pointer to random_seed variable
 * @param queue_log_start_time pointer to queue_log_start_time
 */
void parse_command_line_arguments(int argc, char **argv, int *total_simulation_time, double *landing_plane_prob,
                                  unsigned int *random_seed, int *queue_log_start_time) {
    int simulation_time = 100;
    double probability = 0.5;
    unsigned int seed = time(nullptr);
    int queue_log_start = 0;

    for (int i = 1; i < argc - 1; i++) {
        if (strcmp("-s", argv[i]) == 0) {
            int time_arg = atoi(argv[i + 1]);
            if (time_arg > 0) {
                simulation_time = time_arg;
            } else {
                std::cout << "You entered an illegal value for simulation time, " << simulation_time
                          << " will be used as default" << std::endl;
            }
        } else if (strcmp("-p", argv[i]) == 0) {
            double prob_arg = atof(argv[i + 1]);
            // user entered 0 as probability
            if (strcmp(argv[i + 1], "0") == 0) {
                probability = 0;
            } else if (prob_arg <= 0.0 || prob_arg > 1.0) {
                std::cout << "You entered an illegal value for probability, " << probability
                          << " will be used as default" << std::endl;
            } else {
                probability = prob_arg;
            }
        } else if (strcmp("-r", argv[i]) == 0) {
            try {
                unsigned long seed_arg_long = std::stoul(argv[i + 1], nullptr, 10);
                auto seed_arg = (unsigned int) seed_arg_long;
                if (seed_arg_long != seed_arg) throw std::out_of_range("");
                seed = seed_arg;
            }
            catch (...) {
                std::cout << "You entered an illegal value for seed, " << seed
                          << " will be used as default" << std::endl;
            }
        } else if (strcmp("-n", argv[i]) == 0) {
            int queue_log_start_arg = atoi(argv[i + 1]);
            // user entered 0 as queue_log_start
            if (strcmp(argv[i + 1], "0") == 0) {
                queue_log_start = 0;
            } else if (queue_log_start_arg != 0) {
                queue_log_start = queue_log_start_arg;
            } else {
                std::cout
                        << "You entered an illegal value for queue log starting time, logging will start at time 0 by default"
                        << std::endl;
            }
        }
    }

    *total_simulation_time = simulation_time;
    *landing_plane_prob = probability;
    *random_seed = seed;
    *queue_log_start_time = queue_log_start;
}


unsigned int generate_unique_plane_id(PlaneType planeType) {
    static unsigned int nextDepartingId = -1;
    static unsigned int nextLandingId = -2;

    if (planeType == DEPARTING) {
        return nextDepartingId += 2;
    } else {
        return nextLandingId += 2;
    }
}

double random_double() {
    return (double) rand() / RAND_MAX;
}

void create_plane(PlaneType planeType, time_t currentTime) {

    unsigned int plane_id = generate_unique_plane_id(planeType);
    Plane new_plane = Plane(plane_id, currentTime);
    pthread_t new_plane_thread;
    unsigned int *plane_id_ptr = (unsigned int *) malloc(sizeof(unsigned int));
    *plane_id_ptr = plane_id;
    switch (planeType) {
        case DEPARTING:
            departing_queue.push(new_plane);
            pthread_create(&new_plane_thread, nullptr, departing_plane_main, (void *) plane_id_ptr);
            break;
        case LANDING:
            landing_queue.push(new_plane);
            pthread_create(&new_plane_thread, nullptr, landing_plane_main, (void *) plane_id_ptr);
            break;
        case EMERGENCY:
            emergency_queue.push(new_plane);
            pthread_create(&new_plane_thread, nullptr, landing_plane_main, (void *) plane_id_ptr);
    }
}


void *air_traffic_control_main(void *) {
    // wait for the first plane
    pthread_mutex_lock(&first_plane_mutex);
    struct timeval tp{};
    gettimeofday(&tp, nullptr);
    while (tp.tv_sec < simulation_end_time) {
        if (!emergency_queue.empty()) {
            Plane plane = emergency_queue.front_and_pop();
            current_plane_id = plane.id;
            pthread_cond_broadcast(&iteration_cond);
            fprintf(plane_log_file, "%d\t%c\t%ld\t%ld\t%ld\n",
                    plane.id,
                    'E',
                    plane.arrival_time - simulation_start_time,
                    tp.tv_sec - simulation_start_time,
                    tp.tv_sec - plane.arrival_time + 2);
        } else if (landing_queue.empty() ||
                   (departing_queue.size() >= 5 &&
                    departing_queue.front().arrival_time < landing_queue.front().arrival_time)) {
            Plane plane = departing_queue.front_and_pop();
            current_plane_id = plane.id;
            fprintf(plane_log_file, "%d\t%c\t%ld\t%ld\t%ld\n",
                    plane.id,
                    'D',
                    plane.arrival_time - simulation_start_time,
                    tp.tv_sec - simulation_start_time,
                    tp.tv_sec - plane.arrival_time + 2);
        } else {
            Plane plane = landing_queue.front_and_pop();
            current_plane_id = plane.id;
            fprintf(plane_log_file, "%d\t%c\t%ld\t%ld\t%ld\n",
                    plane.id,
                    'L',
                    plane.arrival_time - simulation_start_time,
                    tp.tv_sec - simulation_start_time,
                    tp.tv_sec - plane.arrival_time + 2);
        }
        pthread_sleep(2);
        gettimeofday(&tp, nullptr);
    }
    pthread_exit(0);
}

void *landing_plane_main(void *id) {
    unsigned int my_id = *((unsigned int *) id);
    if (my_id == 0) {
        // send first plane signal
        pthread_mutex_unlock(&first_plane_mutex);
    }

    // wait for air traffic controller
    pthread_mutex_lock(&iteration_lock);
    while (current_plane_id != my_id) {
        pthread_cond_wait(&iteration_cond, &iteration_lock);
    }
    pthread_mutex_unlock(&iteration_lock);
    pthread_exit(0);
}

void *departing_plane_main(void *id) {
    unsigned int my_id = *((unsigned int *) id);
    // wait for air traffic controller
    pthread_mutex_lock(&iteration_lock);
    while (current_plane_id != my_id) {
        pthread_cond_wait(&iteration_cond, &iteration_lock);
    }
    pthread_mutex_unlock(&iteration_lock);
    pthread_exit(0);
}
