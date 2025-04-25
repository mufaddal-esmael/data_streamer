# Use a base image
FROM python:3.12-slim

# Set the working directory
WORKDIR /app

# Copy server files
COPY server/server.py server.py

# Generate requirements.txt
RUN pip install pipreqs && pipreqs . --force

# Install dependencies
RUN pip install -r requirements.txt

# Expose the server ports
EXPOSE 5555

# Run the server
CMD ["python", "server.py"]
