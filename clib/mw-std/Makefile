AS = rma

%.r: %.a 
	$(AS) $(ASFLAGS) $< -O=$@

SRCFILES = prof.r defdrive.r pffinit.r pflinit.r fopen.r puts.r \
           gets.r mktemp.r fread.r fwrite.r printf.r scanf.r setbuf.r \
	   fseek.r putc.r ftelldummy.r case.r getc.r flshdummy.r setbase.r \
	   findstr.r pfldummy.r pffdummy.r atof.r scale.r system.r strings.r \
	   stringsn.r index.r atol.r atoi.r iob_data.r chcodes.r l3tol.r \
	   ltol3.r sleep.r qsort.r setjmp.r strass.r abs.r memory.r cfloats.r \
	   clmul.r cldiv.r claddsub.r clcompare.r clbits.r clnegcompl.r \
	   clconvert.r clmove.r clincdec.r clshifts.r clcommon.r ccmult.r \
	   ccmod.r ccdiv.r cshifts.r rpterr.r abort.r signal.r change.r stat.r \
	   access.r io.r misc.r mod.r dir.r mem.r time.r process.r id.r intercept.r \
	   syscall.r syscommon.r cfinish.r profdummy.r tidyup.r 


clib.l: $(SRCFILES)
	@cat $(SRCFILES) >clib.l
	@echo didit

$(SRCFILES):

clean:
	rm -f *.r clib.l
