# DS3234
このライブラリは，RTCを取り扱う統一的なAPIを作る思いつきの一部で
[SparkFun DeadOn RTC Breakout - DS3234][BOB-10160]にも搭載されている[DS3234][DS3234]用の
ライブラリ(デバイスドライバ)です．

ベースにしたライブラリは，[SparkFun DeadOn RTC Breakout - DS3234][BOB-10160]用の
ライブラリで，DS3234のモジュール発売元のSparkFunが[公開][github]したものです．

## ライセンスについて
ベースにしたソースのライセンスがMITとなっているので，それを継承します．また下に原作の
ライセンス文を引用しておきますが，詳細については[原本][github]を確認してください．

### 原作のライセンス文
SparkFun License Information
============================

SparkFun uses two different licenses for our files - one for hardware and one for code.

Hardware
---------

**SparkFun hardware is released under [Creative Commons Share-alike 4.0 International](http://creativecommons.org/licenses/by-sa/4.0/).**

Note: This is a human-readable summary of (and not a substitute for) the [license](http://creativecommons.org/licenses/by-sa/4.0/legalcode).

You are free to:

Share � copy and redistribute the material in any medium or format
Adapt � remix, transform, and build upon the material
for any purpose, even commercially.
The licensor cannot revoke these freedoms as long as you follow the license terms.
Under the following terms:

Attribution � You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
ShareAlike � If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original.
No additional restrictions � You may not apply legal terms or technological measures that legally restrict others from doing anything the license permits.
Notices:

You do not have to comply with the license for elements of the material in the public domain or where your use is permitted by an applicable exception or limitation.
No warranties are given. The license may not give you all of the permissions necessary for your intended use. For example, other rights such as publicity, privacy, or moral rights may limit how you use the material.


Code
--------

**SparkFun code, firmware, and software is released under the [MIT License](http://opensource.org/licenses/MIT).**

The MIT License (MIT)

Copyright (c) 2016 SparkFun Electronics, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.



## 動作検証

今回作成したドライバはAdafruitのコードを叩いているだけ(APIのみ)なのですが，
Uno系統しか動きませんでした．レジスタを叩くところがおかしいのですが，
ロジックアナライザを持っていないので対応できません(残念)．

|CPU| 機種 |ベンダ| 結果 | 備考 |
| :--- | :--- | :--- | :---: | :--- |
|AVR| [Uno R3][Uno]  |[Arduino][Arduino]|   ○   |      |
|       | [Mega2560 R3][Mega] |[Arduino][Arduino] |     ×  |      |
|       | [Leonardo Ethernet][LeonardoEth] |[Arduino][Arduino] |  ×   |      |
|       | [Uno WiFi][UnoWiFi] |[Arduino][Arduino] |  ○   | |
|       | [Pro mini 3.3V][ProMini] | [Sparkfun][Sparkfun] |   ×   |      |
| ARM/M0+ | [M0 Pro][M0Pro] |[Arduino][Arduino] |×||
|ESP8266|[ESPr developer][ESPrDev]| [スイッチサイエンス][SwitchScience] |×||
|ESP32 | [ESPr one 32][ESPrOne32] | [スイッチサイエンス][SwitchScience] |× |　|


## 外部リンク

- DS3234 - [https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS3234.html][DS3234]
- SparkFun DeadOn RTC Breakout DS3234 - [https://www.sparkfun.com/products/10160][BOB-10160]
- SparkFun DeadOn RTC Breakout (DS3234) Arduino Library - [https://github.com/sparkfun/SparkFun_DS3234_RTC_Arduino_Library][github]



# API

## オブジェクト生成
```
RTC_DS3234_U(uint8_t cs, SPIClass *spi, int32_t rtcID=-1)
```
RTCが用いるI2CのI/FとIDを指定してオブジェクトを生成．
|引数|内容|
|---|---|
|cs|チップセレクトのピン番号|
|spi|SPI I/F|
|rtcID|rtcに番号をつける場合に利用．(デフォルト値は-1)|

```
RTC_DS3234_U(uint8_t cs, uint8_t miso, uint8_t mosi, uint8_t sck, int32_t rtcID=-1)
```
RTCが用いるI2CのI/FとIDを指定してオブジェクトを生成．
|引数|内容|
|---|---|
|cs|チップセレクトのピン番号|
|mosi, miso, sck| SPI I/Fの各ピン番号|
|rtcID|rtcに番号をつける場合に利用．(デフォルト値は-1)|

## 初期化
```
bool  begin(void)
```
RTCの初期化．

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
設定可能なアラームは，下の表に記載．

| num   | timing->mday | timing->wday | アラーム設定内容 | 設定対象 |
|---|---|---|---|---|
|  0    |  0-31        |              | 日,時,分,秒 | アラーム1番 |
|       | 32以上       | 0-6           | 曜日,時,分,秒 ||
|       |              | 7以上         | 時,分,秒 ||
| 0以外 |  0-31        |               | 日,時,分 | アラーム2番 |
|       |  32以上      | 0-6           | 曜日,時,分 ||
|       |              | 7以上         | 時,分 ||

modeの構造体メンバuseInteruptPinが1の場合に割り込み信号が利用される設定となる．

| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|1 (RTC_U_FAILURE) |設定失敗|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないパラメータの設定など|


## アラームモード設定
```
int   setAlarmMode(uint8_t num, alarm_mode_t * mode)
```

``num``で指定したアラームに``mode``の構造体メンバ``useInteruptPin``で割り込みピンの使用/不使用を設定する．
``useInteruptPin``が0の場合は割り込みピンを制御せず，1の場合は制御する．
|num|設定対象|
|---|---|
|0 | アラーム1番|
|1 | アラーム2番|

| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないパラメータの設定など|


## アラーム制御
```
int   controlAlarm(uint8_t num, uint8_t action)
```

``num``で指定したアラームに``action``で割り込みピンの使用/不使用を設定する．
``action``が0の場合は割り込みピンを制御せず，1の場合は制御する．
|num|設定対象|
|---|---|
|0 | アラーム1番|
|1 | アラーム2番|

| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
|-1 (RTC_U_UNSUPPORTED) |サポートしていないパラメータの設定など|


## 割り込みの確認
```
uint16_t   checkInterupt(void)
```
このRTCはタイマ機能がないため，割り込みはアラームだけで発生する．

| 返り値 | アラーム1番 | アラーム2番 |
|---|---|---|
|0| 割り込みなし | 割り込みなし | 
|1| 割り込み発生 | 割り込みなし | 
|2| 割り込みなし | 割り込み発生 |
|3| 割り込み発生 | 割り込み発生 |

## 割り込みフラグの解除
```
bool  clearInterupt(uint16_t type)
```

| type | アラーム1番 | アラーム2番 |
|---|---|---|
|0| フラグクリア | フラグクリア | 
|1| フラグクリア | なにもしない | 
|2| なにもしない | フラグクリア |
|3| フラグクリア | フラグクリア |
|その他| なにもしない |なにもしない |

typeで4以上を指定するとなにもせず，結果はtrueで返ることに注意が必要．

## クロック出力設定
```
int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1)
```
クロックが出力されているピン番号の設定と，周波数を設定．
|``freq``の値|クロック周波数|
|---|---|
|0|1Hz|
|1|1kHz|
|2|4kHz|
|3|8kHz|

| 返り値 | 意味 |
|---|---|
|0 (RTC_U_SUCCESS) |設定成功|
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
|-1 (RTC_U_UNSUPPORTED) |サポートしていないパラメータの設定など|




## 未サポート機能
RTCに機能がないため，以下の関数は未サポート．返り値は常に「-1 (RTC_U_UNSUPPORTED)」となる．
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


[DS3234]:https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS3234.html
[BOB-10160]:https://www.sparkfun.com/products/10160
[github]:https://github.com/sparkfun/SparkFun_DS3234_RTC_Arduino_Library
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
|AVR| [Uno R3][Uno]  |[Arduino][Arduino]|   ○   |      |
|       | [Mega2560 R3][Mega] |[Arduino][Arduino] |     ×  |      |
|       | [Leonardo Ethernet][LeonardoEth] |[Arduino][Arduino] |  ×   |      |
|       | [Uno WiFi][UnoWiFi] |[Arduino][Arduino] |  ○   | |
|       | [Pro mini 3.3V][ProMini] | [Sparkfun][Sparkfun] |   ×   |      |
| ARM/M0+ | [M0 Pro][M0Pro] |[Arduino][Arduino] |×||
|ESP8266|[ESPr developer][ESPrDev]| [スイッチサイエンス][SwitchScience] |×||
|ESP32 | [ESPr one 32][ESPrOne32] | [スイッチサイエンス][SwitchScience] |× |　|

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
