PJDIR = /home/san/pjsua/pjproject-2.13
include $(PJDIR)/build.mak
OUTDIR = $(HOME)/cmpt433/public/projectApps
OUTFILE = $(OUTDIR)/hello_pjsua
OUTFILE1 = $(OUTDIR)/audio
OUTFILE2 = $(OUTDIR)/pick_up_call

$(OUTFILE): my_app.c 
	$(PJ_CC) -o $(OUTFILE) $< $(PJ_CFLAGS) $(PJ_LDFLAGS) $(PJ_LDLIBS)

$(OUTFILE1): audio_demo.c 
	$(PJ_CC) -o $(OUTFILE1) $< $(PJ_CFLAGS) $(PJ_LDFLAGS) $(PJ_LDLIBS)

$(OUTFILE2): pick_up_call.c 
	$(PJ_CC) -o $(OUTFILE2) $< $(PJ_CFLAGS) $(PJ_LDFLAGS) $(PJ_LDLIBS)


all: $(OUTFILE) $(OUTFILE1) $(OUTFILE2)

clean:
	rm -f $(OUTFILE) $(OUTFILE1) $(OUTFILE2)