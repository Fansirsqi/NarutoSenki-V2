TARGET = libcocosdenshion.so

INCLUDES += -I.. -I../include

SOURCES = ../linux/SimpleAudioEngineOpenAL.cpp 
SHAREDLIBS += -lopenal -lalut -logg -lvorbis -lvorbisfile


COCOS_ROOT = ../..
include $(COCOS_ROOT)/cocos2dx/proj.linux/cocos2dx.mk

TARGET := $(LIB_DIR)/$(TARGET)

all: $(TARGET)

$(TARGET): $(OBJECTS) $(CORE_MAKEFILE_LIST)
	@mkdir -p $(@D)
	$(LOG_LINK)$(CXX) $(CXXFLAGS) $(OBJECTS) -shared -o $(TARGET) $(SHAREDLIBS) $(STATICLIBS)

$(OBJ_DIR)/%.o: ../%.cpp $(CORE_MAKEFILE_LIST)
	@mkdir -p $(@D)
	$(LOG_CXX)$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) $(VISIBILITY) -g -c $< -o $@
