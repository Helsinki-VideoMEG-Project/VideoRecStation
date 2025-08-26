# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = src
HTMLBUILDDIR  = html

.PHONY: html

html:
	rm -rf "$(HTMLBUILDDIR)"
	@$(SPHINXBUILD) -M "html" "$(SOURCEDIR)" "$(HTMLBUILDDIR)" $(SPHINXOPTS) $(O)
	touch "$(HTMLBUILDDIR)/html/.nojekyll"

clean:
	rm -rf "$(HTMLBUILDDIR)"
