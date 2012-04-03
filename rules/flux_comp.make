FLUX_ARGS ?= -c -i --include-prefix=core --call-mode

$(builddir)/%.c $(builddir)/%.h: $(srcdir)/%.flux
	fluxcomp $(FLUX_ARGS) $<
