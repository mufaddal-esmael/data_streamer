INTRODUCTION:

This program prototypes a sensor system that sends data using ZeroMQ.
It comprises of the following:
- a sensor client for sending data
- a server for collecting sensor data


INSTALLATION:

This program was built on a mac.  The compiled binaries are provided as well.
To generate the client binary built in C run gcc client.c -o client -I /opt/homebrew/include -L /opt/homebrew/lib -l czmq from the command line.

The python server can be run in a separate terminal using: python server.py


OVERVIEW:

Sensor data format: 
The sensor program simulates sending temperature sensor data as a string with a delimiter to separate the timestamp, time since sensor started and the sensor temperature reading accurate to two decimal places.  The server receives and displays this data.  The simplified format was used given the nature of the setup and can be expanded to use other formats such as JSON or protobuf to standardize receiving data from various sensors with multiple parameters.  

I did not attempt to include sensor identification information which would be useful in case of multiple sensors.  Also using a pub-sub approach would allow for easier scalability without introducing much complexity into the program.


Sensor config format:
The same command line displaying the sensor information can be used to send commands to change the sensor configuration.

The following are commands that can be used to change the sensor configuration:
freq <value> <- the tag "freq" followed by space and a value between 1 and 10
start <- to start the sensor client
stop <-  to stop the sensor client
exit <-  to shut down the sensor program


API REFERENCE:

Server:
def parse_message(message: str) -> list:
- to parse the sensor data received

def receive_data():
- function for receiving sensor data and formatting 

def send_command():
- function for processing config commands and send them to the client

Client:
char* data_stream_to_send(time_t time, float temperature)
- function to collect all sensor data and convert it into data stream for sending

void* temperature_sensor_simulator(void* arg)
- function for connecting with data aggregator and sending the data stream

void* config_thread_func(void* arg)
- function to process incoming commands and if valid take appropriate action to config the temperature sensor


TESTING AND ADDITIONAL SUGGESTIONS:

The program is tested including accounting for invalid data and inputs.  More time could be spent to test the config parameters at the input as well to ensure the data is valid.  The data frequency was limited to between 1 and 10 seconds but can be easily expanded with appropriate testing.  Data buffering and clearing would be another thing to take into account when scaling this system.  More care would also be applicable in protecting shared variables between the threads.  

The project is designed to have separate threads for data sending and for asynchronously mananging the config parameters on both the client and server.  The client needed a thread to control the data stream thread to ensure it could start and stop this capability.  On the server end threading the config part interleaves and makes the command line not as user friendly, but this was done to simulate real world scenario since the data stream from the sensor can easily be directed to another sink such as a log file for consumption and it is more likely that the data aggregator is also responsible for configuring the various sensors.
