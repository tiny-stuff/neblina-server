Main process:

* The philosphy behind **neblina** is that all the servers reside in a single executable, but each server runs in a
  separate process.
* When neblina is started without the `-s` parameter, it'll start the **watchdog**. The watchdog will start the other
  processes (by **forking** itself with the `-s` parameter for each service). The watchdog will also keep track and
  restart any services that have died, unless they're **non-recoverable** (exited with the `NON_RECOVERABLE` status code),
  or are dying too frequently.

Service / server:

* When a service is started, it'll fire a **Server** (possibly a **TCPServer** or a **SSLServer**). The service passes
  to the service its own implementation of a **Session**. The Session implementation defines what should happen when
  new data is received.
* The server will set up a listener socket, and create a **Poller**, which will listen for anything that data or events
  coming from the socket.
  * In case of a **new connection**, the connection will be **accepted** and added to the **Poller** (for listening to
    new events). A new **Session** will be created and added to the **Session Pool**. The session contains a 
    **Communication Buffer** that will buffer everything that gets in or out of the client socket.
  * If **new data** is received from one of the clients, it is added to the **Communication Buffer**, and the Session
    Pool is communicated that this session received new data.
  * If the connection is **closed**, all data related to that connection is released.

Session pool:

* The **Session Pool** is a thread pool that continuously watches for changes to the **Communication Buffers**. It 
  contains a group of threads and, every time a socket is open, it is attached to one of the threads.
* One thread can monitor multiple sockets.
* The threads sleep until one of its Communication Buffers receive new data. In this case, the thread is woken up,
  and the data is sent to the **Session**, which then process it. If there's new data coming from the Session, it is
  sent to the socket, and the thread goes back to sleep.