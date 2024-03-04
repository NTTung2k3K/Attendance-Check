#include <SoftwareSerial.h>
#include <String.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <Servo.h>


//----------
//Fingerprint
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


//-----------
//keypad
const byte ROWS = 4;  // Số hàng của keypad
const byte COLS = 4;  // Số cột của keypad
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 4, 5, 6, 7 };    // Các chân nối đến các hàng
byte colPins[COLS] = { A0, A1, 8, 9 };  // Các chân nối đến các cột
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


//------------
//init ID
int id = 1;


//------------
//lcd
const int rs = 12, en = 13, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


#define RX_PIN 10  // Chân RX của cổng nối tiếp mềm
#define TX_PIN 11  // Chân TX của cổng nối tiếp mềm


const int MAX_STUDENTS = 20;  // Số lượng tối đa sinh viên


// Khai báo danh sách sinh viên
struct Student {
  String studentId;
  int fingerId;
};




Student students[MAX_STUDENTS];  // Danh sách sinh viên
int numStudents = 0;
char IdString[20];








//----------
//connect to esp
SoftwareSerial Arduino_SoftSerial(RX_PIN, TX_PIN);  //RX, TX
//---------
//Varialble data
char c;
String dataIn;
int8_t indexOfA;
String data1;








//-------------
//Setup

void setup() {
  //Serial
  Serial.begin(9600);
  //connect esp
  Arduino_SoftSerial.begin(9600);

  //fingerprint
  finger.begin(57600);
  //find finger print
  mySerial.listen();
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor 🙁");
  }
  // LCD
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);


  // //check ID has fingerPrint or not
  while (finger.loadModel(id) == 0) {
    id++;
  }


  delay(1000);
}


//----------
//main program

void loop() {
  Arduino_SoftSerial.listen();
  mySerial.listen();

  lcd.clear();
  //Security door
  char customKey;
  customKey = customKeypad.getKey();
  lcd.clear();
  lcd.print("A:Enroll");
  lcd.setCursor(0, 1);
  lcd.print("B:Verify");
  lcd.setCursor(9, 0);
  lcd.print("C:Empty");


  while (customKey == '\0') {
    customKey = customKeypad.getKey();
  }

  Serial.print("KEY: ");
  Serial.print(customKey);

  lcd.clear();


  lcd.clear();
  if (customKey == 'A') {


    lcd.print("Enroll mode");
    delay(2000);
    lcd.clear();
    delay(2000);
    String stuId = getStudentIdFromKeypad();
    bool checkExisted = checkStudentIdExists(stuId);
    if (checkExisted == false) {

      while (!getFingerprintEnroll(stuId))
        ;
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(stuId);
      lcd.setCursor(0, 1);
      lcd.print("Existed");
    }

  } else if (customKey == 'B') {


    lcd.print("Verify mode");
    delay(2000);
    while (!verifyFingerPrint())
      ;
  } else if (customKey == 'C') {
    emptyFingerDB();
  } else {
    lcd.print("Unknown");
    delay(1000);
    lcd.clear();
    lcd.print("Try again");
  }






  delay(1000);  // Đợi 1 giây trước khi gửi dữ liệu tiếp theo
}



String getStudentIdFromKeypad() {
  String studentId = "SE";  // Chuỗi lưu mã sinh viên

  lcd.print("Enrolling ID");
  lcd.setCursor(0, 1);
  lcd.print("SE");
  lcd.setCursor(2, 1);
  while (studentId.length() < 8) {  // Lấy đúng 6 kí tự
    char key = customKeypad.getKey();
    
    if (isDigit(key)) {
      studentId += key;  // Thêm kí tự vào chuỗi
      lcd.print(key);    // Hiển thị kí tự lên LCD
    }
  }

  return studentId;
}


// -------------------------------
// FUNCTION
uint8_t getFingerprintEnroll(String stuId) {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        lcd.clear();
        lcd.print("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        lcd.clear();
        lcd.print("Detecting");
        lcd.setCursor(0, 1);
        lcd.print("finger ID ");
        for (int i = 0; i < 3; i++) {
          if (IdString[i] >= '0' && IdString[i] <= '9')
            lcd.print(IdString[i]);
          else break;
        }
        lcd.print("...");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }
  delay(2000);








  // OK success!








  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      lcd.clear();
      lcd.print("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  delay(2000);
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match");
    lcd.clear();
    lcd.print("Match ID ");
    lcd.setCursor(0, 1);
    String studentId = getStudentIdByFingerId(finger.fingerID);
    lcd.print(studentId);

    delay(2000);
    lcd.clear();
    lcd.print("Enroll again");
    delay(2000);
    return false;
  }








  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }








  p = -1;
  lcd.clear();
  lcd.print("Place again");
  delay(2000);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        lcd.clear();
        lcd.print("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        lcd.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }
  delay(2000);








  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      lcd.clear();
      lcd.print("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  delay(2000);








  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);








  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
    lcd.clear();
    lcd.print("Prints matched!");
    delay(1000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    lcd.clear();
    lcd.print("Not match");
    delay(1000);
    return false;
  } else {
    Serial.println("Unknown error");
    return p;
  }








  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    addStudent(stuId, id);


    id++;
    lcd.clear();
    lcd.print("Success");
    lcd.setCursor(0, 1);
    lcd.print("Enrolling");
    delay(2000);
    lcd.clear();




  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }




  return true;
}








uint8_t verifyFingerPrint() {






  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        lcd.clear();
        lcd.print("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        lcd.clear();
        lcd.print("Detecting");
        lcd.setCursor(0, 1);
        lcd.print("finger");
        lcd.print("...");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      lcd.clear();
      lcd.print("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  delay(2000);
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    lcd.clear();
    lcd.print("Match ID ");
    lcd.setCursor(0, 1);
    String studentId = getStudentIdByFingerId(finger.fingerID);
    lcd.print(studentId);

    delay(2000);
    lcd.clear();
    lcd.print("Attendance");
    lcd.setCursor(0, 1);
    lcd.print("Success");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    Arduino_SoftSerial.listen();
    String data = studentId;                          // Dữ liệu bạn muốn gửi
    Arduino_SoftSerial.println(data);                 // Gửi dữ liệu qua cổng nối tiếp mềm
    Serial.println("Data sent to ESP8266: " + data);  // In thông báo qua cổng nối tiếp cứng




  }




  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    lcd.clear();
    lcd.print("Not exist");
    delay(2000);
    lcd.clear();
    lcd.print("Back to menu...");
    delay(2000);
    return true;
  } else {
    Serial.println("Unknown error");
    return p;
  }
  return true;
}








void emptyFingerDB() {




  id = 1;
  finger.emptyDatabase();
  lcd.clear();
  lcd.print("Empty DB success");
  delay(2000);
}








void printInteger(int number) {
  char string[100];
  sprintf(string, "%d", number);
  for (int i = 0; i < 3; i++) {
    if (string[i] >= '0' && string[i] <= '9')
      lcd.print(string[i]);
    else break;
  }
}








void Parse_the_data() {
  indexOfA = dataIn.indexOf("A");
  data1 = dataIn.substring(0, indexOfA);
}




void addStudent(String studentId, int fingerId) {
  // Kiểm tra xem danh sách đã đầy chưa
  if (numStudents < MAX_STUDENTS) {
    // Thêm sinh viên mới vào danh sách
    students[numStudents].studentId = studentId;
    students[numStudents].fingerId = fingerId;
    Serial.print("Student ID: ");
    Serial.println(students[numStudents].studentId);

    Serial.print("Finger ID: ");
    Serial.println(students[numStudents].fingerId);

    numStudents++;  // Tăng số lượng sinh viên
  } else {
    // Danh sách đã đầy
    Serial.println("Danh sách sinh viên đã đầy!");
  }
}

void printAllStudentIdAndFingerId() {
  for (int i = 0; i < numStudents; i++) {
    Serial.print("Student ID: ");
    Serial.print(students[i].studentId);
    Serial.print(", Finger ID: ");
    Serial.println(students[i].fingerId);
  }
}
String getStudentIdByFingerId(int fingerId) {
  for (int i = 0; i < numStudents; i++) {
    if (students[i].fingerId == fingerId) {
      return students[i].studentId;
    }
  }
  return "";  // Trả về chuỗi rỗng nếu không tìm thấy
}
bool checkStudentIdExists(String studentId) {
  for (int i = 0; i < numStudents; i++) {
    if (students[i].studentId == studentId) {
      return true;  // studentId tồn tại
    }
  }
  return false;  // studentId không tồn tại
}
