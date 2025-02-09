/* main program simulating a temperature sensor */

#include <czmq.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

time_t get_current_time()
{
    time_t current_time = time(NULL);
    time(&current_time);
    return current_time;
}

time_t time_since_start(time_t start_time)
{
    return get_current_time() - start_time;
}

float generate_temperature_value()
{
    return (float)rand()/RAND_MAX * 100;
}

char* data_stream_to_send(time_t time, float temperature)
{
    char delimiter[] = "//#";
    char *time_str = malloc(20);
    time_t current_time = get_current_time();
    sprintf(time_str, "%s%ld%s%ld", delimiter, current_time, delimiter, time);

    char *temperature_str = malloc(12);
    if (temperature_str == NULL) {
        return NULL; // Handle memory allocation failure
    }
    
    sprintf(temperature_str, "%s%.2f", delimiter, temperature);

    char *result = malloc(strlen(time_str) + strlen(temperature_str) + 1);
    if (result == NULL) {
        free(temperature_str); // Handle memory allocation failure
        return NULL;
    }

    strcpy(result, time_str);
    strcat(result, temperature_str);

    free(time_str); // Free the allocated memory for time_str
    free(temperature_str); // Free the allocated memory for temperature_str
    return result;
}

// Global variables
pthread_t temp_thread_id;
volatile int temp_sensor_running = 1;
pthread_mutex_t lock; //worker_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
int sleep_time = 5;

// temp sensor thread function
void* temperature_sensor_simulator(void* arg) {
    printf("temp sensor started.\n");
    
    zsock_t *push = zsock_new_push ("tcp://localhost:5555");
    time_t start_time = get_current_time();
    srand(time(NULL));

    while (1) 
    {
        sleep (sleep_time);   //  delay for sleep_time seconds: default is 5s

        time_t current_time = time_since_start(start_time);
        float temperature = generate_temperature_value();
        char *sensor_data = data_stream_to_send(current_time, temperature);

        //printf ("Sending data: %s\n", sensor_data);

        zstr_send (push, sensor_data);
        free(sensor_data); // Free the allocated memory for sensor_data

        // Check if the worker thread should keep running
        pthread_mutex_lock(&lock);
        if (!temp_sensor_running) {
            pthread_mutex_unlock(&lock);
            printf("Temp sensor stopping.\n");
            zsock_destroy (&push);

            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&lock);

    }
}

// config thread
void* config_thread_func(void* arg) {
    
    zsock_t *pull = zsock_new_pull ("tcp://localhost:5556");
    zsock_connect (pull, "tcp://localhost:5556");

    // Start the temp sensor thread by default
    if (pthread_create(&temp_thread_id, NULL, temperature_sensor_simulator, NULL) != 0) {
        perror("Failed to create temp thread");
        exit(EXIT_FAILURE);
    }
    
    printf("Wait for config to be recvd. options -> (freq num/start/stop/exit): ");
    
    while (1) {
        
        char *command = zstr_recv (pull);
        printf ("Received: %s\n", command);
        char *token;
        char freq[10];
        int value;

        token = strtok(command, " ");
        if(token != NULL && strcmp(token, "freq") == 0) {
            
            strcpy(freq, token);
            token = strtok(NULL, " ");
            if(token != NULL) {
                value = atoi(token);
                if(value > 0 && value < 11) {
                    sleep_time = value;
                    printf("Info: changing sensor frequency to %d\n", sleep_time);
                } else {
                    printf("Error: frequency value should be between 1 and 10\n");
                }
            }
        } else if (strcmp(command, "start") == 0) {
            pthread_mutex_lock(&lock);
            if (!temp_sensor_running) {
                temp_sensor_running = 1;
                pthread_mutex_unlock(&lock);

                // Start the temp sensor thread
                if (pthread_create(&temp_thread_id, NULL, temperature_sensor_simulator, NULL) != 0) {
                    perror("Failed to create temp thread");
                    exit(EXIT_FAILURE);
                }
            } else {
                pthread_mutex_unlock(&lock);
                printf("Temp thread is already running.\n");
            }
        } else if (strcmp(command, "stop") == 0) {
            pthread_mutex_lock(&lock);
            if (temp_sensor_running) {
                temp_sensor_running = 0;
                pthread_mutex_unlock(&lock);

                // Wait for the worker thread to exit
                pthread_join(temp_thread_id, NULL);
                printf("Temp thread stopped.\n");
            } else {
                pthread_mutex_unlock(&lock);
                printf("Temp thread is not running.\n");
            }
        } else if (strcmp(command, "exit") == 0) {
            pthread_mutex_lock(&lock);
            if (temp_sensor_running) {
                temp_sensor_running = 0;
                pthread_mutex_unlock(&lock);

                // Wait for the worker thread to exit
                pthread_join(temp_thread_id, NULL);
                printf("Temp thread stopped.\n");
            } else {
                pthread_mutex_unlock(&lock);
            }
            printf("Stopping config thread.\n");
            zsock_destroy (&pull);
            pthread_exit(NULL);
        } else {
            printf("Invalid command.\n");
        } // end of if-else

        zstr_free (&command);

    } // end of while loop
}

int main (void)
{
    printf ("Initializing temperature sensor\n");

    pthread_t config_thread_id;

    // Initialize the mutex
    if (pthread_mutex_init(&lock, NULL) != 0) {
        perror("Mutex initialization failed");
        return EXIT_FAILURE;
    }

    // Start the config thread
    if (pthread_create(&config_thread_id, NULL, config_thread_func, NULL) != 0) {
        perror("Failed to create config thread");
        return EXIT_FAILURE;
    }

    // Wait for the config thread to finish
    pthread_join(config_thread_id, NULL);

    // Destroy the mutex
    pthread_mutex_destroy(&lock);

    printf("Program exiting.\n");

    return 0;
}
