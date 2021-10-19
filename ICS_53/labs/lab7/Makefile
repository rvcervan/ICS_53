CC=gcc
SAMPLE_DIR=sample_src
LABWORK_DIR=labwork_src
SAMPLE_DEPS=$(SAMPLE_DIR)/client.c $(SAMPLE_DIR)/server.c
LABWORK_DEPS=$(LABWORK_DIR)/client.c $(LABWORK_DIR)/server.c
BDIR=bin
INCLUDE=include

sample: $(SAMPLE_DEPS)
	mkdir -p $(BDIR)
	$(CC) -o $(BDIR)/server_sample $(SAMPLE_DIR)/server.c -I $(INCLUDE)
	$(CC) -o $(BDIR)/client_sample $(SAMPLE_DIR)/client.c -I $(INCLUDE)

lab: $(LABWORK_DEPS)
	mkdir -p $(BDIR)
	$(CC) -o $(BDIR)/server $(LABWORK_DIR)/server.c -I $(INCLUDE)
	$(CC) -o $(BDIR)/client $(LABWORK_DIR)/client.c -I $(INCLUDE)

clean:
	rm -rf $(BDIR)


