#include <Arduino.h>
#include "BluetoothA2DPSink.h"

#define AUDIO_PLAYING 2

#define AMPLIFIER_ON 1
#define AMPLIFIER_OFF 0
#define MUTE_ON 0
#define MUTE_OFF 1

//calculated according to hardware specs
#define PSU_OVERCURRENT_PROTECTION_THR 1600 // 1600 adc units is 3A from battery
#define BATTERY_UNDERVOLTAGE_THR 2200 // 2200 adc units is 7.7V on battery

#define SAMPLING_THR 100

BluetoothA2DPSink a2dp_sink;

const int volumeUpPin = 4; //S5 on board
const int volumeDownPin = 21; //S2 on board
const int previousSongPin = 27; //S3 on board
const int nextSongPin = 19; //S1 on board
const int playOrPausePin = 13; //S4 on board

const int amplifierShutdownPin = 33;
const int amplifierMutePin = 32;

const int ADCPsuCurrentPin = 35;
const int ADCBatteryPin = 34;

uint8_t errorCounter = 0;

void setup() {

    delay(2000); //wait voltages to stabilize
    Serial.begin(9600); //debug

    pinMode(volumeUpPin, INPUT_PULLUP);
    pinMode(volumeDownPin, INPUT_PULLUP);
    pinMode(previousSongPin, INPUT_PULLUP);
    pinMode(nextSongPin, INPUT_PULLUP);
    pinMode(playOrPausePin, INPUT_PULLUP);

    pinMode(amplifierShutdownPin, OUTPUT);
    pinMode(amplifierMutePin, OUTPUT);

    a2dp_sink.start("Turboencabulator");

    digitalWrite(amplifierShutdownPin, AMPLIFIER_ON);
    digitalWrite(amplifierMutePin, MUTE_OFF);
}

void loop() {

    delay(10);

    if(analogRead(ADCBatteryPin) < BATTERY_UNDERVOLTAGE_THR || analogRead(ADCPsuCurrentPin) > PSU_OVERCURRENT_PROTECTION_THR) {
        errorCounter++;
        if(errorCounter > SAMPLING_THR) {
            Serial.println("Error state");
            digitalWrite(amplifierShutdownPin, AMPLIFIER_OFF);
            digitalWrite(amplifierMutePin, MUTE_ON);
            while(1) {
                sleep(1000);
            }    
        }
    }

    while(!a2dp_sink.is_connected()) {
        return;
    }

    if(!digitalRead(volumeDownPin)) {
          uint8_t volume = a2dp_sink.get_volume();
          volume--;
          a2dp_sink.set_volume(volume);
    }

    if(!digitalRead(volumeUpPin)) {
          uint8_t volume = a2dp_sink.get_volume();
          volume++;
          a2dp_sink.set_volume(volume);
    }

    if(!digitalRead(previousSongPin)) {
          a2dp_sink.previous();
    }

    if(!digitalRead(nextSongPin)) {
          a2dp_sink.next();
    }
    
    if(!digitalRead(playOrPausePin)) {
        if(AUDIO_PLAYING == a2dp_sink.get_audio_state()) {
            a2dp_sink.pause();
            return;
        }
        a2dp_sink.play();
    }
}