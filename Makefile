# Trivial Makefile for Schunk driver.

CCS = demo.cc
BINDIR = bin

CXXFLAGS += -std=c++11

.PHONY: demo
demo: $(BINDIR) $(BINDIR)/demo

clean:
	rm -rf $(BINDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/demo: $(CCS)
	$(CXX) $(CXXFLAGS) $^ -o $@
