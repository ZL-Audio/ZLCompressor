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

namespace zlpanel::multilingual::ja {
    static constexpr std::array kTexts = {
        "未圧縮状態と圧縮状態の間の移行の長さを調整します。",
        "圧縮状態の非線形な動作を調整します。",
        "圧縮が開始されるスレッショルド（閾値）を調整します。",
        "圧縮率を調整します。",
        "圧縮のスタイルを選択します。",
        "アタック段階の動作を調整します。",
        "リリース段階の動作を調整します。",
        "ゲインリダクションが増加する速さを調整します。",
        "ゲインリダクションが減少する速さを調整します。",
        "押す：RMS圧縮を有効化\n離す：RMS圧縮を無効化。",
        "RMS測定の長さを調整します。",
        "RMS圧縮の相対的なアタック/リリースの速さを調整します。",
        "RMS圧縮のミックス率を調整します。",
        "ゲインリダクションの最大値を調整します。",
        "ゲインリダクションの減少が抑制される最小時間を調整します。",
        "圧縮後に適用されるメイクアップゲインを調整します。",
        "押す：入力信号と出力信号の統合ラウドネスの測定を開始\n離す：2つのラウドネス値の差分にメイクアップゲインを更新します。",
        "ウェット信号（処理後の信号）の割合を調整します。",
        "M/Sチャンネルがリンクされます。",
        "L/Rチャンネルがリンクされます。",
        "M/Sチャンネルが、M/Sの最大値にリンクされます。",
        "L/Rチャンネルが、L/Rの最大値にリンクされます。",
        "押す：サイドチェーンのステレオチャンネルを入れ替えます。",
        "2つのステレオチャンネル間のリンク率を調整します。",
        "各ステレオチャンネルのウェット信号の割合を調整します。",
        "サイドチェーン信号のゲインを調整します。",
        "押す：外部サイドチェーン信号を使用\n離す：内部サイドチェーン信号を使用。",
        "押す：サイドチェーン信号を聴きます。",
        "離す：プラグインをバイパスします。",
        "押す：差分信号を出力します。",
        "サイドチェーン信号のルックアヘッドタイム（先行読み込み時間）を調整します。",
        "オーバーサンプリングの倍率を選択します。",
        "クリッパーの相対的なドライブ量を調整します。",
        "ダブルクリック：UI設定を開きます。"
    };
}
