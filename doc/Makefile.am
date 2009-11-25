man_PODS = librouteros.pod
man_MANS = librouteros.3

EXTRA_DIST = $(man_MANS) $(man_PODS)

.pod.1:
	pod2man --section=1 --release=$(VERSION) --center=$(PACKAGE) $< >$@

.pod.3:
	pod2man --section=3 --release=$(VERSION) --center=$(PACKAGE) $< >$@

.pod.8:
	pod2man --section=8 --release=$(VERSION) --center=$(PACKAGE) $< >$@
