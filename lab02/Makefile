CC=gcc
CFLAGS=-Wall -Wextra -pedantic

# define any directories containing header files other than /usr/include
INCLUDES=

# define library paths in addition to /usr/lib
LFLAGS=

# define any libraries to link into executable
LIBS=

SERVER_SRCS=server.c itoa.c socket.c
SERVER_OBJS=$(SERVER_SRCS:.c=.o)
CLIENT_SRCS=client.c socket.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)

all:	server client
       
server: $(SERVER_OBJS) 
	$(CC) -o server $(SERVER_OBJS) $(LFLAGS) $(LIBS)

client: $(CLIENT_OBJS)
	$(CC) -o client $(CLIENT_OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ server client 
