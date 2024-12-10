# DataComm Project: Weather Information Service

This project demonstrates a client-server application using C programming.

## Overview
This project implements a client-server application using C programming. The server provides a weather information service that allows clients to:
- Retrieve weather data (`GET <City>`)
- Add new weather data (`ADD <City, Weather>`)
- Update existing weather data (`UPDATE <City, Weather>`)

The server uses TCP sockets for reliable communication and supports multiple clients simultaneously through multithreading.


## Project Features
- **Server**:
  - Listens for multiple client connections using TCP sockets.
  - Handles requests like retrieving, adding, and updating weather information.
  - Processes commands and sends appropriate responses.

- **Client**:
  - Connects to the server via TCP sockets.
  - Sends commands to interact with the server and displays responses.

## How to Use
1. **Compile the Server and Client**:
   ```bash
   gcc Server.c -o Server -lws2_32
   gcc Client.c -o Client -lws2_32
