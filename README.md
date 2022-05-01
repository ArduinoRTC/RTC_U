# RTC_U
このライブラリは，いろいろなセンサのインターフェイスを統一する[プロジェクト][AdafruitUSD]を
参考に，「RTCのAPIも統一できたらプログラミングが楽になるかも」と思い作ってみたAPIの定義と
手元に転がっていたRTCを使ってArduino用のライブラリ(デバイスドライバ)の組み合わせとなって
います．

## 1. ライセンスについて
各RTCのライブラリのライセンスは各ディレクトリの中身を見てください．
各ドライバはどなたかが作ったコードのインターフェイス部分を改造しているので，もとのソースのライセンス条件を引き継いでいます．

## 2. 対応済みRTC
|ベンダ| 型番 |
| :--- | :--- |
|エプソン|[RTC 8564NB][RTC8564NB]|
|MAXIM|[DS1307][DS1307]|
|MAXIM|[DS3234][DS3234]|

検証には，以下のモジュールを使いました．
- 秋月電子8564NB搭載モジュール - [http://akizukidenshi.com/catalog/g/gI-00233/][AkizukiRTC8564NB]
- Grove RTC - [https://www.seeedstudio.com/Grove-RTC.html][GroveRTC]
- SparkFun DeadOn RTC Breakout DS3234 - [https://www.sparkfun.com/products/10160][BOB-10160]

## 3. 利用上の注意
個人の週末のお楽しみで作っているので，各ドライバの網羅的なテストはできていません．
また，私自身はロジックアナライザ等は持っていないので，SPIやI2Cレベルでの動作が
おかしい場合はデバッグできないので放置状態です．

## 4. 利用方法
まず，このディレクトリをArduino開発環境のライブラリ用のディレクトリに置きます．
次に，driverディレクトリの中から利用するRTC用のディレクトリも同じくライブラリ用ディレクトリに
移動(もしくはコピー)します．

[dateUtilsライブラリ](https://github.com/houtbrion/dateUtils)をインストールしてください．

以上で，準備はOKです．

## 5. 外部リンク
- Adafruit Unified Sensor Driver - [https://github.com/adafruit/Adafruit_Sensor][AdafruitUSD]
- RTC-8564NB - [https://www5.epsondevice.com/ja/products/rtc/rtc8564nb.html][RTC8564NB]
- 秋月電子8564NB搭載モジュール - [http://akizukidenshi.com/catalog/g/gI-00233/][AkizukiRTC8564NB]
- DS1307 - [https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS1307.html][DS1307]
- Grove RTC - [https://www.seeedstudio.com/Grove-RTC.html][GroveRTC]
- DS3234 - [https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS3234.html][DS3234]
- SparkFun DeadOn RTC Breakout DS3234 - [https://www.sparkfun.com/products/10160][BOB-10160]

# APIマニュアル

## 1. 構造体定義など
### 1.1. RTCの機能を格納する構造体
```
typedef struct {
    uint32_t    type;                   // チップの種類
    uint8_t     numOfInteruptPin;       // 割り込みピンの数
    uint8_t     numOfAlarm;             // 設定できるアラームの数
    uint8_t     numOfTimer;             // 設定できるタイマの数
    uint8_t     numOfClockOut;          // クロック周波数を出力するピン数
    uint8_t     numOfYearDigits;        // 年を格納するレジスタの桁数
    bool        haveYearOverflowBit;    // 年を格納するレジスタが2桁の場合にオーバーフローをした場合に利用するフラグの有無
    bool        haveMilliSec;           // ミリ秒を取り扱う機能の有無
    bool        independentSQW;         // 割り込みピンと周波数出力ピンが独立しているか否か
} rtc_info_t;
```

### 1.2. タイマの動作モード
```
typedef struct {
    int8_t  repeat;
    int8_t  useInteruptPin;
    int8_t  interval;
} timer_mode_t;
```

### 1.3. アラームの動作モード
```
typedef struct {
    int8_t  useInteruptPin;
} alarm_mode_t;
```

### 1.4. RTCの種類
```
enum {
    EPSON8564NB  = 1,
    EPSON_RX8025,
    EPSON_RX8900,
    EPSON_4543SA,
    DS1307,
    DS3231,
    DS3234,
    PCF8523,
    RV8803
};
```

### 1.5 dateUtils.hのデータ型等
本ライブラリで利用する「dateUtils.h」のデータ型は以下の通り．

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
```

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

## 2. メンバ関数

以下に説明するメンバ関数のうち，時刻の設定取得以外はRTCの種類によっては機能が存在しないものがあります．そのため，機能が存在しないRTCでその関数を呼び出した場合は引数等に関係なく，何もせずに``RTC_U_UNSUPPORTED``を返します．どのRTCにどの関数が存在しないかは各RTC用ドライバのREADME.mdを参照してください．

### 2.1. 全般

RTCのチップの種類や機能の情報を取得するメンバ関数．
```
void  getRtcInfo(rtc_info_t *info)
```

RTCの初期化関数で，引数はRTCの種類によって異なるため，具体的な内容は各ドライバを参照．
```
bool begin(bool init=true)
```
引数にtrueを指定(デフォルト)の場合は時刻(初期値)を設定します．Arduinoが再起動した場合に以前設定しておいた時刻等が消えると問題がある場合は引数に``false``を指定してください．

| 返り値 | 意味 |
|---|---|
|true|初期化成功|
|false|初期化失敗|


### 2.2. 時刻設定
引数で与えられた時刻をRTCに設定する．なお，本ライブラリ(と各RTCのドライバ)は，12時間制(AM/PM)はサポートしていません．
```
bool        setTime(date_t* time)
```
| 返り値 | 意味 |
|---|---|
|true|設定成功|
|false|設定失敗|

### 2.3. 時刻取得
引数で与えた構造体に，RTCから取得した時刻情報を書き込む．
```
bool        getTime(date_t* time)
```
| 返り値 | 意味 |
|---|---|
|true|取得成功|
|false|取得失敗|

### 2.4. アラーム関係

numで指定した番号のアラームを発行する時刻(timing)とアラームの動作モード(mode)を設定．modeの値や意味はRTCによって異なるため，詳細は各ドライバを参照．
```
int         setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing)
```
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないアラーム番号等を指定している|


numで指定した番号のアラームの動作モード(mode)を設定．modeの値や意味はRTCによって異なるため，詳細は各ドライバを参照．
```
int         setAlarmMode(uint8_t num, alarm_mode_t * mode)
```
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないアラーム番号等を指定している|

numで指定した番号のアラームを開始/停止/一時停止する関数．actionの意味や具体的な動作はRTCの種類によって異なるため，各ドライバを参照．
```
int         controlAlarm(uint8_t num, uint8_t action)
```
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないアラーム番号等を指定している|

### 2.5. タイマ関係
numで指定した番号のタイマの動作をmodeとmultiで設定．一般的には，modeで用いる周波数とmultiで何周期で発火するかを意味するが，RTCの書類によって異なるため，詳細は各ドライバを参照．
```
int         setTimer(uint8_t num, timer_mode_t * mode, uint16_t multi)
```
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないアラーム番号の指定など|

numで指定した番号のタイマの動作modeを設定．モードの内容等はRTCの種類によって異なるため，各ドライバを参照．
```
int         setTimerMode(uint8_t num, timer_mode_t * mode)
```
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないアラーム番号の指定など|

numで指定した番号のタイマを開始/停止/一時停止する関数．actionの意味や具体的な動作はRTCの種類によって異なるため，各ドライバを参照．
```
int         controlTimer(uint8_t num, uint8_t action)
```
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないアラーム番号の指定など|

### 2.6. クロック出力関係
第3引数のピンに第2引数freqで指定した周波数の信号を出す．第1引数のnumは
```
int         setClockOut(uint8_t num, uint8_t freq, int8_t pin)
```
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないクロックの指定など|

第1引数で指定したクロック出力設定の周波数を変更する関数．
```
int         setClockOutMode(uint8_t num, uint8_t freq)
```
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないクロックの指定など|

第1引数で指定したクロック出力を制御(開始/停止)する関数．
```
int         controlClockOut(uint8_t num, uint8_t mode)
```
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないクロックの指定など|

### 2.7. 割り込み関係
割り込みが発生しているか否かの検査で返り値が割り込み番号．返り値の内容はRTCの種類によって異なるため，各ドライバを参照．
```
int    checkInterupt(void)
```

typeで指定したアラームやタイマの割り込みフラグを消す．typeの内容はRTCの種類によって異なるため，各ドライバを参照．
```
int        clearInterupt(uint16_t type)
```

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないクロックの指定など|


### 2.8 電源関係
コンピュータ本体の電源が切断されても時刻を更新するための電池を接続する回路構成にすることが珍しくない．
ただし，電池寿命等の関係で電池電圧が降下した場合にRTCがそれを記録し，次回コンピュータの電源が復帰した場合にRTCの
電源電圧降下が発生したか否かを確認することができることが多い．

この機能を装備したRTCでは電圧降下を示すRTC内のフラグ値の取得とそのフラグをクリアするため，以下の2種類の関数があります．

```
int   checkLowPower(void)

```
この関数は，電圧降下があったか否かを示すフラグ値を取得できます．RTCにその機能がない場合は``RTC_U_UNSUPPORTED``，取得に失敗した場合は``RTC_U_FAILURE``を返します．両方の値共に負の値なので，0以上の値が取得できた場合はフラグ値となります．このフラグ値の具体的な内容はRTCの種類によって異なるため，各RTC用ドライバのREADME.mdを参照してください．


```
int   clearPowerFlag(void)

```
この関数は電圧降下を監視するフラグ値をクリアするためのものです．
RTCに電圧降下監視機能がない場合は``RTC_U_UNSUPPORTED``，フラグ値の書き込みに失敗した場合は``RTC_U_FAILURE``，書き込み成功時は``RTC_U_SUCCESS``を返します．

```
int setLowPower(uint8_t mode)
```
一部のRTCでは，電源低下の検出に関してパラメータを設定することができます．設定に成功すると``RTC_U_SUCCESS``，失敗した場合は``RTC_U_FAILURE``を返します．そもそもRTCに機能がない場合は``RTC_U_UNSUPPORTED``となります．


### 2.9 計時の停止関係
一部のRTCでは，消費電力低減のために時計の更新(クロック)を止める機能を持つものがあります．それに対応する以下のメンバ関数を用意しました．

```
int         controlClockHalt(uint8_t mode)
```
クロックを止める機能があるRTCで止める，再開するを制御します．

|modeの値|意味|
|---|---|
|0| クロックを止める|
|1| クロックを再開する|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |``mode``の値が不正|

```
int         clockHaltStatus(void);
```
この関数はクロックが止まっているか否かを示します．
| 返り値 | 意味 |
|---|---|
|1|クロックは停止|
|0|クロックは停止していない|
|RTC_U_FAILURE |情報取得失敗|

### 2.10 時刻調整
一部のRTCでは，計時を止める以外の調整機能を持つものがあります．例えば，EPSON RX8900は秒以下のカウンタをクリア(リセット)することができます．このような機能を以下の関数にまとめてあります．RTCによって，その機能の有無や処理内容に違いがあるため，各RTCのREADME.mdを参照してください．
```
int controlClock(void)
```
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|


### 2.11 ユーティリティ関数
UNIX時間(1970年元日からの秒数)を第2引数に与えると，第一引数の構造体に西暦の年月日，時刻を返す関数．
```
void                convertEpochTime(date_t * dateTime , unsigned long epochTime)
```

引数で時刻を与えると，UNIX時間を返り値として返す．引数で1970/1/1以前の時刻を指定した場合は0が返る．
```
unsigned long       convertDateToEpoch(date_t dateTime)
```

引数で曜日に相当する整数を与えると，Sun, Monといった曜日の文字列を返す．引数が0の時に，返り値は"Sun"，1で"Mon"となる．
```
String              getWday(uint8_t day)
```

[AdafruitUSD]:https://github.com/adafruit/Adafruit_Sensor
[RTC8564NB]:https://www5.epsondevice.com/ja/products/rtc/rtc8564nb.html
[AkizukiRTC8564NB]:http://akizukidenshi.com/catalog/g/gI-00233/
[DS1307]:https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS1307.html
[GroveRTC]:https://www.seeedstudio.com/Grove-RTC.html
[DS3234]:https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS3234.html
[BOB-10160]:https://www.sparkfun.com/products/10160



<!--- コメント

## 動作検証

|CPU| 機種 |ベンダ| 結果 | 備考 |
| :--- | :--- | :--- | :---: | :--- |
|AVR| [Uno R3][Uno]  |[Arduino][Arduino]|  ○    |      |
|       | [Mega2560 R3][Mega] |[Arduino][Arduino] |  ○    |      |
|       | [Leonardo Ethernet][LeonardoEth] |[Arduino][Arduino] | ○     |      |
|       | [Uno WiFi][UnoWiFi] |[Arduino][Arduino] | ○     | |
|       | [Pro mini 3.3V][ProMini] | [Sparkfun][Sparkfun] |   ×   |      |
| ARM/M0+ | [M0 Pro][M0Pro] |[Arduino][Arduino] |○||
|ESP8266|[ESPr developer][ESPrDev]| [スイッチサイエンス][SwitchScience] |||
|ESP32 | [ESPr one 32][ESPrOne32] | [スイッチサイエンス][SwitchScience] |×|　|




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
