// Copyright (C) 2026 - zsliu98
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
    inline constexpr std::array kTexts = {
        "控制線性增益狀態與處理後增益狀態之間的拐點寬度。",
        "控制增益傳遞函數的非線性曲率。",
        "控制動態處理開始的閾值電平。",
        "控制增益傳遞函數的比率。",
        "選擇處理風格。",
        "控制啟動階段的時間響應。",
        "控制釋放階段的時間響應。",
        "控制增益變化增加到目標電平所需的時間。",
        "控制增益變化降低到目標電平所需的時間。",
        "點擊：開啟 RMS 處理模式。",
        "控制 RMS 測量的積分視窗時間。",
        "控制 RMS 處理的相對啟動/釋放時間。",
        "控制 RMS 處理的混合百分比。",
        "控制允許的最大增益變化量。",
        "控制釋放前保持增益變化的最小持續時間。",
        "控制處理後應用的補償增益。",
        "按下：開始測量輸入和輸出的綜合響度\n鬆開：根據測得的響度差自動應用補償增益。",
        "控制全域濕訊號混合百分比。",
        "中/側 通道相互關聯。",
        "左/右 通道相互關聯。",
        "中/側 通道關聯至 中/側 的最大值。",
        "左/右 通道關聯至 左/右 的最大值。",
        "點擊：交換側鏈立體聲輸入通道。",
        "控制通道間的立體聲耦合百分比。",
        "控制各通道的濕訊號混合百分比。",
        "控制側鏈訊號的增益。",
        "按下：啟用外部側鏈輸入\n鬆開：使用內部側鏈訊號。",
        "按下：監聽側鏈訊號。",
        "鬆開：旁通外掛程式處理。",
        "按下：監聽差值訊號。",
        "控制側鏈偵測器的前視延遲。",
        "選擇超取樣倍數。",
        "控制軟削波器的驅動強度。",
        "雙擊：開啟介面配置。",
        "控制增益傳遞函數的底限。",
        "點擊：將最大增益變化設為無限大。",
        "下行壓縮：衰減高於閾值的訊號以減小動態範圍。",
        "上行壓縮：提升低於閾值的訊號以減小動態範圍。",
        "下行擴展：衰減低於閾值的訊號以增大動態範圍。",
        "上行擴展：提升高於閾值的訊號以增大動態範圍。",
        "點擊：切換電平表顯示。",
        "點擊：切換累積 RMS 分析面板。",
        "點擊：切換增益傳遞曲線顯示。",
        "點擊：切換側鏈等化器介面。",
        "點擊：切換側鏈控制面板。",
        "選擇振幅測量方法。",
        "選擇振幅測量的立體聲通道。",
        "選擇振幅分析儀的時間長度。",
        "選擇振幅分析儀的最小分貝值。"
    };
}
