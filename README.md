# OOPL2026 Class Project

本專案為 OOPL2026 課程練習專案，基於 [PTSD Template](https://github.com/ntut-open-source-club/ptsd-template) 進行修改與開發。

專案目標是以物件導向程式設計的方式，練習專案遊戲開發。

## 快速開始

1. 複製專案：

   ```bash
   git clone https://github.com/selflucaslu/Infinitode-2-Tower-Defense-OOPL2026.git --recursive
   ```
2. 使用 `Debug` 模式設定專案：

   > [!WARNING]
   > 請使用 `Debug` 模式建置專案，因為目前 `Release` 模式仍有問題。
   > 此外，請加入 `-DCMAKE_POLICY_VERSION_MINIMUM=3.5`，以避免舊版 CMake 政策造成的相容性問題。
   >

   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POLICY_VERSION_MINIMUM=3.5
   ```
3. 建置專案：

   ```bash
   cmake --build build
   ```

   Mac 平行編譯加快指令（Windows 請將 -j 後面數值換成CPU 核心數）：

   ```
   cmake --build build -j$(sysctl -n hw.ncpu)
   ```
4. 執行專案：

   ```bash
   ./build/REPLACE_WITH_YOUR_PROJECT_NAME
   ```

## 塔臺列表

|        主塔名稱        | 外型分類 |  |
|:------------------:|:----:| :---:| 
|    Basic (基礎塔)     | Ammo |
|    Sniper (狙擊塔)    | Ammo |
|    Cannon (加農塔)    | Ammo |
|   Freezing (冰凍塔)   | Around_Skill |
|    Venom (毒液塔)     | Ammo |
|    Splash (濺射塔)    | Ammo |
|    Blast (爆破塔)     | Around_Skill |
|  Multishot (多重塔)   | Ammo |
|   Minigun (機槍塔)    | Ammo |
|     Air (對空塔)      | Ammo |
|    Tesla (特斯拉塔)    | Ammo |
|   Missile (導彈塔)    | Ammo |
| Flamethrower (火焰塔) | Ammo |
|    Laser (雷射塔)     | Ammo |
|    Gauss (高斯塔)     | Ammo |
|    Crusher(粉碎塔)    | Ammo |