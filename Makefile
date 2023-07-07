CC = gcc
CFLAGS = -Wall -g -Wextra 
LDFLAGS = -lcurl

mdTool: mdTool.c
	$(CC) $(CFLAGS) -o mdTool mdTool.c $(LDFLAGS)

.PHONY: clean
clean:
	rm -f mdTool mdTool1
