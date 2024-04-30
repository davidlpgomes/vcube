#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "smpl.h"


#define TEST 1
#define FAULT 2
#define RECOVERY 3


typedef struct {
    int id;
    int *state;
} Process;

Process *procs;


int get_next_proc(int N, int self) {
    return self + 1 < N ? self + 1 : 0;
}

void c(int i, int s, int *v, int j) {
    if (i == -1)
        return;

    v[j] = i ^ (int) pow(2, s - 1);

    for (int k = 1; k < s; k++)
        c(v[j], k, v, j + 1);

    return;
}

int first_correct(int j, int s) {
    int size = pow(2, s - 1);
    int cjs[size];

    c(j, s, &cjs, 0);

    int first_correct = -1;

    for (int k = 0; k < size; k++)
        if (!status(procs[cjs[k]].id))
            return cjs[k];

    return -1;
}

int *alloc_state(int N, int self) {
    int *state = (int *) malloc(sizeof(int) * N);

    for (int i = 0; i < N; i++)
        state[i] = -1;

    state[self] = 0;

    return state;
}

void free_procs(Process *procs, int n) {
    for (int i = 0; i < n; i++)
        free(procs[i].state);

    free(procs);

    return;
}

int main(int argc, char *argv[]) {
    int token, event, r, t;
    char fa_name[5];

    if (argc != 3) {
        puts("Correct usage: ./vcube <num procs> <max time>\n");
        exit(1);
    }

    int n = atoi(argv[1]);
    int max_time = atoi(argv[2]);

    smpl(0, "A VCube implementation");
    reset();
    stream(1);

    procs = (Process*) malloc(sizeof(Process) * n);

    for (int i = 0; i < n; i++) {
        memset(fa_name, '\0', 5);
        sprintf(fa_name, "%d", i);

        procs[i].id = facility(fa_name, 1);
        procs[i].state = alloc_state(n, i);
    }

    for (int i = 0; i < n; i++)
        schedule(TEST, 30.0, i);

    schedule(FAULT, 31.0, 1);
    schedule(RECOVERY, 61.0, 1);

    while (time() < max_time) {
        cause(&event, &token);

        switch (event) {
            case TEST:
                if (status(procs[token].id) != 0)
                    break;

                printf(
                    "Sou o processo %d e estou testando no tempo %4.1f\n",
                    token, time()
                );

                int n_log = log2(n);

                for (int s = 1; s <= n_log; s++) {
                    int size = pow(2, s - 1);
                    int cis[size];

                    c(token, s, &cis, 0);

                    for (int j = 0; j < size; j++) {
                        int fc = first_correct(j, s);
                        if (fc != token && fc != -1)
                            continue;

                        printf("\tSou o primeiro processo correto em %d\n", j);

                        if (!status(procs[j].id)) {
                            printf("\t\tProcesso %d: CORRETO\n", j);
                            if (procs[token].state[j] % 2 == 1)
                                procs[token].state[j]++;

                            for (int i = 0; i < n; i++)
                                if (procs[j].state[i] > procs[token].state[i])
                                    procs[token].state[i] = procs[j].state[i];
                        } else {
                            printf("\t\tProcesso %d: FALHO\n", j);
                            if (procs[token].state[j] % 2 == 0)
                                procs[token].state[j]++;
                        }
                    }
                }

                printf("\tSTATE[]:\n");

                for (int i = 0; i < n; i++) {
                    printf("\t\t%d: ", i);

                    int s = procs[token].state[i];

                    if (s == -1)
                        printf("UNKNOWN\n");
                    else if (s % 2 == 0)
                        printf("CORRETO\n");
                    else if (s % 2 == 1)
                        printf("FALHO\n");
                }

                schedule(TEST, 30.0, token);

                break;
            case FAULT:
                r = request(procs[token].id, token, 0);

                printf(
                    "Sou o processo %d e estou falhando no tempo %4.1f\n",
                    token, time()
                );

                break;
            case RECOVERY:
                release(procs[token].id, token);

                printf(
                    "Sou o processo %d e estou recuperando no tempo %4.1f\n",
                    token, time()
                );

                schedule(TEST, 1.0, token);

                break;
        }
    }

    free_procs(procs, n);
}
