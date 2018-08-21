//global declarations

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];
boolean newData = false;
char actionFromPi[numChars] = {0};
int channelFromPi = 0;
int delayFromPi = 0;

const int buttonPin = 12;
const int ledPin = 13;
int buttonState = 0;
int lastButtonState = HIGH;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; 

void setup() {
  Serial.begin(9600);
  Serial.println("Arduino CLI PDU ready to receive commands");
  Serial.println("ex. <high/low/reset/sequence/status/map,channel(#),delay(ms)>");
  Serial.println("=============================================================");
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(2, HIGH);      //Router
  digitalWrite(3, HIGH);      //Modem
  digitalWrite(4, HIGH);      //DVR
  digitalWrite(5, HIGH);      //Central PSU
  digitalWrite(6, HIGH);      //POE-AP
  digitalWrite(7, HIGH);      //Switch
  digitalWrite(8, HIGH);      //Exhaust Fan
  digitalWrite(9, HIGH);      //Intake Fan
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  cliInput();
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    parseData();
    printParsedData();
    decodeAction();
    newData = false;
  }
  int buttonReading = digitalRead(buttonPin);
  if (buttonReading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = buttonReading;
      if (buttonState == LOW) {
        delay(500);
        digitalWrite(ledPin, HIGH);
        digitalWrite(6, !digitalRead(6));
        Serial.print("AP button triggered ");
        Serial.println(digitalRead(6));
        delay(10000);
        Serial.println(">>>");
        digitalWrite(ledPin, LOW);
      }
    }
  }
  lastButtonState = buttonReading;
}

void cliInput() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0';
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void parseData() {

  char * strtokIndx;

  strtokIndx = strtok(tempChars, ",");
  strcpy(actionFromPi, strtokIndx);

  strtokIndx = strtok(NULL, ",");
  channelFromPi = atoi(strtokIndx) + 1;

  strtokIndx = strtok(NULL, ",");
  delayFromPi = atoi(strtokIndx);
}

void printParsedData() {
  Serial.println("Executing command with ff parameters:");
  Serial.println("-------------------------------------");
  Serial.print("   Action: ");
  Serial.println(actionFromPi);
  Serial.print("   Channel: ");
  Serial.println(channelFromPi - 1);
  Serial.print("   Delay: ");
  Serial.println(delayFromPi);
  Serial.println("-------------------------------------");
  delay(1000);
}

void decodeAction() {
  if (strcmp(actionFromPi, "high") == 0) {
    turnHighPin();
    commandExecuted();
  }
  else if (strcmp(actionFromPi, "low") == 0) {
    if (channelFromPi > 3) {
      turnLowPin();
      commandExecuted();
    }
    else {
      Serial.println("Rejecting command. Turning off Modem & Router disabled!");
      Serial.println("=============================================================");
    }
  }
  else if (strcmp(actionFromPi, "reset") == 0) {
    resetPin();
    commandExecuted();
  }
  else if (strcmp(actionFromPi, "sequence") == 0) {
    resetSequence();
    commandExecuted();
  }
  else if (strcmp(actionFromPi, "status") == 0) {
    pinStatus();
    commandExecuted();
  }
  else if (strcmp(actionFromPi, "map") == 0) {
    channelMap();
    commandExecuted();
  }
  else {
    Serial.println("Command not recognized!!!");
    Serial.println("=============================================================");
  }
}

void turnHighPin() {
  digitalWrite(channelFromPi, HIGH);
}

void turnLowPin() {
  digitalWrite(channelFromPi, LOW);
}

void resetPin() {
  digitalWrite(channelFromPi, !digitalRead(channelFromPi));
  delay(delayFromPi);
  digitalWrite(channelFromPi, !digitalRead(channelFromPi));
}

void resetSequence() {
  if (channelFromPi - 1 == 0) {
    for (int i = 2; i <= 5; i++) {
      digitalWrite(i, LOW);
      delay(1000);
    }
    delay(delayFromPi);
    for (int i = 2; i <= 5; i++) {
      digitalWrite(i, HIGH);
      delay(2000);
    }
  }
  else if (channelFromPi - 1 == 1) {
    for (int i = 2; i <= 9; i++) {
      digitalWrite(i, LOW);
      delay(1000);
    }
    delay(delayFromPi);
    for (int i = 2; i <= 9; i++) {
      digitalWrite(i, HIGH);
      delay(2000);
    }
  }
}

void pinStatus() {
  Serial.println("Channel Status");
  Serial.println("--------------");
  for (int i = 2; i <= 9; i++) {
    Serial.print(i - 1);
    Serial.print(": ");
    if (digitalRead(i) == 1) {
      Serial.print("HIGH");
    }
    else if (digitalRead(i) == 0) {
      Serial.print("LOW");
    }
    Serial.println();
  }
  Serial.println("--------------");
}

void channelMap() {
  Serial.println("Channel Mapping");
  Serial.println("---------------");
  Serial.println("1: Router");
  Serial.println("2: Modem");
  Serial.println("3: DVR");
  Serial.println("4: CCTV PSU");
  Serial.println("5: POE-AP");
  Serial.println("6: Switch");
  Serial.println("7: Exhaust Fan");
  Serial.println("8: Intake Fan");
  Serial.println("---------------");
}

void commandExecuted() {
  delay(1000);
  Serial.print(actionFromPi);
  Serial.print(" command executed\n");
  Serial.println("=============================================================");
}
