ALL=sleep5

all: $(ALL)

%: %.c
	scorep-gcc -o $@ $<

clean:
	rm -f $(ALL)
.PHONY: clean
