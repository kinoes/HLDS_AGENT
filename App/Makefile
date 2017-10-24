include /home/HD_Project/config/Make.cf
CC= g++

HEADER :=$(wildcard *.h)
SOURCES := $(wildcard *.cpp)
OBJS=$(patsubst %.cpp, %.o,$(SOURCES))

TARGET = HLDS_AGENT
INSTALL = /usr/bin/install -c

USING_LIBS = -lpthread -lxpacket -lxthread -lrt -lcommon -lboost_system -lboost_thread -lboost_locale 
CFLAGS    =-std=c++11 -Wall -Wno-deprecated -D _LINUX_COMPILE -Wno-unknown-pragmas -Wno-literal-suffix -Wno-switch-enum -Wno-delete-non-virtual-dtor -Wno-switch -Wno-unused-variable
LIB_PATHS = $(ISAVER_LIB_PATH)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(SHARED_LD_FLAGS) $(OBJS) $(ISAVER_LIB_PATH) $(USING_LIBS)

%.o: %.cpp $(HEADER)
	@ccdv $(CC) $(CFLAGS) -c $< -o $@ $(SHARED_LD_FLAGS) $(ISAVER_INC_PATH) $(HLDS_INC_PATH)

clean:
	@echo ""
	@echo "[CLEAN]===========================================>"
	rm -f $(OBJS) $(TARGET)
install:
	$(INSTALL) -s $(TARGET)  $(ISAVER_BIN_PATH)


