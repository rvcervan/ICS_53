CC=gcc
EXAMPLE_DIR=src
MULTIPLEXING_DEPS=$(EXAMPLE_DIR)/client.c $(EXAMPLE_DIR)/server_multiplexing.c
MULTIPROCESSING_DEPS=$(EXAMPLE_DIR)/client.c $(EXAMPLE_DIR)/server_multiprocessing.c
MULTITHREADING1_DEPS=$(EXAMPLE_DIR)/client.c $(EXAMPLE_DIR)/server_multithreading_1.c
MULTITHREADING2_DEPS=$(EXAMPLE_DIR)/client.c $(EXAMPLE_DIR)/server_multithreading_2.c
BDIR=bin
INCLUDE=include

default: multiprocessing multithreading1 multithreading2 multiplexing 

multiprocessing: $(MULTIPROCESSING_DEPS)
	mkdir -p $(BDIR)
	$(CC) -o $(BDIR)/server_multiprocessing $(EXAMPLE_DIR)/server_multiprocessing.c -I $(INCLUDE)
	$(CC) -o $(BDIR)/client $(EXAMPLE_DIR)/client.c -I $(INCLUDE)	

multithreading1: $(MULTITHREADING1_DEPS)
	mkdir -p $(BDIR)
	$(CC) -o $(BDIR)/server_multithreading_1 $(EXAMPLE_DIR)/server_multithreading_1.c -I $(INCLUDE) -lpthread
	$(CC) -o $(BDIR)/client $(EXAMPLE_DIR)/client.c -I $(INCLUDE) 
	
multithreading2: $(MULTITHREADING2_DEPS)
	mkdir -p $(BDIR)
	$(CC) -o $(BDIR)/server_multithreading_2 $(EXAMPLE_DIR)/server_multithreading_2.c -I $(INCLUDE) -lpthread
	$(CC) -o $(BDIR)/client $(EXAMPLE_DIR)/client.c -I $(INCLUDE) 

multiplexing: $(MULTIPLEXING_DEPS)
	mkdir -p $(BDIR)
	$(CC) -o $(BDIR)/server_multiplexing $(EXAMPLE_DIR)/server_multiplexing.c -I $(INCLUDE)
	$(CC) -o $(BDIR)/client $(EXAMPLE_DIR)/client.c -I $(INCLUDE)

clean:
	rm -rf $(BDIR)


