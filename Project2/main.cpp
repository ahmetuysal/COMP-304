#include <cstdlib>     /* srand, rand */
#include <ctime>       /* time */
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <sys/time.h>

// function declarations
int pthread_sleep(int seconds);

void parse_command_line_arguments(int argc, char *argv[], int *total_simulation_time, double *landing_plane_prob,
                                  unsigned int *random_seed);


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
    parse_command_line_arguments(argc, argv, &total_simulation_time, &landing_plane_prob, &random_seed);
    std::cout << total_simulation_time << landing_plane_prob << random_seed;
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
 * @param argc argument count
 * @param argv arguments array
 * @param total_simulation_time pointer to total_simulation_time variable
 * @param landing_plane_prob pointer to landing_plane_prob variable
 * @param random_seed pointer to random_seed variable
 */
void parse_command_line_arguments(int argc, char **argv, int *total_simulation_time, double *landing_plane_prob,
                                  unsigned int *random_seed) {
    int simulation_time = 100;
    double probability = 0.5;
    unsigned int seed = time(nullptr);

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
        }
    }

    *total_simulation_time = simulation_time;
    *landing_plane_prob = probability;
    *random_seed = seed;
}

