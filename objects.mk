OBJ = src/watchdog/watchdog.o \
      src/server/connection.o src/server/server.o src/service/session.o src/server/cpool/cpool.o \
      src/server/tcp/tcp_server.o src/server/poller/poller_$(OS).o \
      src/os/$(OS_GENERIC)/os.o src/os/$(OS_GENERIC)/window.o \
      src/util/logs.o src/util/error.o