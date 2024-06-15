/*
사용 부품: LED, 초음파 센서, 블루투스 모듈, cds, 수온 센서
*/
#include <SoftwareSerial.h>
#include <Wire.h>
#include <MsTimer2.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DEBUG // 디버깅 메세지 출력
#define CDS_PIN A0 
#define LED_BUILTIN_PIN 13
#define TRIG 3
#define ECHO 4
#define ONE_WIRE_BUS 8

#define ARR_CNT 5 // 포인터 배열의 개수
#define CMD_SIZE 100 // 프로토콜 버퍼의 크기 60byte
 
char sendBuf[CMD_SIZE];
char recvId[10] = "LMG_ARD";  // SQL 저장 클라이이언트 ID
char userId[10] = "USER";
char stmId[10] = "ACT_STM";
bool ledOn = false;      // LED의 현재 상태 (on/off)
bool timerIsrFlag = false;
unsigned int secCount;
int cds=0;
int cds_cnt = 0;
int dis_cnt = 0;
bool cdsFlag = false;
bool disFlag = false;
bool tempFlag = false;
int getSensorTime;
float duration, distance, temp;
SoftwareSerial BTSerial(10, 11); // RX ==>BT:TXD, TX ==> BT:RXD

//수온 센서 준비
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

void setup()
{
#ifdef DEBUG
  Serial.begin(115200); 
  sensors.begin();
  Serial.println("setup() start!"); //디버그가 정의되어 있으면 출력
#endif
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
 
  BTSerial.begin(9600); // set the data rate for the SoftwareSerial port
  MsTimer2::set(1000, timerIsr); // 1000ms period
  MsTimer2::start();
}

void loop()
{
  if (BTSerial.available()) // 처리할게 있으면!!
    bluetoothEvent(); //클라이언트에서 메세지 보내면 처리

  if (timerIsrFlag)
  {
    // timerIsrFlag가 true 일 때 초음파 센서 TRIG HIGH
    // False일 때 LOW 동작....1초 간격
    digitalWrite(TRIG, HIGH);
    

    timerIsrFlag = false;

    //cds
    cds = map(analogRead(CDS_PIN), 0, 1023, 0, 100);

    // trig LOW되면서 값을 받고 초음파 센서 거리 계산
    digitalWrite(TRIG, LOW);
    duration = pulseIn(ECHO, HIGH);
    distance = ((float)(340 * duration) / 10000) / 2;

    //수온 센서
    sensors.requestTemperatures(); 
    temp = sensors.getTempCByIndex(0);

    Serial.print("Celsius temperature: ");
    Serial.print(temp);
    Serial.println();

    Serial.print("\nDistance: ");
    Serial.print(distance);
    Serial.println("cm\n");
    
    Serial.print("CDS: ");
    Serial.print(cds);
    Serial.println();
#ifdef DEBUG
    Serial.println();
#endif
    if(getSensorTime != 0 && !(secCount % getSensorTime)) {
      sprintf(sendBuf,"[%s]SENSOR@%d@%d@%d\n",recvId,cds,(int)temp, (int)distance);
      BTSerial.write(sendBuf);   
    }    

    // cds 측정 후 stm 클라이언트로 전송 >> 모터 동작
    if ((cds < 40 && cdsFlag))
    {
      cds_cnt++;
      if (cds_cnt >= 5)
      {
        cdsFlag = false;
        sprintf(sendBuf, "[%s]PUMP@ON\n",stmId);
        BTSerial.write(sendBuf, strlen(sendBuf));
        cds_cnt = 0;
      }
    }
    else if ((cds > 50) && !cdsFlag)
    {
      cdsFlag = true;
      sprintf(sendBuf, "[%s]PUMP@OFF\n", stmId);
      BTSerial.write(sendBuf, strlen(sendBuf));
    }

    // 초음파 거리 측정 후 유저에게 전송
    if ((distance > 5) && disFlag)
    {
      dis_cnt++;
      if (dis_cnt>= 5)
      {
        disFlag = false;
        sprintf(sendBuf, "[%s]먹이통을 채워주세요.\n",userId);
        BTSerial.write(sendBuf, strlen(sendBuf));
        dis_cnt = 0;
      }

    }
    else if ((distance < 5) && !disFlag)
    {
      disFlag = true;
    }

    if (temp < 29 && temp > 24)
    {
      tempFlag = true;
      sprintf(sendBuf, "[%s] 현재 수온이 %d*C로 정상입니다.\n",userId,(int)temp);
      BTSerial.write(sendBuf, strlen(sendBuf));

    }
    else if (temp >= 29 && tempFlag)
    {
      tempFlag = false;
      sprintf(sendBuf, "[%s] 현재 수온이 %d*C로 높은 편입니다.\n",userId,(int)temp);
      BTSerial.write(sendBuf, strlen(sendBuf));
    }
    else if (temp < 24 && tempFlag)
    {
      tempFlag = false;
      sprintf(sendBuf, "[%s] 현재 수온이 %d*C로 낮은 편입니다.\n",userId,(int)temp);
      BTSerial.write(sendBuf, strlen(sendBuf));
    }
  }

#ifdef DEBUG
  if (Serial.available())
    BTSerial.write(Serial.read());
#endif
}
void bluetoothEvent()
{
  int i = 0;
  char * pToken;
  char * pArray[ARR_CNT] = {0};
  char recvBuf[CMD_SIZE] = {0};
  int len = BTSerial.readBytesUntil('\n', recvBuf, sizeof(recvBuf) - 1); // \n이 들어 올때까지 recvBuf에 저장

#ifdef DEBUG
  Serial.print("Recv : ");
  Serial.println(recvBuf);
#endif

  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL)
  {
    pArray[i] =  pToken;
    if (++i >= ARR_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  }

  if (!strcmp(pArray[1], "LED")) {
    if (!strcmp(pArray[2], "ON")) {
      digitalWrite(LED_BUILTIN_PIN, HIGH);
    }
    else if (!strcmp(pArray[2], "OFF")) {
      digitalWrite(LED_BUILTIN_PIN, LOW);
    }
    sprintf(sendBuf, "[%s]%s@%s\n", pArray[0], pArray[1], pArray[2]); //[수신자]명령@인자
  }
  else if(!strcmp(pArray[1],"GETSENSOR"))
  {
    if(pArray[2] == NULL){
      getSensorTime = 0;
    }else {
      getSensorTime = atoi(pArray[2]);
      strcpy(recvId,pArray[0]);
    }
    sprintf(sendBuf,"[%s]SENSOR@%d@%d@%d\n",pArray[0],cds, (int)temp, (int)distance);
  }
  else if (!strncmp(pArray[1], " New", 4)) // New Connected
  {
    return ;
  }
  else if (!strncmp(pArray[1], " Alr", 4)) //Already logged
  {
    return ;
  }
  else
  {
    return;
  }
#ifdef DEBUG
  Serial.print("Send : ");
  Serial.print(sendBuf);
#endif
  BTSerial.write(sendBuf);
}
void timerIsr()
{
  timerIsrFlag = true;
  secCount++; // 1초마다 값 +1
}
