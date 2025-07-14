# UART Echo 範例

## 專案簡介
這是一個使用MSP430 Timer_A並利用純軟體實現 UART通訊的範例。  
透過 GPIO + Timer的bit-bang方式實作 UART 傳輸與接收，並在收到完整訊息後自動回傳（Echo），  
同時以LED顯示接收成功的提示。

## 使用技術
- MSP430G2553 
- C 語言
- Timer_A Capture/Compare 中斷作為軟體 UART
- 單向傳輸 & 雙向接收 (Full-duplex)

## 功能說明
1. 開機後透過UART傳輸並顯示 READY. 訊息。
2. 外部透過串列埠工具（如 Tera Term）傳送字串以 `\r` 或 `\n` 結尾。
3. MCU 收到完整字串後：回傳收到的字串（Echo）
   - 板載 LED 亮起 50ms 作為提示

## 串口參數
- 波特率約 9600 bps
- 資料位元：8 位
- 無奇偶校驗
- 1 stop bit


---


