# Micro Crystal RV8803


このライブラリは，RTCを取り扱う統一的なAPIを作る思いつきの一部で
[Micro Crystal RV8803][RV8803]用の
ライブラリ(デバイスドライバ)です．

## 動作検証


| CPU | 機種 | 対応状況 |
|---|---|---|
| AVR | Arduino Mega | ○ |
| SAMD | Arduino MKR WiFi1010 | ○ |
| SAM | Arduino Due | ○ |
| ESP32 | スイッチサイエンスESP developer32 | ○ |

- 動作検証に用いたボード : [RV-8803-BOARD](https://www.marutsu.co.jp/pc/i/1556263/)

## 外部リンク

- データシート - [https://tamadevice.co.jp/pdf/mc/rtc/jpn/rv-8803-c7-datasheet-j.pdf](https://tamadevice.co.jp/pdf/mc/rtc/jpn/rv-8803-c7-datasheet-j.pdf)
- アプリケーションマニュアル - [https://tamadevice.co.jp/pdf/mc/rtc/RV-8803-C7_App-Manual_ja_1.pdf](https://tamadevice.co.jp/pdf/mc/rtc/RV-8803-C7_App-Manual_ja_1.pdf)
- 製品情報 - [https://www.microcrystal.com/jp/%E8%A3%BD%E5%93%81/%E3%83%AA%E3%82%A2%E3%83%AB%E3%82%BF%E3%82%A4%E3%83%A0%E3%82%AF%E3%83%AD%E3%83%83%E3%82%AF%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB/rv-8803-c7/](https://www.microcrystal.com/jp/%E8%A3%BD%E5%93%81/%E3%83%AA%E3%82%A2%E3%83%AB%E3%82%BF%E3%82%A4%E3%83%A0%E3%82%AF%E3%83%AD%E3%83%83%E3%82%AF%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB/rv-8803-c7/)

## 利用上の注意

``RTC_RV8803_U.h``に以下のようなデバッグや性能テストに用いるための機能を生かすフラグがあります．
必要に応じて有効・無効を変更してください．
```
#define DEBUG
```

## サンプルプログラム
サンプルプログラムは，本ドライバの各機能を実行した場合に，RTCの各レジスタが適切に設定されているか否かを
確認するためのものです．

``RTC_RV8803_U.h``の``DEBUG``定義を有効にした上で
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
以下の各関数の説明を理解するために，RTCの各レジスタのbitが持つ意味を知る必要がありますので，[データシート][データシート]を見ながら読んでください．

## 初期化関係
### オブジェクト生成
```
RTC_RV8803_U(TwoWire * theWire, int32_t rtcID=-1)
```
RTCが用いるI2CのI/FとIDを指定してオブジェクトを生成．

### 初期化
```
bool  begin(bool init=true, uint8_t addr=RTC_RV8803_DEFAULT_ADRS)
```
RTCの初期化．引数で``false``を与えると，時刻設定等を行わなず，インターフェイスの最小限の初期化のみを行うので，RTCに時刻設定を行った後にArduinoを再起動した場合に便利です．
第2引数は，RTCデフォルトのI2Cアドレスを用いない場合に指定してください．

| 返り値 | 意味 |
|---|---|
|true|初期化成功|
|false|初期化失敗|


## RTCの情報の取得
RTCのチップの種類や機能の情報を取得するメンバ関数．
```
void  getRtcInfo(rtc_u_info_t *info)
```


## 時刻情報関係
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


## 電源関係
本RTCは電源電圧低下を検出する機能があり，フラグレジスタ(0x0E)の下位2bit(0と1bit)に格納されています．この2bit分のデータを以下の関数でアクセスできます．

### 電源電圧低下等の情報を取得
```
int   checkLowPower(void)
```
フラグレジスタ(0x0E)の下位2bit分を読み出して返します．返り値が負の値の場合はエラーです．

### 電源電圧低下等の情報を消去
```
int   clearPowerFlag(void)
```
フラグレジスタ(0x0E)の下位2bit分を両方共に0に設定します．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## クロック信号出力
制御できる信号は1種類のため，以下の関数の第1引数``num``の値は0限定となります．

### 信号出力のON/OFF
```
int   controlClockOut(uint8_t num, uint8_t mode)
```

|``mode``の値 | 動作|
|---|---|
| 0 | 信号出力を止める |
| 1 | 信号出力を開始 |

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

### 信号の周波数設定
```
int   setClockOutMode(uint8_t num, uint8_t freq)
```

信号の周波数は拡張レジスタ(0x0D)の2,3bit(FD)の値の組み合わせで決まります．この2bitにfreqの値を上書きするため，freqは0から3の間の値を取ります．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

### 信号出力設定
```
int   setClockOut(uint8_t num, uint8_t freq, int8_t pin)
```

第3引数で与えられるピン番号を内部で保持した上で，``setClockOutMode(num, freq)``と``controlClockOut(num,1)``を順次実行する．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## 時計の進み方の制御
本RTCでは時計の進み方を調整するために，内部の時計の進み方を調整する機能``setOscillator()``と秒未満の内部カウンタを0にリセットする機能``controlClock()``があります．


### 進み方を制御するパラメータ設定
```
int   setOscillator(uint8_t mode)
```
オフセットレジスタ(0x2C)にmodeの値がそのまま書き込まれます．modeの意味はデータシートを参照してください．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

### 進み方を制御するパラメータの値の参照
```
int   getOscillator(void)
```

オフセットレジスタ(0x2C)にmodeの値を読み出して返します．0以上の場合はオフセットレジスタの値，負の場合はエラーです．

### 秒未満のリセット
```
int   controlClock(void)
```
制御レジスタ(0x0F)の最下位bit(RESET)に1を書き込み，内部のカウンタの秒未満部分を0に戻します．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

##  割り込み
本RTCでは，フラグレジスタ(0x0E)の2から5bitに電源関係以外の割り込み情報が格納されています．

### 割り込みの情報を取得
```
int   checkInterupt(void)
```

| 返り値 | 意味 |
|---|---|
|0以上 |フラグレジスタ(0x0E)の2から5bitの値|
|RTC_U_FAILURE |読み出し失敗|

### 割り込みの情報をクリア
```
int   clearInterupt(uint16_t type)
```
フラグレジスタ(0x0E)の2から5bitの値のうち，typeで指定(1がたったbit)を0にします．
例えば，フラグレジスタの2bitを0にする場合は，typeの最下位bitを1にします．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## アラーム
本RTCではアラームは1種類であるため，以下の関数の第1引数``num``の値は0限定となります．


### アラームの設定
```
int   setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing)
```

本RTCでは，アラームが発火する時刻を分・時・日(もしくは曜日)で指定できますが，日と曜日の両方を同時に指定することはできません．どちらを用いるかは``mode``のメンバ``type``で指定します．また，割り込み信号を外部に出すか否かを``mode``の``useInteruptPin``で指定します．

|modeの構造体メンバ|書き込み先|意味 |
|---|---|---|
|useInteruptPin|制御レジスタ(0x0F)のAIE(3bit)|アラーム発火でINTピンに割り込み信号を出す(1)か否(0)かの指定| 
|type|拡張レジスタ(0x0D)のWADA(6bit)|アラームの指定が日(1)か曜日(0)か|

実際にアラームが発火する時刻は``timing``で指定しますが，分・時・日(もしくは曜日)のうち，用いない項目については，0xFFをしておいてください．

例えば，以下のように``timing``の設定をすると，毎時1分にアラームが発火します．
```
timing.minute = 1;
timing.hour = 0xFF;
timing.wday = 0xFF;
timing.mday = 0xFF;
```

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

### アラームの割り込み信号の外部出力の切り替え
```
int   setAlarmMode(uint8_t num, alarm_mode_t * mode)
```
この関数では，``setAlarm()``の``mode``で指定する項目のうち，``useInteruptPin``の設定のみを変更します．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

### アラームのON/OFF
```
int   controlAlarm(uint8_t num, uint8_t action)
```
アラームのON/OFFを引数actionに応じて行います．

|actionの値|意味|
|---|---|
|0|アラームを止める|
|1|アラームを再開(一度止めたものに限る)|


| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


## タイマ
本RTCには2種類のタイマ機能があり，毎秒(もしくは毎分)割り込みが発生する定周期タイマと，通常のタイマ機能となります．以下の関数では，第1引数``num``が0の場合に定周期タイマ，``num``が1の場合に通常のタイマの制御を行います．

### タイマ設定と有効化
```
int   setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi)
```

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

#### ``num=0``の時の動作

定周期タイマは毎分か毎秒発火するため，引数``multi``は無視．
``mode``は以下のように利用されます．

|メンバ|用途|内容|
|---|---|---|
|uint8_t  pulse | 未使用 ||
|uint8_t  repeat | 未使用 ||
|uint8_t  useInteruptPin |制御レジスタ(0x0F)のUIE(5bit)に代入|割り込み信号を外部に出すか否か|
|uint8_t  interval |拡張レジスタ(0x0D)のUSEL(5bit)に代入|0:毎秒, 1:毎分|


#### ``num=1``の時の動作

``num=1``の場合は通常のカウントダウンタイマとなるため，引数``multi``は以下のように処理されます．
|multiのbit|処理|
|---|---|
|12から15|無視|
|8から11|タイマカウンタ1レジスタ(0x0C)に代入|
|0から7|タイマカウンタ0レジスタ(0x0B)に代入|

``multi``の処理後は``setTimerMode(1,mode)``, ``controlTimer(1,1)``の順で処理が行われます．

### タイマのパラメータ変更
```
int   setTimerMode(uint8_t num, rtc_timer_mode_t * mode)
```

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

#### ``num=0``の時の動作

``mode``は以下のように利用されます．

|メンバ|用途|内容|
|---|---|---|
|uint8_t  pulse | 未使用 ||
|uint8_t  repeat | 未使用 ||
|uint8_t  useInteruptPin |制御レジスタ(0x0F)のUIE(5bit)に代入|割り込み信号を外部に出すか否か|
|uint8_t  interval |拡張レジスタ(0x0D)のUSEL(5bit)に代入|0:毎秒, 1:毎分|

#### ``num=1``の時の動作

``mode``は以下のように利用されます．

|メンバ|用途|内容|
|---|---|---|
|uint8_t  pulse | 未使用 ||
|uint8_t  repeat | 未使用 ||
|uint8_t  useInteruptPin |制御レジスタ(0x0F)のTIE(4bit)に代入|割り込み信号を外部に出すか否か|
|uint8_t  interval |拡張レジスタ(0x0D)のTD(0,1bit)に代入|カウントダウンタイマの基準周波数選択|

### タイマのOn/OFF

```
int   controlTimer(uint8_t num, uint8_t action)
```

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


|``mode``の値 | 動作|
|---|---|
| 0 | タイマを止める |
| 1 | タイマ動作を開始 |

## 外部イベント
本RTCでは，外部から特定の端子にかかる電圧を調整することで，ある時点の秒と10ms単位の時間のスナップショットを取ることができます．これを「外部イベント」と呼びます．

### 外部イベントに対する設定
```
int   setEvent(event_mode_t *mode)
```

|modeのメンバ|値の利用先|
|---|---|
|uint8_t  useInteruptPin|制御レジスタ(0x0F)のEIE(2bit)|
|bool capture|イベント設定レジスタ(0x2F)のECP(7bit)|
|uint8_t level|イベント設定レジスタ(0x2F)のEHL(6bit)|
|uint8_t filter|イベント設定レジスタ(0x2F)のET(4,5bit)|
|bool reset|イベント設定レジスタ(0x2F)のERST(0bit)|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

### イベント情報の取得
```
int   getEvent(void)
```
| 返り値 | 意味 |
|---|---|
|0以上 |取得したスナップショットデータ(下で説明)|
|RTC_U_FAILURE |読み取り失敗|
|RTC_U_NO_EXTERNAL_EVENT|外部イベントは発生していない|

|返り値の各bit|意味|
|---|---|
|16から31|未使用|
|8から15|秒データ(レジスタ番号0x21)|
|0から7|10ms単位の時間(レジスタ番号0x20)|

## SRAM領域へのアクセス
RV8803ではレジスタ番号``0x07``がSRAM領域として利用できます．以下の2つの関数の第1引数``addr``が0の場合に，このレジスタを利用します．
また，``len``は1しか利用できません．

### SRAM領域からの読み取り
```
int getSRAM(uint8_t addr, uint8_t *array, uint16_t len)
```
レジスタ番号``0x07``のデータを配列arrayの最初の要素に代入されます．

### SRAM領域への書き込み
```
int setSRAM(uint8_t addr, uint8_t *array, uint16_t len)
```
配列arrayの最初の要素のデータをレジスタ番号``0x07``に書き込みます．


[データシート]:https://tamadevice.co.jp/pdf/mc/rtc/jpn/rv-8803-c7-datasheet-j.pdf
[アプリケーションマニュアル]:https://tamadevice.co.jp/pdf/mc/rtc/RV-8803-C7_App-Manual_ja_1.pdf
[RV-8803-BOARD]:https://www.marutsu.co.jp/pc/i/1556263/
[RV8803]:https://www.microcrystal.com/jp/%E8%A3%BD%E5%93%81/%E3%83%AA%E3%82%A2%E3%83%AB%E3%82%BF%E3%82%A4%E3%83%A0%E3%82%AF%E3%83%AD%E3%83%83%E3%82%AF%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB/rv-8803-c7/


[github]:https://github.com/sparkfun/SparkFun_DS3231_RTC_Arduino_Library
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



