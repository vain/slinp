.PHONY: clean

stopclock: stopclock.c stopclock.h
	$(CC) -Wall -Wextra $(CFLAGS) \
		-o stopclock stopclock.c \
		`pkg-config --cflags --libs gtk+-3.0`

clean:
	rm -f stopclock
