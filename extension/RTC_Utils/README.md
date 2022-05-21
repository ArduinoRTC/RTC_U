# RTC_Utils

RTC_Uライブラリは，driver配下の各RTCチップのライブラリを個別にインストールした上で，チップに対応したクラスオブジェクトを生成した上で各APIを呼び出す型となるため，複数のRTCチップを使う可能性があるスケッチを作る場合は，RTCチップによる#ifdefが作成されてしまい，コードが読みにくくなるため，#ifdefの数を削減するためにこのライブラリを作成しました．

利用するRTCがI2Cの場合は特に使い勝手が良くなります．

## インストール
このディレクトリごとArduinoのlibrariesフォルダにコピーしてください．

## 使い方
スケッチのはじめの方に以下のコードをコピーして，#define文のうちの一つ(実際に利用するRTC)を生かす．
```
//#define USE_RTC_4543
//#define USE_RTC8564NB
//#define USE_DS1307
//#define USE_DS3231
//#define USE_DS3234
//#define USE_PCF8523
//#define USE_RV8803
//#define USE_RX8025
//#define USE_RX8900

#include "RTC_Utils.h"
RTC_CLASS rtc = RTC_CLASS(&Wire);
```
以上で利用するRTCのオブジェクトがrtcという名前で利用できます．

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
