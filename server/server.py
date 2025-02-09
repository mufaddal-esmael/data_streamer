#
#   temp sensor receiver/server in Python
#   Waits for sensor data from client
#   can asynchroniously send config commands to sensor
#

import time
import zmq
import threading

context = zmq.Context()

# Socket to receive messages on
receiver = context.socket(zmq.PULL)
receiver.bind("tcp://*:5555")

# Socket to send messages to
sender = context.socket(zmq.PUSH)
sender.connect("tcp://localhost:5556")

def parse_message(message: str) -> list:
    
    delimiter = "//#"
    result = message.split(delimiter)
    return result

def print_message(result):
    print(f"\nTimestamp: {result[1]} Time since last message: {result[2]} temperature: {result[3]} \n")


def receive_data():
    
    #  Wait for next request from client
    print("Waiting for message")

    while True:
        message = receiver.recv()
        #print(f"Received data: {message}")
        print_message(parse_message(message.decode('utf-8')))

        #  thread sleeps
        time.sleep(1)


def send_command():

    print("This is your command interface.")
    print("You can enter a number between 1 and 10 to change the sensor frequency.")
    print("Please enter: (freq num / start / stop / exit).\n")

    while True:
        
        command = input("Enter your command: ")
        #print(f"Command entered: {command} command type: {type(command)}")
        
        #  Send command to client
        sender.send(bytes(command, 'utf-8'))
        time.sleep(1)


# Create the threads
thread1 = threading.Thread(target=receive_data)
thread2 = threading.Thread(target=send_command)

# Start the threads
thread1.start()
thread2.start()

# Join the threads
thread1.join()
thread2.join()

