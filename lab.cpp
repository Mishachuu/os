#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

void send_value(int sock, double value) {
    write(sock, &value, sizeof(value));
}

double recv_value(int sock) {
    double value;
    read(sock, &value, sizeof(value));
    return value;
}

void* sum_thread(void* arg) {
    int sockfd = *(int*)arg;
    double values[3], sum = 0;
    read(sockfd, values, sizeof(values));

    for (int i = 0; i < 3; i++) {
        sum += values[i];
    }

    send_value(sockfd, sum);
    return NULL;
}

void* sub_thread(void* arg) {
    int sockfd = *(int*)arg;
    double sum = recv_value(sockfd);
    double mean = sum / 3;
    double diffs[3];
    
    read(sockfd, diffs, sizeof(diffs));
    for (int i = 0; i < 3; i++) {
        diffs[i] -= mean;
        send_value(sockfd, diffs[i]);
    }
    return NULL;
}

void* mul_thread(void* arg) {
    int sockfd = *(int*)arg;
    double squared;
    for (int i = 0; i < 3; i++) {
        double diff = recv_value(sockfd);
        squared = diff * diff;
        send_value(sockfd, squared);
    }
    return NULL;
}

void* div_thread(void* arg) {
    int sockfd = *(int*)arg;
    double squares[3], sum_squares = 0;
    for (int i = 0; i < 3; i++) {
        squares[i] = recv_value(sockfd);
        sum_squares += squares[i];
    }
    double variance = sum_squares / 2;
    printf("Variance: %f\n", variance);
    return NULL;
}

int main() {
    int sv[4][2];  // Socket pairs for communication between threads
    for (int i = 0; i < 4; i++) {
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv[i]) != 0) {
            perror("socketpair");
            exit(EXIT_FAILURE);
        }
    }

    pthread_t t1, t2, t3, t4;
    double values[3] = {10.0, 20.0, 30.0};

    write(sv[0][1], values, sizeof(values));
    pthread_create(&t1, NULL, sum_thread, &sv[0][0]);
    pthread_create(&t2, NULL, sub_thread, &sv[1][0]);
    pthread_create(&t3, NULL, mul_thread, &sv[2][0]);
    pthread_create(&t4, NULL, div_thread, &sv[3][0]);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    for (int i = 0; i < 4; i++) {
        close(sv[i][0]);
        close(sv[i][1]);
    }

    return 0;
}
