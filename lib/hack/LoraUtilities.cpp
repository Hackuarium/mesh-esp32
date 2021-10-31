#include "./common.h"
#include "./params.h"

char loraTemp[40];

void printLoraHelp(Print* output) {
  output->println(F("LoRa help"));
  output->println(F("(ai) Info"));
  output->println(F("(ar) Reset"));
  output->println(F("(as) Send message"));
  output->println(F("(ap) Send parameters"));
  output->println(F("(aa) set appskey"));
  output->println(F("(an) set nwkskey"));
  output->println(F("(ad) set devaddr"));
}

void processCommand(char command,
                             char* paramValue,
                             Print* output) {  // char and char* ??
  switch (command) {
   
    case 'a':
      setParameter("lora.appskey", paramValue);
      output->println(paramValue);
      break;
    case 'n':
      setParameter("lora.nwkskey", paramValue);
      output->println(paramValue);
      break;
    case 'd':
      setParameter("lora.devaddr", paramValue);
      output->println(paramValue);
      break;
   
    default:
      printLoraHelp(output);
  }
}


