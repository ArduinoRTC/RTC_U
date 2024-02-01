# EPSON 4543SA/SB

このライブラリは，
エプソンのRTC4543SA/SB用の
ドライバです．


## 動作検証
RTCは秋月電子の
[AkizukiRTC_4543][３２ｋＨｚ出力シリアルＲＴＣ基板モジュール　ＲＴ４５４３使用]を使いました．

動作が確認できた機種は以下の3種類です．

| CPUアーキ | 利用した機種 |
|---|---|
| AVR | Arduino Mega |
| SAMD | Arduino MKR WiFi 1010 |
| SAM | Arduino Due |

### 制限事項
電源喪失を検出するフラグの読み取り``int checkLowPower(void)``に失敗(電源OFFにしたのに，電源喪失していない)という結果を返すことがあり，挙動不審です．これはすべての機種(Mega, Due等)で同じでした．

データアクセスのシーケンスの最初の1bit目なので，タイミングの問題かもしれませんが，解決できていません．

通信関係の問題はロジアナ等もっていないので解決はほぼ無理な状況です．

# API

詳しい動作が知りたい方は本ドキュメントと合わせて，EPSONが公開しているアプリケーションマニュアルを参照してください．

## 初期化関係
### オブジェクト生成
```
RTC_4543_U(uint8_t _dataPin, uint8_t _clkPin, uint8_t _wrPin, uint8_t _cePin, uint8_t _fsel, int32_t _rtcID=-1)
```

4543が利用する各端子に接続しているピン番号とrtcのIDを引数で与えます．
``_fsel``はクロック信号出力で利用するので，そちらを参照してください．

### 初期化
```
bool  begin(bool init, uint32_t addr)
```

第1引数は，時刻やタイマ，アラームの設定を実施するか否かを示すフラグ．

第2引数はI2Cのデバイス等のためのものなので，本RTC用ドライバでは無視されます．

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
bool  setTime(rtc_date_t* time)
```
引数で与えた時刻をRTCに設定．
| 返り値 | 意味 |
|---|---|
|true|設定成功|
|false|設定失敗|

なお，引数のデータ型は``dateUtils.h``で下のように定義されています．ミリ秒単位の時刻は本RTC含め大部分のRTCでは利用できません．

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

### 時刻取得
```
bool  getTime(rtc_date_t* time)
```
RTCから取得した時刻情報を引数で与えた構造体に格納．
| 返り値 | 意味 |
|---|---|
|true|取得成功|
|false|取得失敗|


## 周波数信号出力関係
### クロック出力設定
```
int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1)
```

本RTC(4543SA/SB)では，クロック信号の出力は1種類であり，外部からピンの電圧設定で出力する周波数が決定され，別のピンの電圧で出力する/しないが決まります．
そのため，第1引数は0である必要があります．``pin``にはRTCのFOE端子に接続しているArduinoのピン番号を指定してください．

| freqの値 | 出力される周波数 |
|---|---|
| 0 |32.768kHz|
|その他 | 1Hz|

実際の動作としては，本クラスのオブジェクト生成の時に指定した``_fsel``ピンの電圧を``freq``の値に合わせて``HIGH``もしくは``LOW``に設定することで，RTCのクロック信号の出力周波数を制御しています．そのため，``_fsel``やこの関数の``pin``が正しくRTCに接続されていないと正しく信号が出力されないので，ご注意ください．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_ILLEGAL_PARAM |``num``の値が不正|

### クロック周波数設定
```
int   setClockOutMode(uint8_t num, uint8_t freq)
```
各引数の意味や動作の中身は``setClockOut()``と同じです．

| freqの値 | 出力される周波数 |
|---|---|
| 0 |32.768kHz|
|その他 | 1Hz|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_ILLEGAL_PARAM |``num``の値が不正|

### クロック出力の制御
```
int   controlClockOut(uint8_t num, uint8_t mode)
```
周波数信号を出力する端子は1つしかないため，``num``は0限定です．また，第2引数は以下の表のような意味となります．

|``mode``の値|意味|
|---|---|
|0|クロック出力停止|
|1|クロック出力開始|


実際の動作としては，``setClockOut()``で指定した``pin(foeピン)``の電圧を``HIGH``もしくは``LOW``に設定することで，RTCからの信号出力をON/OFFしています．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


## 電源電圧関係
### 電源フラグの取得
```
int checkLowPower(void)
```
RTCのデータのうち，電源電圧の低下を示す``FDT``の値を取得して返します．

| 返り値 | 意味 |
|---|---|
|0 |電圧低下なし|
|1 |電圧低下あり|

本RTCでは，RTCの全データを読み取ると``FDT``はクリアされるため，複数回読み取ることはできません．

### 電源フラグのクリア
```
int clearPowerFlag(void)
```
電源喪失を示すフラグ(``FDT``)をクリアする関数．常時``RTC_U_SUCCESS``が返されます．

本RTCでは，RTCの全データを読み取ると``FDT``はクリアされるため，``checkLowPower()``を実行していれば
本関数を呼び出す必要はありません．


[AkizukiRTC_4543]:https://akizukidenshi.com/catalog/g/gK-10722/
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
