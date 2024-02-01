# EPSON RX8900

このライブラリは，
エプソンのRTC[RX8900][RX8900]用の
ドライバです．


## 動作検証

RTCは秋月電子の
[AkizukiRTC_RX8900][高精度ＲＴＣ（リアルタイムクロック）　ＲＸ８９００　ＤＩＰ化モジュール]を使いました．

動作を確認した機種は以下の表のものになります．
| CPUアーキ | 利用した機種 |
|---|---|
| AVR | Arduino Mega |
| SAMD | Arduino MKR WiFi 1010 |
| SAM | Arduino Due |
| ESP32 | スイッチサイエンス ESP Developer 32 |

## 利用上の注意

``RTC_RX8900_U.h``に以下のようなデバッグや性能テストに用いるための機能を生かすフラグがあります．
必要に応じて有効・無効を変更してください．
```
#define DEBUG
```

## サンプルプログラム
サンプルプログラムは，本ドライバの各機能を実行した場合に，RTCの各レジスタが適切に設定されているか否かを
確認するためのものです．

``RTC_RX8900_U.h``の``DEBUG``定義を有効にした上で
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
チップのレジスタ値の詳細などの情報が必要な機能がありますので，EPSONのアプリケーションマニュアルを参照しながら読んでください．
## 初期化
### オブジェクト生成
```
RTC_RX8900_U(TwoWire * theWire, int32_t rtcID=-1)
```
RTCが用いるI2CのI/FとIDを指定してオブジェクトを生成．
|引数|内容|
|---|---|
|theWire|I2CのI/F|
|rtcID|rtcに番号をつける場合に利用．(デフォルト値は-1)|

### 初期化
```
bool  begin(bool init, uint32_t addr=RTC_EPSON_RX8900_DEFAULT_ADRS)
```

第1引数は，時刻やタイマ，アラームの設定を実施するか否かを示すフラグ．

I2Cのアドレスにデフォルトでないものを用いる場合は
第2引数で指定．指定しない場合は，デフォルトのアドレスで初期化を実施．

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

## アラーム機能
RX8900は1種類しかアラームがないため，以下の各関数の第1引数`num``は0限定です．

### アラーム設定
```
int   setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing)
```

RX8900はアラームのパラメータのうち，日付もしくは曜日のどちらかを用いる必要があります．このどちらを利用するかは第2引数のメンバ``type``で指定します．

| modeのメンバ変数 | 意味 |
|---|---|
| useInteruptPin | 無視 |
| type | 0:曜日 , 1:日付け |

第3引数の``timing``では，分・時・日(もしくは曜日)を指定しますが，特定の分や時ではなく，その要素を無視する指定をすることもできます．この場合は第3引数の該当メンバの値を255(0xFF)にしてください．


| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

### アラームモード設定
```
int   setAlarmMode(uint8_t num, alarm_mode_t * mode)
```
各パラメータは``setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing)``と同じです．

### アラーム制御
```
int   controlAlarm(uint8_t num, uint8_t action)
```
``action``を0に設定すると，アラームが停止，1に設定するとアラームが再開します．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## タイマ機能
RX8900はタイマが2種類あります．以下の関数の第1引数``num``でそれを指定し，``num=0``で利用できるのがある周波数信号が何回発生したかで判定するもの，``num=1``で利用できるのが，毎秒(もしくは毎分)発火するものの2種類です．


### タイマ設定
```
int   setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi)
```

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

#### ``num=0``の場合

第3引数のmultiはtimerカウンタ0の値，第2引数modeのメンバrepeatがtimerカウンタ1の値となります．それぞれの値の意味はアプリケーションマニュアルを参照してください．

第2引数modeの他の構造体メンバは以下のように取り扱われます．
- useInteruptPin : コントールレジスタ 4bit目(TIE bit)に割当て (0 ピン出力なし, 1 ピン出力あり)
- interval : extentionレジスタの0,1bit目(TSEL0,1)に割当て (カウントする周波数信号の種類)

#### ``num=1``の場合
第2引数の構造体メンバ``interval``のみが意味を持ち， extentionレジスタの5bit目(USEL)に代入されます．
なお，値が0の場合は毎秒，値が1の場合は毎分発火します．

### タイマのモード設定
```
int   setTimerMode(uint8_t num, rtc_timer_mode_t * mode)
```

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

#### ``num=0``の場合

第2引数modeのメンバrepeatは無視され，他のメンバ変数は以下のように取り扱われます．
- useInteruptPin : コントールレジスタ 4bit目(TIE bit)に割当て (0 ピン出力なし, 1 ピン出力あり)
- interval : extentionレジスタの0,1bit目(TSEL0,1)に割当て (カウントする周波数信号の種類)

#### ``num=1``の場合
``setTimer()``と同じ動作となります．

### タイマの制御
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
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## クロック信号出力関係
周波数信号を出力する端子は1つしかないため，以下の各関数の第1引数``num``は0限定です．
### クロック出力設定
```
int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1)
```

RX8025は出力できる信号は周波数を選択できません．
そのため，第2引数は無視します．

また，RX8025はある端子(FOE端子)を外部から電圧をHIGHにしないと周波数信号を出力しないため，RTCのFOE端子と接続しているArduinoのピン番号を第3引数で与えます．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

### クロック周波数設定
```
int   setClockOutMode(uint8_t num, uint8_t freq)
```

FOUT端子に出力する信号の周波数を選択します．第2引数をextensionレジスタのFSELビットに代入されるため，その意味はアプリケーションマニュアルを参照してください．

### クロック出力の制御
```
int   controlClockOut(uint8_t num, uint8_t mode)
```
第2引数は以下の表のような意味となります．


|``mode``の値|意味|
|---|---|
|0|クロック出力停止|
|1|クロック出力開始|

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


## 割り込みの確認
```
int   checkInterupt(void)
```
割り込みの有無を16bitの2進数の列として出力．どのbitが立っているかでどの割り込みが発生しているかを判定することができる．
出力値はflagレジスタ(0x0E番)の3～5bit目が出力されるため，その意味はアプリケーションマニュアルを参照してください．

| 返り値 | 意味 |
|---|---|
|0以上 |flagレジスタ(0x0E番)の3～5bit目の値|
|RTC_U_FAILURE |取得失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## 割り込みフラグの解除
```
int  clearInterupt(uint16_t type)
```
``checkInterupt()``の出力と同じく，消すフラグをbit列の中で1として表記して与える．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## 電源電圧低下関係
### 電源フラグの取得
```
int checkLowPower(void)
```
RTCへの電源供給が切れ，時刻やタイマ等全ての設定をやり直す必要があるか否かを示すフラグの情報を取得する．負の値(``RTC_U_FAILURE``)の場合はフラグ値の取得失敗，0以上の場合は読み取りに成功しています．

値としては，``0``以外の値の場合はなんらかの異常が起こっています．この値はflagレジスタの0,1bit目の値になるため，詳細はアプリケーションマニュアルを参照してください．


### 電源フラグのクリア
```
int clearPowerFlag(void)
```
電源喪失を示すフラグをクリアする関数．返り値が負の場合はフラグ値のクリアに失敗．成功の場合は``RTC_U_SUCCESS``が返されます．

## 温度補正機能
一般的に発振器はチップの温度で発振周波数が変化してしまうため，RX8900は内部温度で時計の進み方を自動で調整します．この機能は止めることはできませんが，調整する頻度だけは指定することができます．
### 内部温度の取得
```
float getTemperature(uint8_t mode)
```

RTCの内部温度を取得します．引数の意味は以下の表の通りです．

|modeの値|意味|
|---|---|
|RTC_U_TEMPERATURE_KELVIN|絶対零度で温度を返します|
|RTC_U_TEMPERATURE_CELCIUS|摂氏で温度を返します|
|RTC_U_TEMPERATURE_FAHRENHEIT|華氏で温度を返します|


### 調整周期の設定
```
int   setTemperatureFunction(uint8_t mode)
```

RX8900では調整機能はON/OFFできませんが，調整を実行する周期は指定できます．デフォルトは2秒周期です．

|modeの値(2進数)|周期|
|---|---|
| 00 | 0.5秒 |
| 01 | 2秒 |
| 10 | 10秒 |
| 11 | 30秒 |

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## SRAM領域へのアクセス
RV8803ではレジスタ番号``0x07``がSRAM領域として利用できます．以下の2つの関数の第1引数``addr``が0の場合に，このレジスタを利用します．
また，``len``は1しか利用できません．

### SRAM領域からの読み取り
```
int getSRAM(uint8_t addr, uint8_t *array, uint16_t len)
```
レジスタ番号``0x07``のデータを配列arrayの最初の要素に代入されます．

### SRAM領域への書き込み
```
int setSRAM(uint8_t addr, uint8_t *array, uint16_t len)
```
配列arrayの最初の要素のデータをレジスタ番号``0x07``に書き込みます．


[RX8900]:https://www5.epsondevice.com/ja/products/rtc/rx8900sa.html
[AkizukiRTC_RX8900]:https://akizukidenshi.com/catalog/g/gK-13009/
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
