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

namespace zlpanel::multilingual::zh_Hans {
    static constexpr std::array kTexts = {
        "控制线性增益状态与处理后增益状态之间的拐点宽度。",
        "控制增益传递函数的非线性曲率。",
        "控制动态处理开始的阈值电平。",
        "控制增益传递函数的比率。",
        "选择处理风格。",
        "控制启动阶段的时间响应。",
        "控制释放阶段的时间响应。",
        "控制增益变化增加到目标电平所需的时间。",
        "控制增益变化降低到目标电平所需的时间。",
        "点击：打开 RMS 处理模式。",
        "控制 RMS 测量的积分窗口时间。",
        "控制 RMS 处理的相对启动/释放时间。",
        "控制 RMS 处理的混合百分比。",
        "控制允许的最大增益变化量。",
        "控制释放前保持增益变化的最小持续时间。",
        "控制处理后应用的补偿增益。",
        "按下：开始测量输入和输出的综合响度\n松开：根据测得的响度差自动应用补偿增益。",
        "控制全局湿信号混合百分比。",
        "中/侧 通道相互关联。",
        "左/右 通道相互关联。",
        "中/侧 通道关联至 中/侧 的最大值。",
        "左/右 通道关联至 左/右 的最大值。",
        "点击：交换侧链立体声输入通道。",
        "控制通道间的立体声耦合百分比。",
        "控制各通道的湿信号混合百分比。",
        "控制侧链信号的增益。",
        "按下：启用外部侧链输入\n松开：使用内部侧链信号。",
        "按下：监听旁链信号。",
        "松开：旁通插件处理。",
        "按下：监听差值信号。",
        "控制旁链检测器的前视延迟。",
        "选择过采样倍数。",
        "控制软削波器的驱动强度。",
        "双击：打开界面配置。",
        "控制增益传递函数的底限。",
        "点击：将最大增益变化设为无穷大。",
        "下行压缩：衰减高于阈值的信号以减小动态范围。",
        "上行压缩：提升低于阈值的信号以减小动态范围。",
        "下行扩展：衰减低于阈值的信号以增大动态范围。",
        "上行扩展：提升高于阈值的信号以增大动态范围。",
        "点击：切换电平表显示。",
        "点击：切换累积 RMS 分析面板。",
        "点击：切换增益传递曲线显示。",
        "点击：切换旁链均衡器界面。",
        "点击：切换旁链控制面板。",
        "选择幅度测量方法。",
        "选择幅度测量的立体声通道。",
        "选择幅度分析仪的时间长度。",
        "选择幅度分析仪的最小分贝值。"
    };
}
