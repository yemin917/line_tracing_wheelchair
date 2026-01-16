// 핀 정의
//int pin1 = 9;
int pin2 = 8;
int pin3 = 10;
int pin4 = 11;
int pin5 = 12;
//int pin6 = 3;

#define Trig 7
#define Echo 6
#define Buzzer 2

// 버튼 3개
const int btnRed   = 5;   // 빨간색 선택 버튼
const int btnBlue  = 4;   // 파란색 선택 버튼
const int btnYellow = 13;  // 노란색 선택 버튼

const int ldrAnalogPin = A0;
const int threshold = 500;

bool isStopped = false;
String command = "";
String selectedLine = "RED";  // 기본 선택

long duration;
int distance;

void setup() {
  Serial.begin(9600);
  
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(Buzzer, OUTPUT);

  //pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);
  pinMode(pin5, OUTPUT);
  //pinMode(pin6, OUTPUT);

  // 버튼 3개 입력 풀업 설정
  pinMode(btnRed, INPUT_PULLUP);
  pinMode(btnBlue, INPUT_PULLUP);
  pinMode(btnYellow, INPUT_PULLUP);
}

void loop() {
  readButtons();           // 버튼 입력 확인
  sendLineSelection();     // 선택된 색 전송

  // 초음파 거리 측정
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);

  duration = pulseIn(Echo, HIGH);
  distance = duration * 0.034 / 2;

  if (distance > 0 && distance <= 10) {
    if (!isStopped) {
      tone(Buzzer, 1000);
      stopMotors();
      isStopped = true;
    }
  } else {
    noTone(Buzzer);

    // 🔹 변경된 부분: 시리얼 입력 처리
    while (Serial.available() > 0) {
      char inChar = (char)Serial.read();
      if (inChar == '\n') {
        command.trim();

        if (command == "FORWARD") moveForward();
        else if (command == "LEFT") turnLeft();
        else if (command == "RIGHT") turnRight();
        else if (command == "BACK") moveBackward(); // ← 추가
        else stopMotors();


        command = ""; // 다음 명령 준비
      } else {
        command += inChar;
      }
    }

    isStopped = false;
  }

  int ldrValue = analogRead(ldrAnalogPin);
  //digitalWrite(ledPin, ldrValue < threshold ? HIGH : LOW);
}

// 버튼 읽기
void readButtons() {
  if (digitalRead(btnRed) == LOW) {
    selectedLine = "RED";
  } else if (digitalRead(btnBlue) == LOW) {
    selectedLine = "BLUE";
  } else if (digitalRead(btnYellow) == LOW) {
    selectedLine = "YELLOW";
  }
}

// 선택된 색을 라즈베리파이에 전송
void sendLineSelection() {
  Serial.println(selectedLine);
}

// 모터 제어 함수
void moveForward() {
  digitalWrite(pin2, HIGH); digitalWrite(pin3, LOW); //analogWrite(pin1, 180);
  digitalWrite(pin4, LOW);  digitalWrite(pin5, HIGH); //analogWrite(pin6, 180);
}

void turnLeft() {
  digitalWrite(pin2, HIGH); digitalWrite(pin3, LOW); //analogWrite(pin1, 255);
  digitalWrite(pin4, LOW);  digitalWrite(pin5, LOW); //analogWrite(pin6, 0);
}

void turnRight() {
  digitalWrite(pin2, LOW);  digitalWrite(pin3, LOW); //analogWrite(pin1, 0);
  digitalWrite(pin4, LOW);  digitalWrite(pin5, HIGH); //analogWrite(pin6, 255);
}

void stopMotors() {
  digitalWrite(pin2, LOW); digitalWrite(pin3, LOW); //analogWrite(pin1, 0);
  digitalWrite(pin4, LOW); digitalWrite(pin5, LOW); //analogWrite(pin6, 0);
}

void moveBackward() {
  digitalWrite(pin2, LOW);  digitalWrite(pin3, HIGH);  // Motor A 반대 회전
  digitalWrite(pin4, HIGH); digitalWrite(pin5, LOW);   // Motor B 반대 회전
  delay(70);
  stopMotors();
}