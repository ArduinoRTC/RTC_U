# NXP PCF8523

このライブラリは，RTCを取り扱う統一的なAPIを作る思いつきの一部で
[NXP PCF8523][PCF8523]用の
ライブラリ(デバイスドライバ)です．

## 動作検証


| CPU | 機種 | 対応状況 |
|---|---|---|
| AVR | Arduino Mega | ○ |
| SAMD | Arduino MKR WiFi1010 | ○ |
| SAM | Arduino Due | ○ |
| ESP32 | スイッチサイエンスESP developer32 | ○ |


## 外部リンク


- NXP PCF8523 データシート - [https://www.nxp.com/docs/en/data-sheet/PCF8523.pdf](https://www.nxp.com/docs/en/data-sheet/PCF8523.pdf)
- Adafruit PCF8523 Real Time Clock Assembled Breakout Board - [https://www.adafruit.com/product/3295](https://www.adafruit.com/product/3295)

## 利用上の注意

``RTC_PCF8523_U.h``に以下のようなデバッグや性能テストに用いるための機能を生かすフラグがあります．
必要に応じて有効・無効を変更してください．
```
#define DEBUG
```

## サンプルプログラム
サンプルプログラムは，本ドライバの各機能を実行した場合に，RTCの各レジスタが適切に設定されているか否かを
確認するためのものです．

``RTC_PCF8523_U.h``の``DEBUG``定義を有効にした上で
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
以下の各関数の説明を理解するために，RTCの各レジスタのbitが持つ意味を知る必要がありますので，[データシート][データシート]を見ながら読んでください．

## 初期化関係
### オブジェクト生成
```
RTC_PCF8523_U(TwoWire * theWire, int32_t rtcID=-1)
```
RTCが用いるI2CのI/FとIDを指定してオブジェクトを生成．

### 初期化
```
bool  begin(bool init=true, uint8_t addr=RTC_PCF8523_DEFAULT_ADRS)
```
RTCの初期化．引数で``false``を与えると，時刻設定等を行わなず，インターフェイスの最小限の初期化のみを行うので，RTCに時刻設定を行った後にArduinoを再起動した場合に便利です．
第2引数は，RTCデフォルトのI2Cアドレスを用いない場合に指定してください．

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


## 電源関係

### 電源電圧監視機能の設定
```
int   setLowPower(uint8_t mode)
```

引数``mode``はControl 3レジスタ(0x02番)に書き込む値を指定します．
各bitの意味は以下の表の通りですが，PMの内容はデータシートを参照してください．

| bit | 名前 | 内容 |
|---|---|---|
| 5から7 | PM | 電源電圧低下を監視する機能の選択 |
| 2から4 |  | 未使用 |
| 1 | BSIE | 電源切替発生を示すフラグを使うか否か |
| 0 | BLIE | 電池電圧低下を示すフラグを使うか否か |

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

### 電源電圧
```
int   checkLowPower(void)
```

情報の取得に成功した場合の返り値の内容は以下の通り．
| bit | 名前 | 内容 | 意味 |
|---|---|---|---|
| 7 | OS | 秒レジスタ(0x03)の7bitのデータ |0なら異常なし|
| 4から6 | なし | 未使用 | 意味無し |
| 0から3 | なし | Control 3レジスタ(0x02)の0から3bit目のデータ | データシート参照 |

大まかには，7bit目が1の場合はなんらかの異常があり，その詳しい内容が0から3bit目に出力されている．
もし，情報の取得(レジスタアクセス)に失敗した場合は``RTC_U_FAILURE``が返される．


### 電源電圧低下に関するフラグのクリア
```
int   clearPowerFlag(void)
```
電源異常に関する情報を消去(0を書き込み)．
- Control 3レジスタ(0x02)の2,3bitを0に設定
- 秒レジスタ(0x03)の7bitを0に設定

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


### 電源監視機能の設定
```
setLowPower(uint8_t mode)
```

modeの一部bitの値でcontrol3の対応するbitを上書き．上書きされるbitは以下の通り．

|control 3レジスタのbit | 名前 | 書き換え対象か否か |
|---|---|---|
| 5から7 | PM | ○ |
| 4 | - | × |
| 3 | BSF | × |
| 2 | BLF | × |
| 1 | BSIE | ○ |
| 0 | BLIE | ○ |

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## 割り込み関係
割り込みが発生したことを示すフラグがControl 2レジスタ(0x01)の3から7bitに集中しています．
### 発生した割り込み情報の取得
```
int   checkInterupt(void)
```

Control 2レジスタ(0x01)のデータを3bit右シフトして，割り込み関係のデータだけにした上で返します．各bitの示す情報はデータシートを参照してください．

| 返り値 | 意味 |
|---|---|
|0以上 |Control2から読み取ったデータ|
|RTC_U_FAILURE |control2レジスタ読み取りに失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


### 割り込み情報のクリア
```
int   clearInterupt(uint16_t type)
```
Control 2レジスタ(0x01)の3から7bitのデータのうち，typeで指定した(1が立っているbit)を0でクリアします．
typeのbitとControl2レジスタのbitの対応関係は，typeの最下位bitがControl2レジスタの3bitとなります．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## 時計調整
PCF8523は時計の進み具合を調整する機能の他に，電池駆動の場合に時計を完全に止める機能の2種類があります．また，RTCのソフトウェアリセット機能も存在しています．

### 時計を止める/進める
```
int   controlClockHalt(uint8_t mode)
```

| modeの値 | 処理 |
|---|---|
| 0 | 時計を止める |
| 1 | 時計を進める |


| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|

### 時計が止まっているか否かの情報の取得
```
int   clockHaltStatus(void)
```

| 返り値 | 意味 |
|---|---|
| 0 |時計は動作している|
| 1 |時計は止まっている|
|RTC_U_FAILURE |情報取得失敗|

### ソフトウェアリセット
```
int   controlClock(void)
```
| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|

### 時計の進み方の調整
```
int   setOscillator(uint8_t mode)
```
引数``mode``にoffsetレジスタに設定する内容をそのまま代入して呼び出す必要があります．offsetレジスタの詳細についてはデータシートを参照してください．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|

### 時計の進み方の設定内容参照
```
int   getOscillator(void)
```
offsetレジスタ(0x0E)のデータをそのまま返します．

| 返り値 | 意味 |
|---|---|
|0以上 |offsetレジスタの内容|
|RTC_U_FAILURE |読み取り失敗|

## クロック信号出力
本RTCは同時に出力可能な信号の数は1つであるため，以下の各関数の第1引数は0限定となります．



### クロック信号出力設定変更
```
int   setClockOutMode(uint8_t num, uint8_t freq)
```
第2引数``freq``で出力する周波数を指定し，出力周波数を変更します．
``freq``の値はtimer and CLKOUT controlレジスタ(0x0F)の3から5bit(COF)に
上書きされるものであるため，``freq``も2進数3bitとなります．

``freq``の値がどのような周波数に対応するかは，データシートを参照してください．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


### 信号出力のON/OFF
```
int   controlClockOut(uint8_t num, uint8_t mode)
```

|modeの値 | 意味 |
|---|---|
| 0 | 信号出力を止める |
| 1 | 信号出力再開(止まっていたものを再開) |

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|



### クロック信号出力設定
```
int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1)
```
クロック信号出力を制御するために，RTCの特定端子に加える電圧を調整するという機能は本RTCに存在しないため，このAPIは上の``setClockOutMode(num,mode)``と``controlClockOut(num,1)``をまとめて実行するだけになります．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

## アラーム関係
PCF8523のアラームは1種類なので，以下の各関数の第1引数``num``は0限定です．

### アラーム設定
```
int   setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing)
```

modeの構造体メンバuseInteruptPinが1の場合にINT端子に割り込み信号が発生する設定となる．

PCF8523のアラームは分,時,日,曜日が指定可能で，このうち，利用しない項目は
対応するtimingのメンバ変数に0xFFを代入してください．

例えば，時と分だけを指定する場合は``timing.mday=0xFF``と``timing.wday=0xFF``としてこの関数を実行する必要があります．


| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|



### アラームモード設定
```
int   setAlarmMode(uint8_t num, alarm_mode_t * mode)
```

引数の意味や動作は``setAlarm()``と同じです．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


### アラーム制御
```
int   controlAlarm(uint8_t num, uint8_t action)
```

``action``でアラームのON(1)/OFF(0)を指定します．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


## タイマ
タイマは4つありますが，すべてが同時に利用可能というわけではありません．内部的には2つの回路(AとB)で1(Bは独立)と3つの(Aが自分とB以外の2つ)タイマを担っているので制限が発生します．詳細はデータシートを参照してください．

なお，以下の各関数において，第1引数``num``と4種類のタイマの関係は以下の表となります．

| ``num``の値`` | タイマ |
|---|---|
| 0 | タイマB |
| 1 | タイマA |
! 2 | watchdogタイマ |
| 3 | 定周期(Second)タイマ |

各タイマの詳細な動作はデータシートを参照してください．

### 各タイマのパラメータ

タイマの関数には``rtc_timer_mode_t mode``と``uint16_t multi``の2種類の引数でタイマのパラメータを設定します．各タイマでこの引数がどのように扱われるか(どのレジスタのどのbitに代入されるか)をまとめておきます．各レジスタのbitの意味についてはデータシートを参照してください．

| rtc_timer_mode_tのメンバ | タイマB (num=0) | タイマA (num=1) | watchdogタイマ(num=2) | 定周期(Second)タイマ(num=3)|
|---|---|---|---|---|
| uint8_t  repeat |timer and CLKOUT controlレジスタのTBM  |timer and CLKOUT controlレジスタのTAM  | 同左 | 同左 |
| uint8_t  useInteruptPin |control 2レジスタのCTBIE |control 2レジスタのCTAIE |control 2レジスタのWTAIE|(未使用) |
| uint8_t  interval |timer B frequency controlレジスタのTBQ |timer A frequency controlレジスタのTAQ |同左 |(未使用) |
| uint8_t  pulse  |timer B frequency controlレジスタのTBW | (未使用)    | (未使用) | (未使用) |

|タイマの種類| multi |
|---|---|
|タイマB|下位8bitをtimer Bレジスタに代入|
|タイマA|下位8bitをtimer Aレジスタに代入|
|Watchdogタイマ|同上|
|定周期(Second)タイマ| (未使用) |

### タイマの設定
```
int setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi)
```
上の表に従い，multiの値をタイマの種類毎に処理(レジスタに代入)を行い，``setTimerMode(mode)``と``controlTimer(num,1)``を中で順に呼び出しています．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|


### タイマのパラメータ変更
```
int setTimerMode(uint8_t num, rtc_timer_mode_t * mode)
```

ここでは，上のパラメータの表に従い，mode引数の構造体各メンバの値をレジスタに代入しています．

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|

### タイマのON/OFF
```
int controlTimer(uint8_t num, uint8_t action)
```

|actionの値|意味|
|---|---|
| 0 | タイマを止める |
| 1 | タイマの動作を再開させる(止めたもののみ) |

| 返り値 | 意味 |
|---|---|
|RTC_U_SUCCESS |設定成功|
|RTC_U_FAILURE |設定失敗|
|RTC_U_ILLEGAL_PARAM |サポートしていないパラメータの設定など|



[データシート]:https://www.nxp.com/docs/en/data-sheet/PCF8523.pdf
[AdafruitPCF8523]:https://www.adafruit.com/product/3295
[PCF8523]:https://www.nxp.com/products/peripherals-and-logic/signal-chain/real-time-clocks/rtcs-with-ic-bus/100-na-real-time-clock-calendar-with-battery-backup:PCF8523


[github]:https://github.com/sparkfun/SparkFun_DS3231_RTC_Arduino_Library
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



