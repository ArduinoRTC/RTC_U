# DS1307

このライブラリは，RTCを取り扱う統一的なAPIを作る思いつきの一部で
[Grove RTC][GroveRTC]にも搭載されている[DS1307][DS1307]用の
ライブラリ(デバイスドライバ)です．

ベースにしたライブラリは，Grove RTC用の[https://github.com/Seeed-Studio/RTC_DS1307][github]です．


## ライセンスについて
ベースにしたプログラムのライセンスは，MITとなっているので，それを継承します．なお，原作のライセンス文を
以下に引用しておきます．

### 原作のライセンス文
This software is written by Frankie Chu for seeed studio
and is licensed under The MIT License. Check License.txt for more information.

Contributing to this software is warmly welcomed. You can do this basically by
forking, committing modifications and then pulling requests (follow the links above
for operating guide). Adding change log and your contact into file header is encouraged.
Thanks for your contribution.

Seeed is a hardware innovation platform for makers to grow inspirations into differentiating products. By working closely with technology providers of all scale, Seeed provides accessible technologies with quality, speed and supply chain knowledge. When prototypes are ready to iterate, Seeed helps productize 1 to 1,000 pcs using in-house engineering, supply chain management and agile manufacture forces. Seeed also team up with incubators, Chinese tech ecosystem, investors and distribution channels to portal Maker startups beyond.


## 動作検証
動作検証に利用した[Grove RTC][GroveRTC]が5V専用であるため，VDDが5Vの
機種のみ検証しました．

|CPU| 機種 |ベンダ| 結果 | 備考 |
| :--- | :--- | :--- | :---: | :--- |
|AVR| [Uno R3][Uno]  |[Arduino][Arduino]|   ○   |      |
|       | [Mega2560 R3][Mega] |[Arduino][Arduino] |  ○    |      |
|       | [Leonardo Ethernet][LeonardoEth] |[Arduino][Arduino] |   ○  |      |
|       | [Uno WiFi][UnoWiFi] |[Arduino][Arduino] |    ○  | |
|       | [Pro mini 3.3V][ProMini] | [Sparkfun][Sparkfun] |      |      |
| ARM/M0+ | [M0 Pro][M0Pro] |[Arduino][Arduino] |||
|ESP8266|[ESPr developer][ESPrDev]| [スイッチサイエンス][SwitchScience] |||
|ESP32 | [ESPr one 32][ESPrOne32] | [スイッチサイエンス][SwitchScience] ||　|



## 外部リンク

- DS1307 - [https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS1307.html][DS1307]
- Grove RTC - [https://www.seeedstudio.com/Grove-RTC.html][GroveRTC]
- Seeed studio RTC_DS1307 library [https://github.com/Seeed-Studio/RTC_DS1307][github]


# API

## オブジェクト生成
```
RTC_DS1307_U(TwoWire *theWire, int32_t rtcID = -1)
```
RTCが用いるI2CのI/FとIDを指定してオブジェクトを生成．
|引数|内容|
|---|---|
|theWire|I2CのI/F|
|rtCID|rtcに番号をつける場合に利用．(デフォルト値は-1)|

## 初期化
```
bool  begin(uint32_t addr=RTC_DS1307_DEFAULT_I2C_ADDR)
```
DS1307のデフォルトI2Cのアドレスではない番号を持つモジュールを
用いる場合は，引数で指定．指定しない場合は，デフォルトのアドレスで初期化を実施．

| 返り値 | 意味 |
|---|---|
|true|初期化成功|
|false|初期化失敗|

## RTCの情報の取得
RTCのチップの種類や機能の情報を取得するメンバ関数．
```
void  getRtcInfo(rtc_info_t *info)
```

## 時刻設定
```
bool  setTime(rtc_date_t* time)
```
引数で与えた時刻をRTCに設定．
| 返り値 | 意味 |
|---|---|
|true|設定成功|
|false|設定失敗|

## 時刻取得
```
bool  getTime(rtc_date_t* time)
```
RTCから取得した時刻情報を引数で与えた構造体に格納．
| 返り値 | 意味 |
|---|---|
|true|取得成功|
|false|取得失敗|

## クロック出力設定
```
int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1)
```
クロックが出力されているピン番号の設定と，周波数を設定．

|``freq``の値|クロック周波数|
|---|---|
|0|1Hz|
|1|4kHz|
|2|8kHz|
|3|32kHz|

| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないパラメータの設定など|

## クロック周波数設定
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
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないパラメータの設定など|


## クロック出力の制御
```
int   controlClockOut(uint8_t num, uint8_t mode)
```
|``mode``の値|意味|
|---|---|
|0|クロック出力停止|
|1|クロック出力開始|

| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないパラメータの設定など|


## 利用を進めない機能(廃止予定)

### クロック出力の開始
```
void  startClock(void)
```

### クロック出力の停止
```
void  stopClock(void)
```


## DS1307にない機能
以下の関数はRTCに対応する機能がないので，-1(RTC_U_UNSUPPORTED)を返す．
### アラーム設定
```
int   setAlarm(uint8_t num, alarm_mode_t * mode, rtc_date_t* timing)
```
### アラームモード設定
```
int   setAlarmMode(uint8_t num, alarm_mode_t * mode)
```
### アラーム制御
```
int   controlAlarm(uint8_t num, uint8_t action)
```
### タイマ設定
```
int   setTimer(uint8_t num, timer_mode_t * mode, uint8_t multi)
```
### タイマのモード設定
```
int   setTimerMode(uint8_t num, timer_mode_t * mode)
```
### タイマの制御
```
int   controlTimer(uint8_t num, uint8_t action)
```
### 割り込みの確認
```
uint16_t   checkInterupt(void)
```
### 割り込みフラグの解除
```
bool  clearInterupt(uint16_t type)
```



[DS1307]:https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS1307.html
[GroveRTC]:https://www.seeedstudio.com/Grove-RTC.html
[github]:https://github.com/Seeed-Studio/RTC_DS1307
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
5V専用

|CPU| 機種 |ベンダ| 結果 | 備考 |
| :--- | :--- | :--- | :---: | :--- |
|AVR| [Uno R3][Uno]  |[Arduino][Arduino]|   ○   |      |
|       | [Mega2560 R3][Mega] |[Arduino][Arduino] |  ○    |      |
|       | [Leonardo Ethernet][LeonardoEth] |[Arduino][Arduino] |   ○  |      |
|       | [Uno WiFi][UnoWiFi] |[Arduino][Arduino] |    ○  | |
|       | [Pro mini 3.3V][ProMini] | [Sparkfun][Sparkfun] |      |      |
| ARM/M0+ | [M0 Pro][M0Pro] |[Arduino][Arduino] |||
|ESP8266|[ESPr developer][ESPrDev]| [スイッチサイエンス][SwitchScience] |||
|ESP32 | [ESPr one 32][ESPrOne32] | [スイッチサイエンス][SwitchScience] ||　|

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
