#include <MIDI.h>
int note_no = 0;                //note value 1-61
int bend_range = 0;            // pitch bend range from 0-127
long after_bend_pitch = 0;      
byte note_on_count = 0;
unsigned long trigTimer = 0;
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI);

byte clock_count = 0;
byte clock_max = 24;//clock_max change by knob setting
byte clock_on_time = 0;
int clock_rate = 0;//knob CVin

// V/OCT LSB for DAC
const long cv[61] = {
 0,  68, 137,  205,  273,  341,  410,  478,  546,  614,  683,  751,
 819,  887,  956,  1024, 1092, 1161, 1229, 1297, 1365, 1434, 1502, 1570,       //Note voltage value array
 1638, 1707, 1775, 1843, 1911, 1980, 2048, 2116, 2185, 2253, 2321, 2389,
 2458, 2526, 2594, 2662, 2731, 2799, 2867, 2935, 3004, 3072, 3140, 3209,
 3277, 3345, 3413, 3482, 3550, 3618, 3686, 3755, 3823, 3891, 3959, 4028, 4095

};
// -----------------------------------------------------------------------------

// This function will be automatically called when a NoteOn is received.
// It must be a void-returning function with the correct parameters,
// see documentation here:
// https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-Callbacks

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  note_on_count ++;
  trigTimer = millis();
  note_no = pitch - 36 ;
  if (note_no < 0) {
         note_no = 0;
       }
       else if (note_no >= 61) {
         note_no = 60;
       }
  digitalWrite(5, HIGH); //GateをHIGH
OUT_CV(cv[note_no]);
 

}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
     note_on_count --;
     if (note_on_count == 0) {
       digitalWrite(5, LOW); //set Gate LOW
       }

}

void handlePitchBend(byte channel, int bend)
{
bend_range = (bend >> 7)+64;                                                 //Pitchbend 

if (bend_range > 64) {
         after_bend_pitch = cv[note_no] + cv[note_no] * (bend_range - 64) * 0.0004;
         if (after_bend_pitch>4095){
          after_bend_pitch=4095;
         }
         OUT_CV(after_bend_pitch);
       }

       else if (bend_range < 64) {
         after_bend_pitch = cv[note_no] - cv[note_no] * (64 - bend_range) * 0.0004;
         if (after_bend_pitch<0) {
          after_bend_pitch=0;
         }
         OUT_CV(after_bend_pitch);
       }

}
void handleClock(void)  //musical clock output 
{    
  clock_count ++;

       if (clock_count >= clock_max) {
         clock_count = 0;
       }

       if (clock_count == 1) {
         digitalWrite(4, HIGH);
       }
       else if (clock_count != 1) {
         digitalWrite(4, LOW);
       }
}
// -----------------------------------------------------------------------------

void setup()
{
  DDRA = B11111111;      //setting up mcu registers
  DDRC = B001111;
 pinMode(4, OUTPUT) ;//CLK_OUT
 pinMode(5, OUTPUT) ;//GATE_OUT
    // Connect the handleNoteOn function to the library,
    // so it is called upon reception of a NoteOn.
    MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function
    MIDI.setHandlePitchBend(handlePitchBend);
    // Do the same for NoteOffs
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandleClock(handleClock);
    // Initiate MIDI communications, listen to channel 1
    MIDI.begin(1);
}

void loop()
{
   clock_rate = analogRead(1);//read knob voltage
 
 if (clock_rate < 256) {
   clock_max = 24;//slow
 }
 else if (clock_rate < 512 && clock_rate >= 256) {
   clock_max = 12;
 }
 else if (clock_rate < 768 && clock_rate >= 512) {
   clock_max = 6;
 }
 else if (clock_rate >= 768) {
   clock_max = 3;//fast
 }
    MIDI.read();
 if (note_on_count != 0) {
   if ((millis() - trigTimer <= 20) && (millis() - trigTimer > 10)) {
     digitalWrite(5, LOW);
   }
   if ((trigTimer > 0) && (millis() - trigTimer > 20)) {
     digitalWrite(5, HIGH);
   }
 }
    // There is no need to check if there are messages incoming
    // if they are bound to a Callback function.
    // The attached method will be called automatically
    // when the corresponding message has been received.
}

void OUT_CV(int cv) {
byte hiCV = cv >> 8 ;
PORTA = cv;           //writing 1st 8 bits of cv value to register A of arduino
PORTC = hiCV;        //writing last 4 bits of cv value to register C of arduino
}
