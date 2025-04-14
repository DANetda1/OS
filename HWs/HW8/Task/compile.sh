gcc writer.c common.c -o writer -lrt -pthread
gcc reader.c common.c -o reader -lrt -pthread