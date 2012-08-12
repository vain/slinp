.PHONY: clean

showpdf: showpdf.c showpdf.h
	$(CC) -Wall -Wextra $(CFLAGS) \
		-o showpdf showpdf.c \
		`pkg-config --cflags --libs gtk+-3.0 poppler-glib`

clean:
	rm -f showpdf
