RELS = cp.o cp1.o cp2.o cp3.o cp4.o solve.o

CFLAGS = -mx32 -g -I.

coprep: $(RELS)
	gcc -mx32 $(RELS) -o $@

$(RELS): cp.h

clean:
	rm -f $(RELS);
