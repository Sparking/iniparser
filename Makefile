AR       := ar
CC       := gcc
CXX      := g++
RANLIB   := ranlib
ARFLAGS  := rc
CPPFLAGS := -Wall -Werror -O0
CXXFLAGS := -g
LDFLAGS  := -L$(CURDIR)
LIBS     := -lini

out      := libini.a
cxx_src  := ini.cc
depends  := $(patsubst %.cc,%.d,$(cxx_src))
cxx_objs := $(patsubst %.cc,%.o,$(cxx_src))
objs     := $(cxx_objs)

demo_out := demo
demo_src := demo.cc
demo_objs := $(patsubst %.cc,%.o,$(demo_src))
demo_deps := $(patsubst %.cc,%.d,$(demo_src))

.PHONY: all
all: $(demo_out)
$(demo_out): $(demo_objs) $(out)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(demo_objs) -o $@ $(LIBS)
$(out): $(objs)
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@
-include $(depends) $(demo_deps)
$(demo_objs) $(cxx_objs): %.o: %.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -std=c++11 -MMD -c -o $@ $<

.PHONY: clean
clean:
	$(RM) $(demo_deps)
	$(RM) $(demo_objs)
	$(RM) $(demo_out)
	$(RM) $(depends)
	$(RM) $(objs)
	$(RM) $(out)

