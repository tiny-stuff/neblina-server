#ifndef NEBLINA_SERVER_SERVER_H
#define NEBLINA_SERVER_SERVER_H

typedef struct Server Server;

int server_flush_connection(Server* server, Connection* connection);

#endif //NEBLINA_SERVER_SERVER_H
