#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <inttypes.h>

#define TRACK_SIZE 35
#define STEP_TIME 500
#define STEP_LENGTHS_COUNT 8

const float step_lengths[] = {
    0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2
};

struct Racer {
    float position;
    int place;
};

void die(const char *message) {
    if (errno) {
        perror(message);
    } else {
        printf("ERROR: %s\n", message);
    }
    exit(1);
}

void msleep(long msec) {
    struct timespec ts;
    int res;

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
}

void initialize_racers(struct Racer *racers, int racers_count) {
    for (int i = 0; i < racers_count; i++) {
        struct Racer racer = { .position = TRACK_SIZE,.place = 1 };
        racers[i] = racer;
    }
}

void render_race(struct Racer *racers, int racers_count, bool first, bool show_all_flags) {
    int margin_char_count = TRACK_SIZE + 6;

    char *margin = malloc(sizeof(char) * (margin_char_count + 1));
    if (!margin)
        die("Memory error");

    memset(margin, '-', sizeof(char) * margin_char_count);
    margin[margin_char_count] = '\0';

    if (first) {
        printf("%s\n", margin);
    } else {
        int up_lines_count = 2 * racers_count;
        for (int i = 0; i < up_lines_count; i++) {
            printf("\33[A");
        }
    }

    for (int i = 0; i < racers_count; i++) {
        int position = (int) rintf(racers[i].position);

        // Build right side
        int right_side_char_count = TRACK_SIZE - position;

        char *right_side = malloc(sizeof(char) * (right_side_char_count + 1));
        if (!right_side)
            die("Memory error");

        memset(right_side, ' ', sizeof(char) * right_side_char_count);
        right_side[right_side_char_count] = '\0';

        // Build left side
        int left_side_char_count = position;

        char *left_side = malloc(sizeof(char) * (left_side_char_count + 1));
        if (!left_side)
            die("Memory error");

        memset(left_side, ' ', sizeof(char) * left_side_char_count);
        left_side[left_side_char_count] = '\0';

        char *flag;
        if (racers[i].position <= 0 || show_all_flags) {
            int n = i + 1;
            int length = snprintf(NULL, 0, "%d", n) + 1;

            flag = malloc(length);
            if (!flag)
                die("Memory error");

            snprintf(flag, length, "%d", n);
        } else {
            flag = " ";
        }

        printf("[%s]%s🏇%s|\n", flag, left_side, right_side);

        free(right_side);
        free(left_side);

        if (racers[i].position <= 0 || show_all_flags)
            free(flag);

        printf("%s\n", margin);
    }
    free(margin);
}

float get_random_length_step() {
    int rnd_idx = rand() % STEP_LENGTHS_COUNT;
    return step_lengths[rnd_idx];
}

void step_horses(struct Racer *racers, int racers_count) {
    for (int i = 0; i < racers_count; i++) {
        if (racers[i].position > 0) {
            racers[i].position -= get_random_length_step();
            if (racers[i].position < 0) {
                racers[i].position = 0;
            }
        }
    }
}

bool verify_end(struct Racer *racers, int racers_count) {
    for (int i = 0; i < racers_count; i++) {
        if (racers[i].position > 0)
            return false;
    }
    return true;
}

bool verify_finish(struct Racer *racers, int racers_count) {
    int not_finished = 0;
    for (int i = 0; i < racers_count; i++) {
        if (racers[i].position > 0)
            not_finished++;

        if (not_finished > 1)
            return false;
    }
    return true;
}

int compare(const void* a, const void* b) {
    float float_a = * ( (float*) a );
    float float_b = * ( (float*) b );

    if (float_a == float_b) return 0;
    else if (float_a < float_b) return -1;
    else return 1;
}

void set_racers_place(struct Racer *racers, int racers_count) {
    float positions[racers_count];
    for (int k = 0; k < racers_count; k++) {
        positions[k] = racers[k].position;
    }
    qsort(positions, racers_count, sizeof(float), compare);

    float unique_positions[racers_count];
    for (int l = 0; l < racers_count; l++) {
        unique_positions[l] = -1;
    }

    int q = 0;
    for (int m = 0; m < racers_count; m++) {
        bool is_in_unique_positions = false;
        for (int n = 0; n < racers_count; n++) {
            if (positions[m] == unique_positions[n]) {
                is_in_unique_positions = true;
                break;
            }
        }
        if (!is_in_unique_positions) {
            unique_positions[q] = positions[m];
            q++;
        }
    }

    for(int i = 0; i < racers_count; i++) {
        for (int j = 0; j < racers_count; j++) {
            if (unique_positions[i] == racers[j].position)
                racers[j].place = i + 1;
        }
    }
}

void print_results(struct Racer *racers, int racers_count) {
    printf("\nResults:\n");
    for (int i = 0; i < racers_count; i++) {
        printf("%d: %d\n", i + 1, racers[i].place);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        die("usage: horse-race <number-of-horses>");

    int racers_count = (int) strtoumax(argv[1], NULL, 10);
    if (racers_count == UINTMAX_MAX && errno == ERANGE)
        die("Could not convert <number-of-horses> to integer");

    if (racers_count <= 0)
        die("Number of horses must be a positive value");

    srand(time(NULL));

    struct Racer racers[racers_count];
    initialize_racers(racers, racers_count);

    render_race(racers, racers_count, true, false);
    while (true) {
        step_horses(racers, racers_count);

        msleep(STEP_TIME);
        if (verify_end(racers, racers_count)) {
            render_race(racers, racers_count, false, true);
            break;
        } else if (verify_finish(racers, racers_count)) {
            render_race(racers, racers_count, false, true);
        } else {
            set_racers_place(racers, racers_count);
            render_race(racers, racers_count, false, false);
        }
    }

    print_results(racers, racers_count);
}
