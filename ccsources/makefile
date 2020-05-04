DIRS = ansifront c.comp c.opt cc253 cprep19 rlink rma

.PHONY: all subdirs $(DIRS)

all: subdirs

subdirs: $(DIRS)

$(DIRS):
	$(MAKE) -C $@ -f makefile $(ACTION)

install: ACTION = install
install: subdirs

clean: ACTION = clean
clean: subdirs
