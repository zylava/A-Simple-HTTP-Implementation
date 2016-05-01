CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE) -I/usr/local/include
USERID=EDIT_MAKE_FILE
CLASSES=

all: web-server web-server-async web-server-persistent web-client web-server-timeout web-client-timeout

web-server: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp httpResponse.cpp httpRequest.cpp

web-server-async: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp httpResponse.cpp httpRequest.cpp

web-server-persistent: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp httpResponse.cpp httpRequest-persistent.cpp

web-client: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp httpResponse.cpp httpRequest.cpp

web-client-timeout: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp httpResponse.cpp httpRequest.cpp

web-server-timeout: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp httpResponse.cpp httpRequest.cpp

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM web-server web-server-persistent web-client web-server-async web-client-timeout web-server-timeout *.tar.gz

tarball: clean
	tar -cvf $(USERID).tar.gz *
