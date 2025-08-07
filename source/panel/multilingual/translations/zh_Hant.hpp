// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

// This file is also dual licensed under the Apache License, Version 2.0. You may obtain a copy of the License at <http://www.apache.org/licenses/LICENSE-2.0>

#pragma once

#include <array>

namespace zlpanel::multilingual::zh_Hant {
    static constexpr std::array kTexts = {
        "調整未壓縮和壓縮狀態之間的過渡長度。",
        "調整壓縮狀態的非線性行為。",
        "調整壓縮狀態的起始點。",
        "調整壓縮狀態下的增益衰減量。",
        "選擇壓縮類型。",
        "調整啟動階段的行為。",
        "調整釋放階段的行為。",
        "調整增益衰減的增加速度。",
        "調整增益衰減的減小速度。",
        "按下：啟用 RMS 壓縮\n釋放：關閉 RMS 壓縮。",
        "調整 RMS 測量的時長。",
        "調整 RMS 壓縮的相對啟動/釋放速度。",
        "調整 RMS 壓縮的混合百分比。",
        "調整增益衰減的最大值。",
        "調整增益衰減被阻止減小的最短時間。",
        "調整壓縮後施加的補償增益。",
        "按下：開始測量輸入和輸出信號的綜合響度\n釋放：將補償增益更新為兩個響度值的差值。",
        "調整處理後信號（濕信號）的百分比。",
        "中/側（M/S）聲道連結。",
        "左/右（L/R）聲道連結。",
        "中/側（M/S）聲道與中/側（M/S）最大值連結。",
        "左/右（L/R）聲道與左/右（L/R）最大值連結。",
        "按下：交換側鏈立體聲通道。",
        "調整兩個立體聲通道的連結百分比。",
        "調整每個立體聲通道的處理後信號（濕信號）百分比。",
        "調整側鏈信號的增益。",
        "按下：使用外部側鏈信號\n釋放：使用內部側鏈信號。",
        "按下：監聽側鏈信號。",
        "釋放：旁通插件。",
        "按下：輸出差異信號。",
        "調整旁鏈信號的前瞻時間。",
        "選擇過採樣倍數。",
        "調整削波器的相對驅動量。",
        "雙擊：打開用戶界面設置。"
    };
}
