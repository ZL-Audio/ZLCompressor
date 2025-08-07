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

namespace zlpanel::multilingual::zh_Hans {
    static constexpr std::array kTexts = {
        "调整未压缩和压缩状态之间的过渡长度。",
        "调整压缩状态的非线性行为。",
        "调整压缩状态的起始点。",
        "调整压缩状态下的增益衰减量。",
        "选择压缩类型。",
        "调整启动阶段的行为。",
        "调整释放阶段的行为。",
        "调整增益衰减的增加速度。",
        "调整增益衰减的减小速度。",
        "按下：启用 RMS 压缩\n释放：关闭RMS压缩。",
        "调整 RMS 测量的时长。",
        "调整 RMS 压缩的相对启动/释放速度。",
        "调整 RMS 压缩的混合百分比。",
        "调整增益衰减的最大值。",
        "调整增益衰减被阻止减小的最短时间。",
        "调整压缩后施加的补偿增益。",
        "按下：开始测量输入和输出信号的综合响度\n释放：将补偿增益更新为两个响度值的差值。",
        "调整处理后信号（湿信号）的百分比。",
        "中/侧（M/S）声道链接。",
        "左/右（L/R）声道链接。",
        "中/侧（M/S）声道与中/侧（M/S）最大值链接。",
        "左/右（L/R）声道与左/右（L/R）最大值链接。",
        "按下：交换侧链立体声通道。",
        "调整两个立体声通道的链接百分比。",
        "调整每个立体声通道的处理后信号（湿信号）百分比。",
        "调整侧链信号的增益。",
        "按下：使用外部侧链信号\n释放：使用内部侧链信号。",
        "按下：监听侧链信号。",
        "释放：旁通插件。",
        "按下：输出差异信号。",
        "调整旁链信号的前瞻时间。",
        "选择过采样倍数。",
        "调整削波器的相对驱动量。",
        "双击：打开用户界面设置。"
    };
}
