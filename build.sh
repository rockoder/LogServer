g++ src/Server.cpp -o bin/server -I include -pthread -std=c++11
g++ test/SampleClient.cpp -o bin/client
g++ test/QueueTest.cpp -o bin/queuetest -I include -pthread -std=c++11
