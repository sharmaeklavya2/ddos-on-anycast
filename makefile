LIB_SRCDIR := src/lib
LIB_SOURCES := $(wildcard $(LIB_SRCDIR)/*.cpp)
LIB_HEADERS := $(wildcard $(LIB_SRCDIR)/*.h)

COMMON_FLAGS := -std=c++11 -Wall -Wpedantic # -mmacosx-version-min=10.7 -stdlib=libc++
DEBUG_FLAGS := -D DEBUG -g $(COMMON_FLAGS)
RELEASE_FLAGS := -D NDEBUG -O2 $(COMMON_FLAGS)

MASTER_BUILDDIR := build
DEBUG_BUILDDIR := $(MASTER_BUILDDIR)/debug
RELEASE_BUILDDIR := $(MASTER_BUILDDIR)/release

ifneq ($(DEBUG),)
	FLAGS := $(DEBUG_FLAGS)
	BUILDDIR := $(DEBUG_BUILDDIR)
else
	FLAGS := $(RELEASE_FLAGS)
	BUILDDIR := $(RELEASE_BUILDDIR)
endif

LIB_BUILDDIR := $(BUILDDIR)/lib
CXX := g++ $(FLAGS)

.PHONY: all
all: $(BUILDDIR)/simulate $(BUILDDIR)/graph_gen $(BUILDDIR)/dag $(BUILDDIR)/segtree;

.PHONY: clean
clean:
	rm -rf var $(MASTER_BUILDDIR)
	rm -f $(MAIN_OUTPUT)
	rm -f gmon.out
	find . -name "*.pyc" -type f -delete
	find . -name "__pycache__" -type d -delete

# Lib files

BASE_HSET := $(LIB_SRCDIR)/base.hpp
UTIL_HSET := $(LIB_SRCDIR)/util.hpp $(BASE_HSET)
SAMPLE_HSET := $(LIB_SRCDIR)/sample.hpp $(BASE_HSET)
DSU_HSET := $(LIB_SRCDIR)/dsu.hpp $(BASE_HSET)
GRAPH_GEN_HSET := $(LIB_SRCDIR)/graph_gen.hpp $(BASE_HSET)
NETWORK_HSET := $(LIB_SRCDIR)/network.hpp $(GRAPH_GEN_HSET)
PLACE_VICTIMS_HSET := $(LIB_SRCDIR)/place_victims.hpp $(NETWORK_HSET)
ATTACK_HSET := $(LIB_SRCDIR)/attack.hpp $(NETWORK_HSET)
DAG_HSET := $(LIB_SRCDIR)/dag.hpp $(BASE_HSET)
SEGTREE_HSET := $(LIB_SRCDIR)/segtree.hpp $(BASE_HSET)

LIB_OBJS := $(LIB_BUILDDIR)/network.o $(LIB_BUILDDIR)/network_grow.o $(LIB_BUILDDIR)/graph_gen.o $(LIB_BUILDDIR)/attack.o $(LIB_BUILDDIR)/place_victims.o $(LIB_BUILDDIR)/dag.o $(LIB_BUILDDIR)/segtree.o $(LIB_BUILDDIR)/sample.o

$(LIB_BUILDDIR)/network.o: $(LIB_SRCDIR)/network.cpp $(GRAPH_GEN_HSET) $(NETWORK_HSET) $(UTIL_HSET)
	@mkdir -p $(LIB_BUILDDIR)
	$(CXX) -c $< -o $@

$(LIB_BUILDDIR)/sample.o: $(LIB_SRCDIR)/sample.cpp $(SEGTREE_HSET)
	@mkdir -p $(LIB_BUILDDIR)
	$(CXX) -c $< -o $@

$(LIB_BUILDDIR)/network_grow.o: $(LIB_SRCDIR)/network_grow.cpp $(UTIL_HSET) $(SAMPLE_HSET) $(GRAPH_GEN_HSET) $(NETWORK_HSET)
	@mkdir -p $(LIB_BUILDDIR)
	$(CXX) -c $< -o $@

$(LIB_BUILDDIR)/graph_gen.o: $(LIB_SRCDIR)/graph_gen.cpp $(GRAPH_GEN_HSET) $(SAMPLE_HSET) $(DSU_HSET)
	@mkdir -p $(LIB_BUILDDIR)
	$(CXX) -c $< -o $@

$(LIB_BUILDDIR)/place_victims.o: $(LIB_SRCDIR)/place_victims.cpp $(PLACE_VICTIMS_HSET)
	@mkdir -p $(LIB_BUILDDIR)
	$(CXX) -c $< -o $@

$(LIB_BUILDDIR)/attack.o: $(LIB_SRCDIR)/attack.cpp $(ATTACK_HSET)
	@mkdir -p $(LIB_BUILDDIR)
	$(CXX) -c $< -o $@

$(LIB_BUILDDIR)/dag.o: $(LIB_SRCDIR)/dag.cpp $(DAG_HSET) $(UTIL_HSET)
	@mkdir -p $(LIB_BUILDDIR)
	$(CXX) -c $< -o $@

$(LIB_BUILDDIR)/segtree.o: $(LIB_SRCDIR)/segtree.cpp $(SEGTREE_HSET)
	@mkdir -p $(LIB_BUILDDIR)
	$(CXX) -c $< -o $@

# Executable files

$(BUILDDIR)/simulate.o: src/simulate.cpp $(GRAPH_GEN_HSET) $(NETWORK_HSET)
	@mkdir -p $(BUILDDIR)
	$(CXX) -c $< -o $@

$(BUILDDIR)/graph_gen.o: src/graph_gen.cpp $(GRAPH_GEN_HSET)
	@mkdir -p $(BUILDDIR)
	$(CXX) -c $< -o $@

$(BUILDDIR)/dag.o: src/dag.cpp $(DAG_HSET)
	@mkdir -p $(BUILDDIR)
	$(CXX) -c $< -o $@

$(BUILDDIR)/segtree.o: src/segtree.cpp $(SEGTREE_HSET)
	@mkdir -p $(BUILDDIR)
	$(CXX) -c $< -o $@

$(BUILDDIR)/simulate: $(LIB_OBJS) $(BUILDDIR)/simulate.o
	$(CXX) $^ -o $@

$(BUILDDIR)/graph_gen: $(LIB_OBJS) $(BUILDDIR)/graph_gen.o
	$(CXX) $^ -o $@

$(BUILDDIR)/dag: $(LIB_BUILDDIR)/dag.o $(BUILDDIR)/dag.o
	$(CXX) $^ -o $@

$(BUILDDIR)/segtree: $(LIB_BUILDDIR)/segtree.o $(BUILDDIR)/segtree.o
	$(CXX) $^ -o $@
