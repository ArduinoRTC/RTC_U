# DS1307

このライブラリは，RTCを取り扱う統一的なAPIを作る思いつきの一部で
[Grove RTC][GroveRTC]にも搭載されている[DS1307][DS1307]用の
ライブラリ(デバイスドライバ)です．

以前はGrove RTC用の[https://github.com/Seeed-Studio/RTC_DS1307][github]のラッパのような実装になっていましたが，完全独自にしました．


## 動作検証
秋月電子の[ＤＳ１３０７使用リアルタイムクロック（ＲＴＣ）モジュールキット][https://akizukidenshi.com/catalog/g/gK-15488/]を使いました．

以下の表の機種で動作を確認しています．
| CPU | 機種 | 対応状況 |
|---|---|---|
| AVR | Arduino Mega | ○ |
| SAMD | Arduino MKR WiFi1010 | ○ |
| SAM | Arduino Due | ○ |
| ESP32 | スイッチサイエンスESP developer32 | ○ |


## 外部リンク

- DS1307 - [https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS1307.html][DS1307]
- Grove RTC - [https://www.seeedstudio.com/Grove-RTC.html][GroveRTC]
- Seeed studio RTC_DS1307 library [https://github.com/Seeed-Studio/RTC_DS1307][github]


## 利用上の注意

``RTC_DS1307_U.h``に以下のようなデバッグや性能テストに用いるための機能を生かすフラグがあります．
必要に応じて有効・無効を変更してください．
```
#define DEBUG
```

## サンプルプログラム
サンプルプログラムは，本ドライバの各機能を実行した場合に，RTCの各レジスタが適切に設定されているか否かを
確認するためのものです．

``RTC_DS1307_U.h``の``DEBUG``定義を有効にした上で
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
maximのデータシートを合わせて読んでください．
## 初期化
### オブジェクト生成
```
RTC_DS1307_U(TwoWire *theWire, int32_t rtcID = -1)
```
RTCが用いるI2CのI/FとIDを指定してオブジェクトを生成．
|引数|内容|
|---|---|
|theWire|I2CのI/F|
|rtCID|rtcに番号をつける場合に利用．(デフォルト値は-1)|


### 初期化
```
bool  begin(bool init, uint32_t addr=RTC_DS1307_DEFAULT_I2C_ADDR)
```
第1引数は，時刻やタイマ，アラームの設定を実施するか否かを示すフラグで``false``の場合はI2Cまわりの初期化しかしません．

DS1307のデフォルトI2Cのアドレスではない番号を持つモジュールを
用いる場合は，第2引数で指定．指定しない場合は，デフォルトのアドレスで初期化を実施．

| 返り値 | 意味 |
|---|---|
|true|初期化成功|
|false|初期化失敗|

## RTCの情報の取得
RTCのチップの種類や機能の情報を取得するメンバ関数．
```
void  getRtcInfo(rtc_u_info_t *info)
```

## 時刻関係
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

## 周波数信号出力関係
DS1307は周波数信号を出力する端子/機能は1つしかないので，以下の関数の第1引数の値は0限定です．
### 出力設定
```
int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1)
```

DS1307は周期信号の出力を外部からの信号入力で制御する機能はないため，第3引数は無視されます．

第2引数の値は以下の表の通りとなります．

|``freq``の値|クロック周波数|
|---|---|
|0|1Hz|
|1|4kHz|
|2|8kHz|
|3|32kHz|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_UNSUPPORTED |サポートしていないパラメータの設定など|

### クロック周波数設定
```
int   setClockOutMode(uint8_t num, uint8_t freq)
```
``setClockOut()``の第3引数の``pin``以外は同じであり，``setClockOut()``では``pin``が無視されるため，
``setClockOut()``と同じ動作を行います．

第2引数は``setClockOut()``と同じく下の表の通りです．
|``freq``の値|クロック周波数|
|---|---|
|0|1Hz|
|1|4kHz|
|2|8kHz|
|3|32kHz|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_UNSUPPORTED |サポートしていないパラメータの設定など|


### クロック出力の制御
```
int   controlClockOut(uint8_t num, uint8_t mode)
```
|``mode``の値|意味|
|---|---|
|0|クロック出力停止|
|1|クロック出力開始|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_UNSUPPORTED |サポートしていないパラメータの設定など|

## 計時用クロックの停止/再開
本RTCは，消費電力削減用に時刻のカウントアップを停止/再開する機能があります．(レジスタ番号00hの最上位bit``CH``)

### クロックの状態の参照
```
int   clockHaltStatus(void)
```
電源電圧降下(電源断)や人為的に計時を止めたか否かを確認できます．

|返り値|意味|
|---|---|
|0|クロックは動作している|
|1|クロックは止まっている|
|RTC_U_FAILURE|レジスタ読み出し失敗|

### 計時用クロックの制御
```
int   controlClockHalt(uint8_t mode)
```
計時のクロック動作を止める/再開する関数．

|``mode``の値 | 意味|
|---|---|
|0 |クロック停止|
|1 |クロック再開|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|

## SRAM領域へのアクセス
DS1307ではレジスタ番号``0x08``から56個がSRAM領域として利用できます．以下の2つの関数の第1引数``addr``が0の場合にレジスタ番号``0x08``
からデータを読み取ります．また，第1引数``addr``と第3引数``len``の合計がレジスタの数より大きい場合はエラーとなります．

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
