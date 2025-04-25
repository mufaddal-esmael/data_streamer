# Use a base image
FROM gcc:latest

# Set the working directory
WORKDIR /app

# Update package list and install libczmq-dev
RUN apt-get update && apt-get install -y libczmq-dev

# Copy client files
COPY client/client.c client.c

# Compile the client
RUN gcc client.c -o client -I /opt/homebrew/include -L /opt/homebrew/lib -l czmq

# Expose the client port (if applicable)
EXPOSE 5556


# Run the client
CMD ["./client"]
