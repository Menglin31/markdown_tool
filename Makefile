CC = gcc
CFLAGS = -Wall -g -Wextra 
LDFLAGS = -lcurl

mdTool1: mdTool1.c
	$(CC) $(CFLAGS) -o mdTool1 mdTool1.c $(LDFLAGS)


mdTool: mdTool.c
	$(CC) $(CFLAGS) -o mdTool mdTool.c $(LDFLAGS)

.PHONY: clean
clean:
	rm -f mdTool mdTool1
