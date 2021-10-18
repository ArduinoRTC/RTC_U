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
### 1.1. 時刻用構造体
```
typedef struct  {
    uint16_t    year;           // 年
    uint8_t     month;          // 月
    uint8_t     mday;           // 日
    uint8_t     wday;           // 曜日 (日曜が0で土曜が6)
    uint8_t     hour;           // 時
    uint8_t     minute;         // 分
    uint8_t     second;         // 秒
    int16_t     millisecond;    // ミリ秒 (ミリ秒の機能があるRTCのみで利用)
} rtc_date_t;
```


### 1.2. RTCの機能を格納する構造体
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

### 1.3. タイマの動作モード
```
typedef struct {
    int8_t  repeat;
    int8_t  useInteruptPin;
    int8_t  interval;
} timer_mode_t;
```

### 1.4. アラームの動作モード
```
typedef struct {
    int8_t  useInteruptPin;
} alarm_mode_t;
```

### 1.5. RTCの種類
```
enum {
    EPSON8564NB  = 1,
    DS1307,
    DS3234
};
```

### 1.6. 曜日
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


## 2. メンバ関数

### 2.1. 全般

RTCのチップの種類や機能の情報を取得するメンバ関数．
```
void  getRtcInfo(rtc_info_t *info)
```


RTCの初期化関数で，引数はRTCの種類によって異なるため，具体的な内容は各ドライバを参照．
```
bool begin()
```
| 返り値 | 意味 |
|---|---|
|true|初期化成功|
|false|初期化失敗|


### 2.2. 時刻設定
引数で与えられた時刻をRTCに設定する．
```
bool        setTime(rtc_date_t* time)
```
| 返り値 | 意味 |
|---|---|
|true|設定成功|
|false|設定失敗|

### 2.3. 時刻取得
引数で与えた構造体に，RTCから取得した時刻情報を書き込む．
```
bool        getTime(rtc_date_t* time)
```
| 返り値 | 意味 |
|---|---|
|true|取得成功|
|false|取得失敗|

### 2.4. アラーム関係

numで指定した番号のアラームを発行する時刻(timing)とアラームの動作モード(mode)を設定．modeの値や意味はRTCによって異なるため，詳細は各ドライバを参照．
```
int         setAlarm(uint8_t num, alarm_mode_t * mode, rtc_date_t* timing)
```
| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないアラーム番号等を指定している|


numで指定した番号のアラームの動作モード(mode)を設定．modeの値や意味はRTCによって異なるため，詳細は各ドライバを参照．
```
int         setAlarmMode(uint8_t num, alarm_mode_t * mode)
```
| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないアラーム番号等を指定している|

numで指定した番号のアラームを開始/停止/一時停止する関数．actionの意味や具体的な動作はRTCの種類によって異なるため，各ドライバを参照．
```
int         controlAlarm(uint8_t num, uint8_t action)
```
| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないアラーム番号等を指定している|

### 2.5. タイマ関係
numで指定した番号のタイマの動作をmodeとmultiで設定．一般的には，modeで用いる周波数とmultiで何周期で発火するかを意味するが，RTCの書類によって異なるため，詳細は各ドライバを参照．
```
int         setTimer(uint8_t num, timer_mode_t * mode, uint8_t multi)
```
| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないアラーム番号の指定など|

numで指定した番号のタイマの動作modeを設定．モードの内容等はRTCの種類によって異なるため，各ドライバを参照．
```
int         setTimerMode(uint8_t num, timer_mode_t * mode)
```
| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないアラーム番号の指定など|

numで指定した番号のタイマを開始/停止/一時停止する関数．actionの意味や具体的な動作はRTCの種類によって異なるため，各ドライバを参照．
```
int         controlTimer(uint8_t num, uint8_t action)
```
| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないアラーム番号の指定など|

### 2.6. クロック出力関係
第3引数のピンに第2引数freqで指定した周波数の信号を出す．第1引数のnumは
```
int         setClockOut(uint8_t num, uint8_t freq, int8_t pin)
```
| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないクロックの指定など|

第1引数で指定したクロック出力設定の周波数を変更する関数．
```
int         setClockOutMode(uint8_t num, uint8_t freq)
```
| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないクロックの指定など|

第1引数で指定したクロック出力を制御(開始/停止)する関数．
```
int         controlClockOut(uint8_t num, uint8_t mode)
```
| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないクロックの指定など|

### 2.7. 割り込み関係
割り込みが発生しているか否かの検査で返り値が割り込み番号．返り値の内容はRTCの種類によって異なるため，各ドライバを参照．
```
uint16_t    checkInterupt(void)
```

typeで指定したアラームやタイマの割り込みフラグを消す．typeの内容はRTCの種類によって異なるため，各ドライバを参照．
```
bool        clearInterupt(uint16_t type)
```
| 返り値 | 意味 |
|---|---|
|true|設定成功|
|false|設定失敗|

### 2.8. ユーティリティ関数
UNIX時間(1970年元日からの秒数)を第2引数に与えると，第一引数の構造体に西暦の年月日，時刻を返す関数．
```
void                convertEpochTime(rtc_date_t * dateTime , unsigned long epochTime)
```

引数で時刻を与えると，UNIX時間を返り値として返す．引数で1970/1/1以前の時刻を指定した場合は0が返る．
```
unsigned long       convertDateToEpoch(rtc_date_t dateTime)
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
