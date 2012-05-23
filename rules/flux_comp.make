FLUX_ARGS ?= -c -i --include-prefix=core --call-mode

$(builddir)/%.c $(builddir)/%.h: $(srcdir)/%.flux
	$(FLUXCOMP) $(FLUX_ARGS) $<
