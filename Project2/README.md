# COMP 304 - Project II
**Author**: Ahmet Uysal (AUYSAL16)

All parts of my implementation work as described in the project description.

## Part II: How to solve Starvation

The given algorithm for part II is favoring the landing planes until one the below conditions hold:
- The `landing_queue` is empty 
- There are at least 5 planes in the `departing_queue`

This algorithm causes starvation since it is possible that this algorithm only allows planes to depart and wait landing planes indefinitely. We can observe this problem by running the simulation with a low `landing_plane_prob` (and therefore a high departing plane probability) like -p=0.01.

To solve this problem, I added an extra constraint to algorithm.

Landing planes are favored unless one of the conditions hold:
- The `landing_queue` is empty.
- There is at least 5 planes in the `departing_queue`and the first plane at the `departing_queue` has a lower `arrival_time` than the first plane at the `landing_queue`. 