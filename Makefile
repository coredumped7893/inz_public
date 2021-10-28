
# - inc/
#     - *.h
# - src/
#	  Control
#     	- *.c
#		- *.h
#	  DataInput
#     	- *.c
#		- *.h
#     Cameras
#     	- *.c
#		- *.h
#     	- *.cpp
#		- *.hpp
#	  FlightManager
#     	- *.c
#		- *.h
#	  ImageProcessing
#     	- *.c
#		- *.h
#     	- *.cpp
#		- *.hpp
#     Maps
#	  	- *.c
#	  	- *.h
#	  Util
#     	- *.c
#		- *.h
#     	- *.cpp
#	  Tests
#		- *.c
#		- *.cpp
#		- *.h
#		- *.hpp
# - build/
#     - *.o
# - FS_exec
# - FS_exec_img


# local or PI
HARDWARE := local


TEST=0
TARGETR1 := ./build/FS_exec
TARGETR2 := ./build/FS_exec_img

#Sources for target1
SOURCESR1C := $(wildcard  src/Control/*.c src/DataInput/*.c src/Util/*.c src/FlightManager/*.c src/Aircraft/*.c)
SOURCESR1CXX := $(wildcard  src/Control/*.cpp src/DataInput/*.cpp src/Util/*.cpp src/FlightManager/*.cpp src/Aircraft/*.cpp)
#-------------------

#Sources for target2
SOURCESR2C := $(wildcard   src/ImageProcessing/*.c src/Util/*.c src/Maps/*.c src/Cameras/*.c)
SOURCESR2CXX := $(wildcard	src/ImageProcessing/*.cpp src/Util/*.cpp src/Cameras/*.cpp)
#-------------------

#sources for tests--------------------------------------
SOURCESTESTSR1C := $(wildcard	src/Tests/*r1.c)
SOURCESTESTSR1CXX := $(wildcard	src/Tests/*r1.cpp)


SOURCESTESTSR2C := $(wildcard	src/Tests/*r2.c)
SOURCESTESTSR2CXX := $(wildcard	src/Tests/*r2.cpp)
#--------------------------------------------------------

OBJECTSTESTSR1 := ${SOURCESTESTSR1C:%.c=%.o}
OBJECTSTESTSR1 += ${SOURCESTESTSR1CXX:%.cpp=%.o}
OBJECTSTESTSR12 := ${SOURCESTESTSR1C:%.c=build/%.o}
OBJECTSTESTSR12 += ${SOURCESTESTSR1CXX:%.cpp=build/%.o}

OBJECTSTESTSR2 := ${SOURCESTESTSR2C:%.c=%.o}
OBJECTSTESTSR2 += ${SOURCESTESTSR2CXX:%.cpp=%.o}
OBJECTSTESTSR22 := ${SOURCESTESTSR2C:%.c=build/%.o}
OBJECTSTESTSR22 += ${SOURCESTESTSR2CXX:%.cpp=build/%.o}

OBJECTSR1 := ${SOURCESR1C:%.c=%.o}
OBJECTSR1 += ${SOURCESR1CXX:%.cpp=%.o}

OBJECTSR2 := ${SOURCESR2C:%.c=%.o}
OBJECTSR2 += ${SOURCESR2CXX:%.cpp=%.o}

ifeq ($(HARDWARE), local)
export PKG_CONFIG_PATH=:/usr/lib/pkgconfig
else
export PKG_CONFIG_PATH=:/usr/local/lib/pkgconfig
endif


INCLUDE := -I./inc/ -L./lib/$(HARDWARE)/
LIBPATH := -lsocket -lpthread -lm
LIBPATH_CV := $(shell pkg-config --libs opencv4)
LIBS := $(wildcard lib/$(HARDWARE)/*.o)
INCLUDE_CV := $(shell pkg-config --cflags opencv4)

FLAGS := -Wall -pedantic -D_FORTIFY_SOURCE=2 -O0 -g
CCFLAGS := $(FLAGS) -std=c11
CXXFLAGS := $(FLAGS) -std=c++11

CC := gcc
Cxx := g++
export LD_LIBRARY_PATH=./lib/$(HARDWARE)/


#Convert sources for target1 to object files
OBJECTSR12 := ${SOURCESR1C:%.c=build/%.o}
OBJECTSR12 += ${SOURCESR1CXX:%.cpp=build/%.o}
#-------------------------------------------

#Convert sources for target2 to object files, separate lists for .c and .cpp files
OBJECTSR22 := ${SOURCESR2C:%.c=build/%.o}
OBJECTSR22 += ${SOURCESR2CXX:%.cpp=build/%.o}
#---------------------------------------------------------------------------------

#Debug print variables
#$(error    VAR is $(OBJECTSTESTSR1))

all: R1 R2

R1: $(OBJECTSR1)
ifeq ($(TEST),1)
	$(CC) $(CCFLAGS) $(OBJECTSR12)  $(LIBS)  -fprofile-arcs -ftest-coverage -p -o $(TARGETR1) $(INCLUDE)  $(LIBPATH)
else
	$(CC) $(CCFLAGS) $(OBJECTSR12)  $(LIBS) -o $(TARGETR1) $(INCLUDE)  $(LIBPATH)
endif

R2: $(OBJECTSR2)
ifeq ($(TEST),1)
	$(Cxx) $(CXXFLAGS) $(OBJECTSR22) $(LIBS) -fprofile-arcs -ftest-coverage -p -o $(TARGETR2) $(INCLUDE) $(INCLUDE_CV)  $(LIBPATH) $(LIBPATH_CV)
else
	$(Cxx) $(CXXFLAGS) $(OBJECTSR22) $(LIBS) -o $(TARGETR2) $(INCLUDE) $(INCLUDE_CV)  $(LIBPATH) $(LIBPATH_CV)
endif

%.o: %.c
	mkdir -p build/$(@D)
ifeq ($(TEST),1)
	$(CC) $(CCFLAGS) -fprofile-arcs -ftest-coverage $(INCLUDE) -c  -DTEST $< -o build/$@
else
	$(CC) $(CCFLAGS) $(INCLUDE) -c  $< -o build/$@
endif

%.o: %.cpp
	mkdir -p build/$(@D)
ifeq ($(TEST),1)
	$(CXX) $(CXXFLAGS) -fprofile-arcs -ftest-coverage $(INCLUDE) $(INCLUDE_CV) -c  -DTEST $< -o build/$@
else
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(INCLUDE_CV) -c  $< -o build/$@
endif


clean:
	rm -rf build/*
	rm -rf coverage/*
	rm -rf docs/*

run: all
	$(TARGETR2)
	$(TARGETR1) &
	#./run_all.sh
runr1: R1
	$(TARGETR1)

runr2: R2
	$(TARGETR2)

stop:
	pkill -f FS_exec

debugr1: all
	gdb  $(TARGETR1)  -ex "break main"

debugr2: all
	gdb  $(TARGETR2)  -ex "break main"

TEST_testsr1 := -Wl,--wrap=throw_error -Wl,--wrap=printf
TEST_testsr2 :=
TEST_testAppr1 := -Wl,--wrap=calloc -Wl,--wrap=destroy_inet_socket -Wl,--wrap=x_plane_socket -Wl,--wrap=exit  -Wl,--wrap=printf -Wl,--wrap=perror
TEST_testUtilsr1 :=
TEST_testDataInputr1 := -Wl,--wrap=socket_send_ip
#-Wl,--wrap=printf


#TESTTARGET1 := ./build/src/Tests/testsr1
#TESTTARGET2 := ./build/src/Tests/testsr2

TESTTARGETS := $(basename $(notdir $(wildcard	src/Tests/test*)))


#Execute every test target
tests: clean testr1 testr2
	mkdir -p coverage;
	for number in $(TESTTARGETS) ; do \
  		echo $$number ; \
		./build/$$number ; \
		echo '\n' ; \
	done
	gcovr -r ./ --html --html-details -o coverage/coverage-report.html

#make testr1 TEST=1
#NAMESR1 = $(basename $(OBJECTSTESTSR1))
#$(error    VAR is $(TESTTARGETS))

testr1: R1 $(OBJECTSTESTSR1)
	$(foreach dir,$(OBJECTSTESTSR1),$(CXX) $(CXXFLAGS)  $(OBJECTSR12) ./build/$(dir) -fprofile-arcs -ftest-coverage -p -DTEST $(TEST_$(basename $(notdir $(dir)))) $(LIBS) -o ./build/$(basename $(notdir $(dir)))  $(INCLUDE)  $(LIBPATH) -lcmocka ;)


testr2: R2 $(OBJECTSTESTSR2)
	$(foreach dir,$(OBJECTSTESTSR2),$(CXX) $(CXXFLAGS) $(OBJECTSR22) ./build/$(dir) -fprofile-arcs -ftest-coverage -p -DTEST $(TEST_$(basename $(notdir $(dir)))) $(LIBS) -o ./build/$(basename $(notdir $(dir))) $(INCLUDE) $(INCLUDE_CV) $(LIBPATH) $(LIBPATH_CV) -lcmocka ;)

doxygen:
	echo "Generating documentation"
	doxygen Doxyfile

memcheckr1:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose  --track-fds=yes build/FS_exec
memcheckr2:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose  --track-fds=yes build/FS_exec_img
massifr1:
	valgrind --tool=massif --verbose build/FS_exec
massifr2:
	valgrind --tool=massif --verbose build/FS_exec_img
callgrindr1:
	valgrind --tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes build/FS_exec
callgrindr2:
	valgrind --tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes build/FS_exec_img