CXX:= g++ -std=c++0x
CFLAGS:= -c -Wall -O0

PROJECT := itf
BUILD_DIR:=build
BUILD_INCLUDE_DIR := $(BUILD_DIR)/src
OBJ_BUILD_DIR := $(BUILD_DIR)/src/$(PROJECT)
LIB_BUILD_DIR := $(BUILD_DIR)/lib


TRACKER_SRC_DIR=src/$(PROJECT)/trackers
TRACKER_KLT_BUILD_DIR := $(OBJ_BUILD_DIR)/trackers/klt_c
TRACKER_BUILD_DIR := $(OBJ_BUILD_DIR)/trackers


CXX_TRACKER_SRCS := $(shell find $(TRACKER_SRC_DIR)/ ! -name "test_*.cpp" -name "*.cpp")
CXX_TRACKER_OBJS := $(addprefix $(BUILD_DIR)/, ${CXX_TRACKER_SRCS:.cpp=.o})

OBJS :=  $(CXX_TRACKER_OBJS)

EXAMPLE_SRC_DIR:=examples
EXAMPLE_SRCS := $(shell find $(EXAMPLE_SRC_DIR)/ -name "*.cpp")
EXAMPLE_OBJS := $(addprefix $(BUILD_DIR)/, ${EXAMPLE_SRCS:.cpp=.o})
EXAMPLE_BUILD_DIR := $(BUILD_DIR)/examples
EXAMPLE_BUILD_DIRS := $(EXAMPLE_BUILD_DIR)
EXAMPLE_BUILD_DIRS += $(foreach obj,$(EXAMPLE_OBJS),$(dir $(obj)))
EXAMPLE_BINS := ${EXAMPLE_OBJS:.o=.bin}

INCLUDE_DIRS += $(BUILD_INCLUDE_DIR) ./src ./include

OPENCV_INCLUDE_DIR :=/usr/include
OPENCV_LIB_DIR:=/usr/local/lib
OPENCV_LIBS:= opencv_core opencv_highgui opencv_imgproc opencv_video
INCLUDE_DIRS+=$(OPENCV_INCLUDE_DIR)
LIBRARY_DIRS+=$(OPENCV_LIB_DIR)
LIBRARIES+=$(OPENCV_LIBS)

LIB_BUILD_DIR := $(BUILD_DIR)/lib
SHARED_NAME := $(LIB_BUILD_DIR)/lib$(PROJECT).so
STATIC_NAME := $(LIB_BUILD_DIR)/lib$(PROJECT).a
ALL_BUILD_DIRS := $(sort \
		$(BUILD_DIR) $(LIB_BUILD_DIR) $(OBJ_BUILD_DIR) \
        $(TRACKER_KLT_BUILD_DIR) $(TRACKER_BUILD_DIR) \
		$(EXAMPLE_BUILD_DIRS))

##############################
WARNS_EXT := warnings.txt
WARNINGS := -Wall -Wno-sign-compare
COMMON_FLAGS += $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
CXXFLAGS += -pthread -fPIC $(COMMON_FLAGS) $(WARNINGS) -std=c++0x 
LINKFLAGS_SHARED=-fPIC $(COMMON_FLAGS) $(WARNINGS)
LINKFLAGS_STATIC= $(COMMON_FLAGS) $(WARNINGS)
LINKFLAGS += $(LINKFLAGS_SHARED)
LDFLAGS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir)) $(foreach library,$(LIBRARIES),-l$(library))
SUPERCLEAN_EXTS := .so .a .o .bin .testbin .pb.cc .pb.h _pb2.py .

all: $(ALL_BUILD_DIRS) $(STATIC_NAME) examples
examples:  $(EXAMPLE_BINS)
mkbuilddir:
	@ mkdir -p $(BUILD_DIR)
$(ALL_BUILD_DIRS):|mkbuilddir
	@ mkdir -p $@

$(SHARED_NAME): $(PROTO_OBJS) $(OBJS) | $(LIB_BUILD_DIR)
	$(CXX) -shared -o $@ $(OBJS) $(LINKFLAGS) $(LDFLAGS)
	@ echo
$(STATIC_NAME): $(PROTO_OBJS) $(OBJS) | $(LIB_BUILD_DIR)
	ar rcs $@ $(PROTO_OBJS) $(OBJS)
	@ echo

$(EXAMPLE_BINS): %.bin : %.o $(STATIC_NAME)
	$(CXX) $< $(STATIC_NAME) -o $@ $(LINKFLAGS) $(LDFLAGS)
	@ echo

$(TRACKER_KLT_BUILD_DIR)/%.o: src/$(PROJECT)/trackers/klt_c/%.cpp $(HXX_SRCS) \
		| $(TRACKER_KLT_BUILD_DIR)
	$(CXX) $< $(CXXFLAGS) -c -o $@ 2> $@.$(WARNS_EXT) \
		|| (cat $@.$(WARNS_EXT); exit 1)
	@ cat $@.$(WARNS_EXT)
	@ echo
$(TRACKER_BUILD_DIR)/%.o: src/$(PROJECT)/trackers/%.cpp $(HXX_SRCS) \
		| $(TRACKER_BUILD_DIR) 
	$(CXX) $< $(CXXFLAGS) -c -o $@ 2> $@.$(WARNS_EXT) \
		|| (cat $@.$(WARNS_EXT); exit 1)
	@ cat $@.$(WARNS_EXT)
	@ echo
$(EXAMPLE_BUILD_DIR)/%.o: examples/%.cpp $(PROTO_GEN_HEADER) \
		| $(EXAMPLE_BUILD_DIRS)
	$(CXX) $< $(CXXFLAGS) -c -o $@ 2> $@.$(WARNS_EXT) \
		|| (cat $@.$(WARNS_EXT); exit 1)
	@ cat $@.$(WARNS_EXT)
	@ echo
clean:
	@- $(RM) -rf $(ALL_BUILD_DIRS)
