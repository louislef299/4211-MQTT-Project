# 4211-MQTT-Project

In order to successfully run the project, you type

```bash
make server
```

then type

```bash
make client
```

then, on one terminal, type

```bash
./start_server
```

and on another type

```bash
./start_client
```

This is how to start the code. Within the client program, by typing 'help', a list of command will appear and you will be able to publish, list topics, and subscribe to the main server. Enjoy!

## BUGLIST

- The server only works on the LAN(127.0.0.1), so if there are not enough outgoing sockets, there will usually be issues with multiple clients connecting at once.

- There are most likely others but, to be honest, I ran out of time and this was my best effort.

### Louis Lefebvre
### lefeb073
### lefeb073@umn.edu
