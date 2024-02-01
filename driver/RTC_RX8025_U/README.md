# EPSON RX8025

このライブラリは，
エプソンのRTC[RX8025][RX8025]用の
ドライバです．


このドライバでは，RTCを24時間モードで動作させる作りになっています．AM/PMの12時間制はサポートしていません．

## 動作検証


RTCは秋月電子の[AkizukiRTC_RX8025][ＲＸ－８０２５ＮＢ使用　Ｉ２Ｃ接続リアルタイムクロック（ＲＴＣ）モジュール]を使いました．

以下の表の機種で動作を確認しています．
| CPU | 機種 | 対応状況 |
|---|---|---|
| AVR | Arduino Mega | ○ |
| SAMD | Arduino MKR WiFi1010 | ○ |
| SAM | Arduino Due | ○ |
| ESP32 | スイッチサイエンスESP developer32 | ○ |

## 利用上の注意

``RTC_RX8025_U.h``に以下のようなデバッグや性能テストに用いるための機能を生かすフラグがあります．
必要に応じて有効・無効を変更してください．
```
#define DEBUG
```

## サンプルプログラム
サンプルプログラムは，本ドライバの各機能を実行した場合に，RTCの各レジスタが適切に設定されているか否かを
確認するためのものです．

``RTC_RX8025_U.h``の``DEBUG``定義を有効にした上で
コンパイルとインストールをしてください．

以下の行を無効にすることで，詳細なメッセージとレジスタの内容の確認が実行されます．
```
#undef DEBUG 
```

以下の行を有効にすると，各テストでレジスタの書き換えが行われた後に，全レジスタの内容をダンプします．
```
#define DUMP_REGISTER  // レジスタの値を書き換えた後に，レジスタ値のdumpを見たい場合はこれを有効にする(DEBUGも有効にする)
```

# API
詳細な動作を理解したい方はEPSONのアプリケーションマニュアルを参照しながら読んでください．

## 初期化
### オブジェクト生成
```
RTC_RX8025_U(TwoWire * theWire, int32_t rtcID=-1)
```
RTCが用いるI2CのI/FとIDを指定してオブジェクトを生成．
|引数|内容|
|---|---|
|theWire|I2CのI/F|
|rtcID|rtcに番号をつける場合に利用．(デフォルト値は-1)|

### 初期化
```
bool  begin(bool init, uint32_t addr=RTC_EPSON_RX8025_DEFAULT_ADRS)
```

第1引数は，時刻やタイマ，アラームの設定を実施するか否かを示すフラグで``false``の場合はI2C関係など最小限の初期化しか行いません．

I2Cのアドレスにデフォルトでないものを用いる場合は
第2引数で指定．指定しない場合は，デフォルトのアドレスで初期化を実施．

| 返り値 | 意味 |
|---|---|
|true|初期化成功|
|false|初期化失敗|

## RTCの情報の取得
RTCのチップの種類や機能の情報を取得するメンバ関数．
```
void  getRtcInfo(rtc_u_info_t *info)
```

## 時刻関連
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

## アラーム
RX8025のアラームは曜日，時，分を取り扱うアラームと時分だけのアラームの2種類があります．
以下の関数では，第1引数の``num``が0の場合は曜日を取り扱うもの，1の場合は時分だけのものを対象とします．

### アラーム設定
```
int   setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing)
```

RX8025は2種類のアラームがあり，第1引数``num``でそれを指定します．
``num``が0の場合は曜日，時，分が指定可能で1の場合は時と分のみが指定可能です．

また，RX8025のアラームには動作モードが存在しないため，第2引数は無視されます．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

#### 曜日の指定方法
1つの曜日だけを指定する場合，通常の曜日のデータ(0から6)を``timing.wday``に代入します．もし，複数の曜日を指定したい場合は以下の表を参考に指定したい曜日のデータの値を2進ORを計算して，その値を``timing.wday``を設定してください．

|曜日|7bit|6bit|5bit|4bit|3bit|2bit|0bit|
|---|---|---|---|---|---|---|---|---|
|日|1|0|0|0|0|0|0|1|
|月|1|0|0|0|0|0|1|0|
|火|1|0|0|0|0|1|0|0|
|水|1|0|0|0|1|0|0|0|
|木|1|0|0|1|0|0|0|0|
|金|1|0|1|0|0|0|0|0|
|土|1|1|0|0|0|0|0|0|

### アラームモード設定
```
int   setAlarmMode(uint8_t num, alarm_mode_t * mode)
```
RX8025のアラームは動作モードが存在しないため，何もせずに``RTC_U_UNSUPPORTED``を返します．

### アラーム制御
```
int   controlAlarm(uint8_t num, uint8_t action)
```
``action``を0に設定すると，アラームが停止，1に設定するとアラームが再開します．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## タイマ
RX8025はタイマが1種類しかないため，以下の関数の第1引数``num``は0のみとなります．
### タイマ設定
```
int   setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi)
```
第2引数modeの構造体メンバは以下の4種類がありますが，RX8025ではintervalのみが意味を持ちます．

```
uint8_t  pulse;
uint8_t  repeat;
uint8_t  useInteruptPin;
uint8_t  interval;
```

intervalの値はRX8025のctrl1レジスタの下3bit(CT2, CT1, CT0)の値(0から7)を用いる必要があります．とり得る値は8つあり，細かな
説明が必要になってしまうため，アプリケーションマニュアルを参照してください．

RTCの種類によっては，第2引数で設定した動作モードの整数倍に設定する機能がありますが，RX8025には存在しないため，
第3引数のmultiは無視します．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

### タイマのモード設定
```
int   setTimerMode(uint8_t num, rtc_timer_mode_t * mode)
```
第2引数は上の``setTimer()``と同じ．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

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
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## クロック信号出力
RX8025はクロック信号を出力する機能/端子は1つであるため，以下の関数の第1引数``num``の値は0に限定されます．
### クロック出力設定
```
int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1)
```

RX8025の信号出力機能は周波数を選択できないため，第2引数の``freq``は無視します．


また，RX8025はある端子(FOE端子)を外部から電圧をHIGHにしないと周波数信号を出力しないため，RTCのFOE端子と接続しているArduinoのピン番号を第3引数で与えます．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


### クロック出力の制御
```
int   controlClockOut(uint8_t num, uint8_t mode)
```
第2引数は以下の表のような意味となります．


|``mode``の値|意味|
|---|---|
|0|クロック出力停止|
|1|クロック出力開始|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## 割り込み関係
タイマやアラームで発生する割り込み関連機能．

### 割り込みの確認
```
int   checkInterupt(void)
```
割り込みの有無を16bitの2進数の列として出力．どのbitが立っているかでどの割り込みが発生しているかを判定することができる．
出力値はctrl2レジスタ(0x0f番)の下3bitが出力されるため，詳細はアプリケーションマニュアルを参照してください．


| 返り値 | 意味 |
|---|---|
|正の値|ctrl2レジスタの割り込み関連フラグの値|
|RTC_U_FAILURE |取得失敗|

|bit|名前|内容|
|---|---|---|
|2|CTFG|定周期割り込み発生(タイマ)|
|1|WAFG|曜日時分アラーム発生|
|0|DAFG|時分アラーム発生|


### 割り込みフラグの解除
```
int  clearInterupt(uint16_t type)
```
``checkInterupt()``の出力と同じく，消すフラグをbit列の中で0として表記して与える．

typeの下位3bitをそれぞれ，タイマ，曜日アラーム，時分アラームとみなして1となっている割り込みのフラグだけを消します．


| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## 電源電圧低下対応機能
### 電源フラグの取得
```
int checkLowPower(void)
```
RTCへの電源供給が切れ，時刻やタイマ等全ての設定をやり直す必要があるか否かを示すフラグの情報を取得する．負の値の場合はフラグ値の取得失敗，0以上の場合は読み取りに成功しています．

値としては，``0b010``の場合以外はなんらかの異常が起こっています．この値はctrl2レジスタの4～6bit目の値になるため，詳細はアプリケーションマニュアルを参照してください．


### 電源フラグのクリア
```
int clearPowerFlag(void)
```
電源喪失を示すフラグをクリアする関数．返り値が負の場合はフラグ値のクリアに失敗．成功の場合は``RTC_U_SUCCESS``が返されます．


### 電源電圧低下のしきい値設定
```
int setLowPower(uint8_t mode)
```
このRTCでは，どこまで電源電圧が低下したら検出対象とするかのしきい値電圧を設定することができます．

|``mode``の値|意味|
|---|---|
|0|2.1Vに設定(デフォルト)|
|1|1.3Vに設定|


| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


## 計時用発振器調整機能
NTPのように，時計がずれていた場合に，時計の進み方を遅くしたり，早めたりすることができます．そのための発信周波数設定の機能がこの関数となります．
```
int setOscillator(uint8_t mode)
```

引数の``mode``はDigital offsetレジスタ(0x07)に代入する値になります．詳細な情報はアプリケーションマニュアルを参照してください．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

```
int getOscillator(void)
```
返り値は以下の表の通りになります．0以上の値が取得できた場合の値の詳細情報はアプリケーションマニュアルを参照してください．

| 返り値 | 意味 |
|---|---|
|0以上|Digital offsetレジスタ(0x07)の値|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

### 2.10 時刻調整
EPSON RX8900は秒以下のカウンタをクリア(リセット)することができます．
```
int controlClock(void)
```
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|


[RX8025]:https://www5.epsondevice.com/ja/products/rtc/rx8025sa.html
[AkizukiRTC_RX8025]:https://akizukidenshi.com/catalog/g/gK-08585/
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
