CC = g++

CFLAGS  = -Wall

CFLAGSA = -lwiringPi

all: serial_read gps_test lora_rx lora_tx telemetry compass servo windvane logger windvane2 rf

rf: rf.cpp
	$(CC) $(CFLAGS) -o rf rf.cpp $(CFLAGSA)

logger: logger.cpp TinyGPS.o
	$(CC) $(CFLAGS) -o logger logger.cpp TinyGPS.o $(CFLAGSA)

windvane: windvane.cpp
	$(CC) $(CFLAGS) -o windvane windvane.cpp $(CFLAGSA)

windvane2: windvane2.cpp
	$(CC) $(CFLAGS) -o windvane2 windvane2.cpp $(CFLAGSA)

servo: servo.cpp
	$(CC) $(CFLAGS) -o servo servo.cpp $(CFLAGSA)

compass: compass.cpp
	$(CC) $(CFLAGS) -o compass compass.cpp $(CFLAGSA)

telemetry: telemetry.cpp lora.o packet.o TinyGPS.o
	$(CC) $(CFLAGS) -o telemetry telemetry.cpp TinyGPS.o lora.o packet.o $(CFLAGSA)

lora_tx: lora_rx.cpp lora.o packet.o
	$(CC) $(CFLAGS) -o lora_tx lora_tx.cpp lora.o packet.o $(CFLAGSA)

lora_rx: lora_rx.cpp lora.o packet.o
	$(CC) $(CFLAGS) -o lora_rx lora_rx.cpp lora.o packet.o $(CFLAGSA)

lora.o: ../include/lora/lora.cpp ../include/lora/lora.h
	$(CC) $(CFLAGS) -c ../include/lora/lora.cpp $(CFLAGSA)

packet.o: ../include/lora/packet.cpp ../include/lora/packet.h
	$(CC) $(CFLAGS) -c ../include/lora/packet.cpp $(CFLAGSA)

serial_read: serial_read.c
	$(CC) $(CFLAGS) -o serial_read serial_read.c  $(CFLAGSA)

gps_test: gps_test.cpp TinyGPS.o
	$(CC) $(CFLAGS) -o gps_test gps_test.cpp TinyGPS.o  $(CFLAGSA)

TinyGPS.o: ../include/gps/TinyGPS.cpp ../include/gps/TinyGPS.h
	$(CC) $(CFLAGS) -c ../include/gps/TinyGPS.cpp $(CFLAGSA)

clean: cleanobj
	$(RM) serial_read gps_test lora_rx lora_tx telemetry compass servo a.out windvane logger windvane2 rf

cleanobj:
	$(RM) TinyGPS.o lora.o packet.o
