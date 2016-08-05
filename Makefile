# Trivial Makefile for Schunk driver.

CCS = demo.cc
HS = wsg_command_sender.h wsg_command_message.h

BINDIR = bin

CXXFLAGS += -std=c++11

.PHONY: demo
demo: $(BINDIR)/demo

clean:
	rm -rf $(BINDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/demo: $(BINDIR) $(CCS) $(HS)
	$(CXX) $(CXXFLAGS) $(filter %.cc,$^) -o $@
