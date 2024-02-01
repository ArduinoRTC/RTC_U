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


| CPU | 機種 | 対応状況 |
|---|---|---|
| AVR | Arduino Mega | ○ |
| SAMD | Arduino MKR WiFi1010 | ○ |
| SAM | Arduino Due | ○ |
| ESP32 | スイッチサイエンスESP developer32 | ○ |


## 外部リンク

- DS3234 - [https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS3234.html][DS3234]
- SparkFun DeadOn RTC Breakout DS3234 - [https://www.sparkfun.com/products/10160][BOB-10160]
- SparkFun DeadOn RTC Breakout (DS3234) Arduino Library - [https://github.com/sparkfun/SparkFun_DS3234_RTC_Arduino_Library][github]

## 利用上の注意

``RTC_DS3234_U.h``に以下のようなデバッグや性能テストに用いるための機能を生かすフラグがあります．
必要に応じて有効・無効を変更してください．
```
#define DEBUG
```

## サンプルプログラム
サンプルプログラムは，本ドライバの各機能を実行した場合に，RTCの各レジスタが適切に設定されているか否かを
確認するためのものです．

``RTC_DS3234_U.h``の``DEBUG``定義を有効にした上で
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
以下の各関数の説明を理解するために，RTCの各レジスタのbitが持つ意味を知る必要がありますので，データシートを見ながら読んでください．
## 初期化関係
### オブジェクト生成
```
RTC_DS3234_U(uint8_t cs, SPIClass *spi, int32_t rtcID=-1)
```
RTCが用いるSPIのI/FとIDを指定してオブジェクトを生成．
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

### 初期化
```
bool  begin(bool init=true)
```
RTCの初期化．引数で``false``を与えると，時刻設定等を行わなず，インターフェイスの最小限の初期化のみを行うので，RTCに時刻設定を行った後にArduinoを再起動した場合に便利です．

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

## アラーム関係
DS3234はアラームが2種類あるので，以下の各関数の第1引数``num``は0もしくは1となります．

| num   | timing->mday | timing->wday | アラーム設定内容 | 設定対象 |
|---|---|---|---|---|
|  0    |  0-31        |              | 日,時,分,秒 | アラーム1番 |
|       | 32以上       | 0-6           | 曜日,時,分,秒 ||
|       |              | 7以上         | 時,分,秒 ||
| 1     |  0-31        |               | 日,時,分 | アラーム2番 |
|       |  32以上      | 0-6           | 曜日,時,分 ||
|       |              | 7以上         | 時,分 ||


### アラーム設定
```
int   setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing)
```



modeの構造体メンバuseInteruptPinが1の場合に割り込み信号が利用される設定となる．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

DS3234は設定した秒だけを使う，分と秒だけを使うなど複雑な設定が可能です．
本ドライバでは，引数``timing``の各メンバをアラームのレジスタに直接代入しているため，
どのような値を指定するとどのような意味となるかはデータシートのアラーム機能の説明を参照してください．

### アラームモード設定
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
|RTC_U_SUCCESS |設定成功|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


### アラーム制御
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
|RTC_U_SUCCESS |設定成功|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## 割り込み関連
### 割り込みの確認
```
int   checkInterupt(void)
```
このRTCはタイマ機能がないため，割り込みはアラームだけで発生します．返り値は以下の表の通りです．

| 返り値 | アラーム1番 | アラーム2番 |
|---|---|---|
|0| 割り込みなし | 割り込みなし | 
|1| 割り込み発生 | 割り込みなし | 
|2| 割り込みなし | 割り込み発生 |
|3| 割り込み発生 | 割り込み発生 |

### 割り込みフラグの解除
```
int  clearInterupt(uint16_t type)
```

| type | アラーム1番 | アラーム2番 |
|---|---|---|
|0| フラグクリア | フラグクリア | 
|1| フラグクリア | なにもしない | 
|2| なにもしない | フラグクリア |
|3| フラグクリア | フラグクリア |
|その他| なにもしない |なにもしない |

typeで4以上を指定するとなにもせず，結果はtrueで返ることに注意が必要．

## クロック信号出力関連
### クロック出力設定
DS3234は，内蔵するクロックの信号(32kHz)をそのまま出力する端子(32kHz端子)と，周波数を選択して出力する端子(INT/SQW端子)を持ちます．
INT/SQW端子はアラームの割り込み端子と共用のため，アラームを利用する場合は周波数信号を出力することはできません．
なお，RTCは両方同時に設定できてしまうため，利用時はご注意ください．

```
int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1)
```
第1引数の``num``が0の場合は，SQW/INT端子に出力する機能の設定となります．また，``pin``を利用する機能はDS3234に存在しないため，``pin``は``-1``にしてください．``num``が1の場合(32kHz端子への出力)は周波数などのパラメータを設定する機能が存在しないため，常に``RTC_U_UNSUPPORTED``を返します．

第2引数のfreqの意味は下の表の通りです．
|``freq``の値|クロック周波数|
|---|---|
|0|1Hz|
|1|1kHz|
|2|4kHz|
|3|8kHz|

``num``が0の場合は以下の返り値となります．
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

### クロック周波数設定
``setClockOut()``と同じ動作となります．実装は``setClockOut()``が本メンバ関数を呼び出しているだけなので当然です．
```
int   setClockOutMode(uint8_t num, uint8_t freq)
```

第2引数のfreqの意味は下の表の通りです．
|``freq``の値|クロック周波数|
|---|---|
|0|4096Hz|
|1|64Hz|
|2|1秒|
|3|1分|

``num``が0の場合は以下の返り値となります．
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


### クロック出力の制御
本メンバ関数はクロック信号の出力のON/OFFを行います．``setClockOut()``と``setClockOutMode()``の2つと異なり，本メンバ関数は``num``が1の場合も処理が行われます．
```
int   controlClockOut(uint8_t num, uint8_t mode)
```
第1引数が0の場合は``setClockOut()``や``setClockOutMode()``で設定した信号をSQW/INT端子に出力する機能をON/OFFします．``num``が1の場合は32kHz端子に32kHzの信号を出力する機能をON/OFFします．

|``mode``の値|意味|
|---|---|
|0|クロック信号出力停止|
|1|クロック信号出力開始|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## 電源電圧低下監視機能
### 電源低下フラグの読み出し
```
int   checkLowPower(void)
```
電源電圧降下(電源断)が発生したか否かをレジスタ値を読み出して判定．

|返り値|意味|
|---|---|
|0|電源断は発生していない|
|1|電源断が発生した|

### 電源低下フラグのクリア
```
int   clearPowerFlag(void)
```
電源電圧降下(電源断)が発生したか否かを示すレジスタ(ステータスレジスタ0x0F)のOSF bit(最上位bit)をクリア．
返り値は常に``RTC_U_SUCCESS``．

## RTC温度関連機能
DS3234は内部の温度に応じて発振周波数が変化するため，時刻の進み方を内部温度に応じて調整する機能があります．なお，DS3234ではこの機能は止めることができません．

### RTC内部温度取得
```
float getTemperature(uint8_t mode)
```

|modeの値|意味|
|---|---|
|RTC_U_TEMPERATURE_KELVIN|絶対零度で温度を返します|
|RTC_U_TEMPERATURE_CELCIUS|摂氏で温度を返します|
|RTC_U_TEMPERATURE_FAHRENHEIT|華氏で温度を返します|

### 温度制御機能の設定
DS3234では，調整する周期と主電源が止まり，バッテリ動作中に温度制御機能を動かし続けるか否かを設定できます．
```
int setTemperatureFunction(uint8_t mode)
```

下の表が引数``mode``の意味となりますが，最下位bitが0の場合はバッテリ動作中は温度制御機能が止まり，1の場合は常時動作します．
また，1bit目と2bit目の値で温度制御処理が実行される周期を設定することができます．
|``mode``のbit|意味|備考|
|---|---|---|
|0|バッテリ動作中の温度制御機能の動作|レジスタ番号``0x13``の最下位bitに相当|
|1|温度制御処理を動作させる周期|制御/ステータスレジスタ(``0x0F``)のCEATE0 bitに相当|
|2|同上|制御/ステータスレジスタ(``0x0F``)のCEATE1 bitに相当|

1bit目と2bit目の組み合わせの意味は以下の表となります．

|CRATE1|CRATE2|周期|
|---|---|---|
| 0 | 0 | 64秒 |
| 0 | 1 | 128秒 |
| 1 | 0 | 256秒 |
| 1 | 1 | 512秒 |

返り値は常時``RTC_U_SUCCESS``となります．

### 温度制御機能の状態取得
```
int getTemperatureFunctionState(void)
```
事前に設定された周期(なにも設定していない場合はデフォルトで64秒周期)で実行される温度制御機能が実行中か否かを取得します．
返り値が``1``の場合は制御処理実行中，``0``の場合は処理が行われていません．ただし，温度制御機能はすぐに終わってしまうため，
``1``が返されるのは非常に稀なタイミングとなります．

### 温度制御機能の強制実行
```
int controlTemperatureFunction(uint8_t action)
```
強制的に温度制御機能を実行することができます．ただし，一定周期で自動的に実行される温度制御機能実行中は実行できないため，エラー(``RTC_U_FAILURE``)が返されます．また，温度制御機能はOFFにすることができないため，引数の``action``は1以外を指定するとエラー``RTC_U_ILLEGAL_PARAM``となります．

実行の設定ができた場合は``RTC_U_SUCCESS``が返されます．

## 温度制御用周波数設定
DS3234の温度制御機能では，内部の時計の進み方を温度で調整しますが，この時の調整幅を設定することができます．

### 調整幅設定値の参照
```
int getOscillator(void)
```
このメンバ関数では，RTCのエージングオフセットレジスタ(0x10番)の値を返します．実際に返される値の中身は8bitの2進数となっています．
返り値の詳細な意味はデータシートを参照してください．

### 調整幅の設定変更
```
int setOscillator(uint8_t mode)
```
RTCのエージングオフセットレジスタ(0x10番)の値を引数``mode``の値で置き換えます．
返り値は常時``RTC_U_SUCCESS``です．


## SRAM領域へのアクセス
DS1307ではレジスタとは別に256バイトのSRAM領域が利用できます．以下の2つの関数の第1引数``addr``は0から255となります．
また，第1引数``addr``と第3引数``len``の合計がレジスタの数より大きい場合はエラーとなります．

### SRAM領域からの読み取り
```
int getSRAM(uint8_t addr, uint8_t *array, uint16_t len)
```
第1引数の``addr``から``len``個のデータを連続して読み取ります．

### SRAM領域への書き込み
```
int setSRAM(uint8_t addr, uint8_t *array, uint16_t len)
```
第1引数の``addr``から``len``個のデータをSRAM領域として利用可能なレジスタに連続して書き込みます．



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
