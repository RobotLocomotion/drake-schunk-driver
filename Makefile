# Trivial Makefile for Schunk driver.

CCS = wsg_command_message.cc wsg_command_sender.cc \
      wsg_return_message.cc wsg_return_receiver.cc \
      position_force_control.cc

HS = crc.h defaults.h \
     wsg_command_sender.h wsg_command_message.h \
     wsg_return_message.h wsg_return_receiver.h \
     wsg.h \
     position_force_control.h

BINDIR = bin

CXXFLAGS += -g -std=c++14

.PHONY: all
all: $(BINDIR)/demo $(BINDIR)/pcdemo

.PHONY: demo
demo: $(BINDIR)/demo
pcdemo: $(BINDIR)/pcdemo

clean:
	rm -rf $(BINDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/demo: $(BINDIR) $(CCS) $(HS) demo.cc
	$(CXX) $(CXXFLAGS) $(filter %.cc,$^) -o $@

$(BINDIR)/pcdemo: $(BINDIR) $(CCS) $(HS) position_force_demo.cc
	$(CXX) $(CXXFLAGS) $(filter %.cc,$^) -o $@
