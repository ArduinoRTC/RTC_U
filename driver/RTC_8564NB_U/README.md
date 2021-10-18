# EPSON 8564NB

このライブラリは，RTCを取り扱う統一的なAPIを作る思いつきの一部で
私も以前良くつかっていたエプソンの[8564NB][RTC8564NB]用の
ドライバです．

## ライセンスについて
[元のソース][original]はライセンスが不明です．
気になる方は原作者まで問い合わせてください．
(わかったら教えてもらえるとうれしいです．)

## 動作検証

|CPU| 機種 |ベンダ| 結果 | 備考 |
| :--- | :--- | :--- | :---: | :--- |
|AVR| [Uno R3][Uno]  |[Arduino][Arduino]|    ○  |      |
|       | [Mega2560 R3][Mega] |[Arduino][Arduino] |  ○    |      |
|       | [Leonardo Ethernet][LeonardoEth] |[Arduino][Arduino] |  △   |   割り込みピンが使えない? (D3,D4はだめだった)   |
|       | [Uno WiFi][UnoWiFi] |[Arduino][Arduino] |    ○  | |
|       | [Pro mini 3.3V][ProMini] | [Sparkfun][Sparkfun] | ○     |      |
| ARM/M0+ | [M0 Pro][M0Pro] |[Arduino][Arduino] |○||
|ESP8266|[ESPr developer][ESPrDev]| [スイッチサイエンス][SwitchScience] |○|割り込みピンはD14を利用|
|ESP32 | [ESPr one 32][ESPrOne32] | [スイッチサイエンス][SwitchScience] |○|割り込みピンはD13を利用|


## サンプルプログラム

- RTC_8564NB_U
  - 8564NBの内部タイマでの割り込み機能を用いて，コールバック関数を呼び出すサンプルプログラム
- RTC_8564NB_U_settime
  - NTPもしくは，プログラム上の定数(固定値)でRTCに時刻を設定するサンプルプログラム

## 外部リンク
- RTC-8564NB - [https://www5.epsondevice.com/ja/products/rtc/rtc8564nb.html][RTC8564NB]
- 秋月電子8564NB搭載モジュール - [http://akizukidenshi.com/catalog/g/gI-00233/][AkizukiRTC8564NB]
- オリジナルプログラム - [http://zattouka.net/GarageHouse/micon/Arduino/RTC/RTC.htm][original]


# API

## オブジェクト生成
```
RTC_8564NB_U(TwoWire * theWire, int32_t rtcID=-1)
```
RTCが用いるI2CのI/FとIDを指定してオブジェクトを生成．
|引数|内容|
|---|---|
|theWire|I2CのI/F|
|rtcID|rtcに番号をつける場合に利用．(デフォルト値は-1)|

## 初期化
```
bool  begin(uint32_t addr=RTC_EPSON_8564NB_DEFAULT_ADRS)
```
EPSON　8564NBのデフォルトI2Cのアドレスではない番号を持つモジュールを
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

## アラーム設定
```
int   setAlarm(uint8_t num, alarm_mode_t * mode, rtc_date_t* timing)
```
8564NBのアラームは，日付，曜日，時間，分を設定でき，それを第3引数で指定．モードは``setAlarmMode()``を参照．

| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないパラメータの設定など|

## アラームモード設定
```
int   setAlarmMode(uint8_t num, alarm_mode_t * mode)
```
アラームが発生したタイミングでの割り込みピンの制御を行うか否かを設定する．
``mode``の構造体メンバ``useInteruptPin``が0の場合は割り込みピンを制御せず，1の場合は制御する．

| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないパラメータの設定など|

## アラーム制御
```
int   controlAlarm(uint8_t num, uint8_t action)
```
``action``を0に設定すると，アラームが停止する．それ以外の動作はサポートしていない．そのため，止めたアラームを再度有効にするためには，再度のセットアップが必要．

| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないパラメータの設定など|

## タイマ設定
```
int   setTimer(uint8_t num, timer_mode_t * mode, uint8_t multi)
```
第2引数modeの構造体メンバの意味は以下の通り．
|構造体メンバ|0|1|2|3|
|---|---|---|---|---|
|``repeat``|リピートなし|リピートあり|||
|``useInteruptPin``|割り込みピンの利用|割り込みピン不使用|||
|``interval``|4096Hz|64Hz|1秒|1分|

第3引数のmultiは第2引数の``interval``の周期の``multi``倍でタイマが発火する設定を行う．

| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないパラメータの設定など|

## タイマのモード設定
```
int   setTimerMode(uint8_t num, timer_mode_t * mode)
```
第2引数は上の``setTimer()``と同じ．

| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないパラメータの設定など|

## タイマの制御
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
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないパラメータの設定など|


## クロック出力設定
```
int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1)
```
クロックが出力されているピン番号の設定と，周波数を設定．
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


## 割り込みの確認
```
uint16_t   checkInterupt(void)
```
割り込みの有無を16bitの2進数の列として出力．どのbitが立っているかでどの割り込みが発生しているかを判定することができる．


## 割り込みフラグの解除
```
bool  clearInterupt(uint16_t type)
```
``checkInterupt()``の出力と同じく，消すフラグをbit列の中で0として表記して与える．




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
