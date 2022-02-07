# RTC_Utils

RTC_Uライブラリは，driver配下の各RTCチップのライブラリを個別にインストールした上で，チップに対応したクラスオブジェクトを生成した上で各APIを呼び出す型となる．

そのため，複数のRTCチップを使う可能性があるスケッチを作る場合は，RTCチップによる#ifdefが作成されてしまい，コードが読みにくくなるため，#ifdefの数を削減するためのものです．

## インストール
このディレクトリごとArduinoのlibrariesフォルダにコピーしてください．

## 使い方
スケッチのはじめの方に以下のコードをコピーして，3つの#define文のうちの一つ(実際に利用するRTC)を生かす．
```
//#define USE_RTC8564NB
//#define USE_DS1307
//#define USE_DS3234
#include <RTC_Utils.h>
```
以下の1行でRTCチップ用クラスオブジェクトの生成をする．(RTCのチップ種類に関係なく有効)
```
RTC_CLASS rtc
```
あとは，RTC_Uライブラリの各APIが利用可能になります．

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
