# RTC_U
このライブラリは，いろいろなセンサのインターフェイスを統一する[プロジェクト][AdafruitUSD]を
参考に，「RTCのAPIも統一できたらプログラミングが楽になるかも」と思い作ってみたAPIの定義と
手元に転がっていたRTCを使ってArduino用のライブラリ(デバイスドライバ)の組み合わせとなって
います．

## ライセンスについて
APIの定義(RTC_U.h)はLGPLです．具体的な各RTCのライブラリは各ディレクトリの中身を見てください．
各ドライバはどなたかが作ったコードのインターフェイス部分を改造しているので，もとのソースのライセンス条件を引き継ぎます．

## 対応済みRTC
|ベンダ| 型番 |
| :--- | :--- |
|エプソン|[RTC 8564NB][RTC8564NB]|
|MAXIM|[DS1307][DS1307]|
|MAXIM|[DS3234][DS3234]|

検証には，以下のモジュールを使いました．
- 秋月電子8564NB搭載モジュール - [http://akizukidenshi.com/catalog/g/gI-00233/][AkizukiRTC8564NB]
- Grove RTC - [https://www.seeedstudio.com/Grove-RTC.html][GroveRTC]
- SparkFun DeadOn RTC Breakout DS3234 - [https://www.sparkfun.com/products/10160][BOB-10160]

## 利用上の注意
個人の週末のお楽しみで作っているので，各ドライバの網羅的なテストはできていません．
また，私自身はロジックアナライザ等は持っていないので，SPIやI2Cレベルでの動作が
おかしい場合はデバッグできないので放置状態です．

## 外部リンク
- Adafruit Unified Sensor Driver - [https://github.com/adafruit/Adafruit_Sensor][AdafruitUSD]
- RTC-8564NB - [https://www5.epsondevice.com/ja/products/rtc/rtc8564nb.html][RTC8564NB]
- 秋月電子8564NB搭載モジュール - [http://akizukidenshi.com/catalog/g/gI-00233/][AkizukiRTC8564NB]
- DS1307 - [https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS1307.html][DS1307]
- Grove RTC - [https://www.seeedstudio.com/Grove-RTC.html][GroveRTC]
- DS3234 - [https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS3234.html][DS3234]
- SparkFun DeadOn RTC Breakout DS3234 - [https://www.sparkfun.com/products/10160][BOB-10160]

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
