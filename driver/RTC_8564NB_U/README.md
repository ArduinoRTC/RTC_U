# EPSON 8564NB

このライブラリは，RTCを取り扱う統一的なAPIを作る思いつきの一部で
私も以前良くつかっていたエプソンの[8564NB][RTC8564NB]用の
ドライバです．

## 動作検証
[秋月電子8564NB搭載モジュール][http://akizukidenshi.com/catalog/g/gI-00233/]を使いました．
また，動作を確認した機種は以下の表のものになります．

| CPU | 機種 | 対応状況 |
|---|---|---|
| AVR | Arduino Mega | ○ |
| SAMD | Arduino MKR WiFi1010 | ○ |
| SAM | Arduino Due | ○ |
| ESP32 | スイッチサイエンスESP developer32 | ○ |


## 利用上の注意

``RTC_8564NB_U.h``に以下のようなデバッグや性能テストに用いるための機能を生かすフラグがあります．
必要に応じて有効・無効を変更してください．
```
#define DEBUG
```

## サンプルプログラム
サンプルプログラムは，本ドライバの各機能を実行した場合に，RTCの各レジスタが適切に設定されているか否かを
確認するためのものです．

``RTC_8564NB_U.h``の``DEBUG``定義を有効にした上で
コンパイルとインストールをしてください．

以下の行を無効にすることで，詳細なメッセージとレジスタの内容の確認が実行されます．
```
#undef DEBUG 
```

以下の行を有効にすると，各テストでレジスタの書き換えが行われた後に，全レジスタの内容をダンプします．
```
#define DUMP_REGISTER  // レジスタの値を書き換えた後に，レジスタ値のdumpを見たい場合はこれを有効にする(DEBUGも有効にする)
```

## 外部リンク
- RTC-8564NB - [https://www5.epsondevice.com/ja/products/rtc/rtc8564nb.html][RTC8564NB]
- 秋月電子8564NB搭載モジュール - [http://akizukidenshi.com/catalog/g/gI-00233/][AkizukiRTC8564NB]


# API

本ドキュメントとEPSONのアプリケーションマニュアルをあわせて読んでください．

## 初期化
### オブジェクト生成
```
RTC_8564NB_U(TwoWire * theWire, int32_t rtcID=-1)
```
RTCが用いるI2CのI/FとIDを指定してオブジェクトを生成．
|引数|内容|
|---|---|
|theWire|I2CのI/F|
|rtcID|rtcに番号をつける場合に利用．(デフォルト値は-1)|

### 初期化
```
bool  begin(bool init, uint32_t addr=RTC_EPSON_8564NB_DEFAULT_ADRS)
```

第1引数は，時刻やタイマ，アラームの設定を実施するか否かを示すフラグ．

EPSON　8564NBのデフォルトI2Cのアドレスではない番号を持つモジュールを
用いる場合は，第2引数で指定．指定しない場合は，デフォルトのアドレスで初期化を実施．

| 返り値 | 意味 |
|---|---|
|true|初期化成功|
|false|初期化失敗|

## RTCの情報の取得
RTCのチップの種類や機能の情報を取得するメンバ関数．
```
void  getRtcInfo(rtc_u_info_t *info)
```

## 時刻関係機能
時刻に関する機能で使うデータ型は``dateUtils.h``で以下のように定義されています．
```
typedef struct  {
  uint16_t  year;
  uint8_t   month;
  uint8_t   mday;
  uint8_t   wday;
  uint8_t   hour;
  uint8_t   minute;
  uint8_t   second;
  int16_t   millisecond;
} date_t;
```

enum {
  SUN = 0,
  MON = 1,
  TUE = 2,
  WED = 3,
  THU = 4,
  FRI = 5,
  SAT = 6
};

### 時刻設定
```
bool  setTime(date_t* time)
```
引数で与えた時刻をRTCに設定．
| 返り値 | 意味 |
|---|---|
|true|設定成功|
|false|設定失敗|

### 時刻取得
```
bool  getTime(date_t* time)
```
RTCから取得した時刻情報を引数で与えた構造体に格納．
| 返り値 | 意味 |
|---|---|
|true|取得成功|
|false|取得失敗|

## アラーム関連機能
本RTC(8564)では，アラームは1種類しかないため，以下の各API関数の第1引数``num``は0限定となります．
### アラーム設定
```
int   setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing)
```
8564NBのアラームは，日付，曜日，時間，分を設定でき，それを第3引数で指定．モードは``setAlarmMode()``を参照．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_UNSUPPORTED |サポートしていないパラメータの設定など|

### アラームモード設定
```
int   setAlarmMode(uint8_t num, alarm_mode_t * mode)
```
アラームが発生したタイミングでの割り込みピンの制御を行うか否かを設定する．
``mode``の構造体メンバ``useInteruptPin``が0の場合は割り込みピンを制御せず，1の場合は制御します．modeのメンバ``type``は無視します．

```
typedef struct {
    uint8_t  useInteruptPin;
    uint8_t  type;
} alarm_mode_t;

```

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_UNSUPPORTED |サポートしていないパラメータの設定など|

### アラーム制御
```
int   controlAlarm(uint8_t num, uint8_t action)
```
``action``を0に設定すると，アラームが停止，1の場合は動作再開します．``setAlarm()``や``setAlarmMode()``の実行を
事前にしていない状態で``controlAlarm(0,1)``を呼び出した場合の動作は不定です．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_UNSUPPORTED |サポートしていないパラメータの設定など|

## タイマ関連機能
アラームと同様に，タイマ機能も1種類しかないため，以下の関数の第1引数の値は``0``限定となります．
### タイマ設定
```
int   setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi)
```
第2引数modeの構造体メンバの意味は以下の通り．
|構造体メンバ|0|1|2|3|
|---|---|---|---|---|
|``repeat``|リピートなし|リピートあり|||
|``useInteruptPin``|割り込みピン利用せず|割り込みピン使用する|||
|``interval``|4096Hz|64Hz|1秒|1分|

第3引数のmultiは第2引数の``interval``の周期の``multi``倍でタイマが発火する設定を行う．
ただし，本RTCでは倍数は``8bit``であるため，``255``以上の値を入れた場合の動作は保証できません．

タイマの周期とその倍数(カウンタ)の値についてはアプリケーションマニュアルを参照してください．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_UNSUPPORTED |サポートしていないパラメータの設定など|

### タイマのモード設定
```
int   setTimerMode(uint8_t num, rtc_timer_mode_t * mode)
```
第2引数は上の``setTimer()``と同じ．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_UNSUPPORTED |サポートしていないパラメータの設定など|

### タイマの制御
```
int   controlTimer(uint8_t num, uint8_t action)
```
第2引数``action``の意味は以下の通り．

|``action``の値|意味|
|---|---|
|0|タイマ停止|
|1|タイマ再開|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_UNSUPPORTED |サポートしていないパラメータの設定など|

## クロック信号出力関連
本RTCは周波数信号を出す端子/機能は1つしかないので，以下の関数の第1引数は0限定です．
### クロック出力設定
```
int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1)
```
クロックが出力の制御に用いるピンの番号(foeピンに接続しているArduinoのピン番号)と，周波数を引数に与えて実行します．
|``freq``の値|クロック周波数|
|---|---|
|0|32kHz|
|1|1kHz|
|2|32Hz|
|3|1Hz|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_UNSUPPORTED |サポートしていないパラメータの設定など|

### クロック周波数設定
```
int   setClockOutMode(uint8_t num, uint8_t freq)
```
クロックの周波数のみを設定．

|``freq``の値|クロック周波数|
|---|---|
|0|4096Hz|
|1|64Hz|
|2|1秒|
|3|1分|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_UNSUPPORTED |サポートしていないパラメータの設定など|

### クロック出力の制御
```
int   controlClockOut(uint8_t num, uint8_t mode)
```
|``mode``の値|意味|
|---|---|
|0|クロック出力停止|
|1|クロック出力開始|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_UNSUPPORTED |サポートしていないパラメータの設定など|

## 割り込みフラグ関連
### 割り込みの確認
```
int   checkInterupt(void)
```
割り込みの有無を16bitの2進数の列として出力．どのbitが立っているかでどの割り込みが発生しているかを判定することができる．


| 返り値 | 意味 |
|---|---|
|0以上 |control2レジスタ(0x01番)の2,3bit目の値|
|RTC_U_FAILURE |取得失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

返り値が0以上の場合は2bitの値を返しますが，0の場合はアラーム，タイマどちらの割り込みも発生しておらず，1の場合はタイマ，2の場合はアラーム，3の場合は両方の割り込みが発生したことを示します．

### 割り込みフラグの解除
```
int  clearInterupt(uint16_t type)
```
``checkInterupt()``の出力と同じく，消すフラグをbit列の中で1として表記して与える．

最下位bit(0bit目)が1の場合はタイマのフラグを消し，1bit目が1の場合はアラームのフラグを消す．
両方1の場合(値が3の場合)は両方消します．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## 電源電圧低下関連
### 電源フラグの取得
```
int checkLowPower(void)
```
RTCへの電源供給が切れ，時刻やタイマ等全ての設定をやり直す必要があるか否かを示すフラグの情報を取得する．負の値の場合はフラグ値の取得失敗，正の値の場合は電源喪失が発生したことを示す．0の場合は電源喪失が起こっていない．


### 電源フラグのクリア
```
int clearPowerFlag(void)
```
電源喪失を示すフラグをクリアする関数．返り値が負の場合はフラグ値のクリアに失敗．成功の場合は``RTC_U_SUCCESS``が返されます．


## 未サポート機能
``RTC_Unified``クラスのメンバ関数のうち，``RTC_8564NB_U``クラスで定義されていない
関数を呼び出した場合はなにもせずに``RTC_U_UNSUPPORTED``を返します．


[RTC8564NB]:https://www5.epsondevice.com/ja/products/rtc/rtc8564nb.html
[AkizukiRTC8564NB]:http://akizukidenshi.com/catalog/g/gI-00233/
[original]:http://zattouka.net/GarageHouse/micon/Arduino/RTC/RTC.htm
[Grove]:https://www.seeedstudio.io/category/Grove-c-1003.html
[SeedStudio]:https://www.seeedstudio.io/
[AdafruitUSD]:https://github.com/adafruit/Adafruit_Sensor
[shield]:https://www.seeedstudio.com/Base-Shield-V2-p-1378.html
[M0Pro]:https://store.arduino.cc/usa/arduino-m0-pro
[Due]:https://store.arduino.cc/usa/arduino-due
[Uno]:https://store.arduino.cc/usa/arduino-uno-rev3
[UnoWiFi]:https://store.arduino.cc/usa/arduino-uno-wifi-rev2
[Mega]:https://store.arduino.cc/usa/arduino-mega-2560-rev3
[LeonardoEth]:https://store.arduino.cc/usa/arduino-leonardo-eth
[ProMini]:https://www.sparkfun.com/products/11114
[ESPrDev]:https://www.switch-science.com/catalog/2652/
[ESPrDevShield]:https://www.switch-science.com/catalog/2811/
[ESPrOne]:https://www.switch-science.com/catalog/2620/
[ESPrOne32]:https://www.switch-science.com/catalog/3555/
[Grove]:https://www.seeedstudio.io/category/Grove-c-1003.html
[SeedStudio]:https://www.seeedstudio.io/
[Arduino]:http://https://www.arduino.cc/
[Sparkfun]:https://www.sparkfun.com/
[SwitchScience]:https://www.switch-science.com/



<!--- コメント

## 動作検証

|CPU| 機種 |ベンダ| 結果 | 備考 |
| :--- | :--- | :--- | :---: | :--- |
|AVR| [Uno R3][Uno]  |[Arduino][Arduino]|    ○  |      |
|       | [Mega2560 R3][Mega] |[Arduino][Arduino] |  ○    |      |
|       | [Leonardo Ethernet][LeonardoEth] |[Arduino][Arduino] |  △   |   割り込みピンが使えない? (D3,D4はだめだった)   |
|       | [Uno WiFi][UnoWiFi] |[Arduino][Arduino] |    ○  | |
|       | [Pro mini 3.3V][ProMini] | [Sparkfun][Sparkfun] | ○     |      |
| ARM/M0+ | [M0 Pro][M0Pro] |[Arduino][Arduino] |○||
|ESP8266|[ESPr developer][ESPrDev]| [スイッチサイエンス][SwitchScience] |○|D14|
|ESP32 | [ESPr one 32][ESPrOne32] | [スイッチサイエンス][SwitchScience] |○|　D13|

## 外部リンク

- Seeed Studio技術Wiki [http://wiki.seeedstudio.com/Grove-3-Axis_Digital_Accelerometer-1.5g/][SeedWiki]
- センサ商品ページ [https://www.seeedstudio.com/Grove-3-Axis-Digital-Accelerometer-1-5-p-765.html][ProductPage]
- ソースリポジトリ [https://github.com/Seeed-Studio/Accelerometer_MMA7660][github]
- Adafruit Unified Sensor Driver - [https://github.com/adafruit/Adafruit_Sensor][AdafruitUSD]
- Groveシールド - [https://www.seeedstudio.com/Base-Shield-V2-p-1378.html][shield]
- Arduino M0 Pro - [https://store.arduino.cc/usa/arduino-m0-pro][M0Pro]
- Arduino Due - [https://store.arduino.cc/usa/arduino-due][Due]
- Arduino Uno R3 - [https://store.arduino.cc/usa/arduino-uno-rev3][Uno]
- Arduino Uno WiFi - [https://store.arduino.cc/usa/arduino-uno-wifi-rev2][UnoWiFi]
- Arduino Leonardo Ethernet - [https://store.arduino.cc/usa/arduino-leonardo-eth][LeonardoEth]
- Arduino Mega2560 R3 - [https://store.arduino.cc/usa/arduino-mega-2560-rev3][Mega]
- Arduino Pro mini 328 - 3.3V/8MHz - [https://www.sparkfun.com/products/11114][ProMini]
- ESPr developer - [https://www.switch-science.com/catalog/2652/][ESPrDev]
- ESPr Developer用GROVEシールド - [https://www.switch-science.com/catalog/2811/][ESPrDevShield]
- ESpr one - [https://www.switch-science.com/catalog/2620/][ESPrOne]
- ESPr one 32 - [https://www.switch-science.com/catalog/3555/][ESPrOne32]
- Grove - [https://www.seeedstudio.io/category/Grove-c-1003.html][Grove]
- Seed Studio - [https://www.seeedstudio.io/][SeedStudio]
- Sparkfun Electronics - [https://www.sparkfun.com/][Sparkfun]
- スイッチサイエンス - [https://www.switch-science.com/][SwitchScience]


[Grove]:https://www.seeedstudio.io/category/Grove-c-1003.html
[SeedStudio]:https://www.seeedstudio.io/
[ProductPage]:https://www.seeedstudio.com/Grove-3-Axis-Digital-Accelerometer-1-5-p-765.html
[SeedWiki]:http://wiki.seeedstudio.com/Grove-3-Axis_Digital_Accelerometer-1.5g/
[github]:https://github.com/Seeed-Studio/Accelerometer_MMA7660
[AdafruitUSD]:https://github.com/adafruit/Adafruit_Sensor
[shield]:https://www.seeedstudio.com/Base-Shield-V2-p-1378.html
[M0Pro]:https://store.arduino.cc/usa/arduino-m0-pro
[Due]:https://store.arduino.cc/usa/arduino-due
[Uno]:https://store.arduino.cc/usa/arduino-uno-rev3
[UnoWiFi]:https://store.arduino.cc/usa/arduino-uno-wifi-rev2
[Mega]:https://store.arduino.cc/usa/arduino-mega-2560-rev3
[LeonardoEth]:https://store.arduino.cc/usa/arduino-leonardo-eth
[ProMini]:https://www.sparkfun.com/products/11114
[ESPrDev]:https://www.switch-science.com/catalog/2652/
[ESPrDevShield]:https://www.switch-science.com/catalog/2811
[ESPrOne]:https://www.switch-science.com/catalog/2620/
[ESPrOne32]:https://www.switch-science.com/catalog/3555/
[Grove]:https://www.seeedstudio.io/category/Grove-c-1003.html
[SeedStudio]:https://www.seeedstudio.io/
[Arduino]:http://https://www.arduino.cc/
[Sparkfun]:https://www.sparkfun.com/
[SwitchScience]:https://www.switch-science.com/


[Adafruit Unified Sensor Driver][AdafruitUSD]
[Groveシールド][shield]
[Arduino M0 Pro][M0Pro]
[Arduino Due][Due]
[Arduino Uno R3][Uno]
[Arduino Mega2560 R3][Mega]
[Arduino Leonardo Ethernet][LeonardoEth]
[Arduino Pro mini 328 - 3.3V/8MHz][ProMini]
[ESpr one][ESPrOne]
[ESPr one 32][ESPrOne32]
[Grove][Grove]
[Seed Studio][SeedStudio]
[Arduino][Arduino]
[Sparkfun][Sparkfun]
[スイッチサイエンス][SwitchScience]
--->
