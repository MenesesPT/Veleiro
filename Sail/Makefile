CC = g++ -std=c++17

CFLAGS  = -Wall -Werror

CAFLAGS = -lwiringPi -lm

all: boat

boat: boat.cpp Windvane.o Compass.o GPS.o TinyGPS.o Servo.o packet.o lora.o RF.o LoRaComs.o Coms.o Logger.o Obstacle.o planner.o Level1.o
	$(CC) $(CFLAGS) -o boat boat.cpp Windvane.o Compass.o GPS.o TinyGPS.o Servo.o packet.o lora.o RF.o LoRaComs.o Coms.o Logger.o Obstacle.o planner.o Level1.o $(CAFLAGS)

Level1.o: Level1.hpp Level1.cpp
	$(CC) $(CFLAGS) -c Level1.hpp Level1.cpp $(CAFLAGS)

planner.o: planner.hpp planner.cpp
	$(CC) $(CFLAGS) -c planner.hpp planner.cpp $(CAFLAGS)
	
Obstacle.o: coms/Obstacle.hpp coms/Obstacle.cpp
	$(CC) $(CFLAGS) -c coms/Obstacle.hpp coms/Obstacle.cpp $(CAFLAGS)

Coms.o: coms/Coms.hpp coms/Coms.cpp
	$(CC) $(CFLAGS) -c coms/Coms.hpp coms/Coms.cpp $(CAFLAGS)

RF.o: coms/RF.hpp coms/RF.cpp
	$(CC) $(CFLAGS) -c coms/RF.hpp coms/RF.cpp $(CAFLAGS)

LoRaComs.o: coms/LoRaComs.hpp coms/LoRaComs.cpp
	$(CC) $(CFLAGS) -c coms/LoRaComs.hpp coms/LoRaComs.cpp $(CAFLAGS)

Logger.o: logger/Logger.hpp logger/Logger.cpp
	$(CC) $(CFLAGS) -c logger/Logger.hpp logger/Logger.cpp $(CAFLAGS)

Servo.o: servos/Servo.hpp servos/Servo.cpp
	$(CC) $(CFLAGS) -c servos/Servo.hpp servos/Servo.cpp $(CAFLAGS)

GPS.o: sensors/GPS.hpp sensors/GPS.cpp
	$(CC) $(CFLAGS) -c sensors/GPS.hpp sensors/GPS.cpp $(CAFLAGS)

Compass.o: sensors/Compass.hpp sensors/Compass.cpp
	$(CC) $(CFLAGS) -c sensors/Compass.hpp sensors/Compass.cpp $(CAFLAGS)

Windvane.o: sensors/Windvane.hpp sensors/Windvane.cpp
	$(CC) $(CFLAGS) -c sensors/Windvane.hpp sensors/Windvane.cpp $(CAFLAGS)

lora.o: include/lora/lora.cpp include/lora/lora.h
	$(CC) $(CFLAGS) -c include/lora/lora.h include/lora/lora.cpp $(CAFLAGS)

packet.o: include/lora/packet.cpp include/lora/packet.h
	$(CC) $(CFLAGS) -c include/lora/packet.h include/lora/packet.cpp $(CAFLAGS)

TinyGPS.o: include/gps/TinyGPS.cpp include/gps/TinyGPS.h
	$(CC) $(CFLAGS) -c include/gps/TinyGPS.h include/gps/TinyGPS.cpp $(CAFLAGS)

clean: cleanobj cleangch
	$(RM) a.out boat

cleanobj:
	$(RM) TinyGPS.o lora.o packet.o Windvane.o Compass.o GPS.o Servo.o Logger.o RF.o Coms.o LoRa.o Obstacle.o LoRaComs.o planner.o Level1.o

cleangch:
	$(RM) sensors/*.hpp.gch servos/*.hpp.gch logger/*.hpp.gch coms/*.hpp.gch include/lora/*.gch *.gch

cleanlogs:
	$(RM) Logs/*.txt Logs/*.csv
