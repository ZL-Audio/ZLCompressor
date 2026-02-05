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

namespace zlpanel::multilingual::ja {
    inline constexpr std::array kTexts = {
        "リニアゲイン状態と処理後のゲイン状態の間のニー（Knee）幅を制御します。",
        "ゲイン伝達関数の非線形カーブを制御します。",
        "ダイナミクス処理が開始されるスレッショルド（閾値）レベルを制御します。",
        "ゲイン伝達関数のレシオ（比率）を制御します。",
        "処理スタイルを選択します。",
        "アタックステージの時間応答を制御します。",
        "リリースステージの時間応答を制御します。",
        "ゲイン変化が目標レベルまで増加するのにかかる時間を制御します。",
        "ゲイン変化が目標レベルまで減少するのにかかる時間を制御します。",
        "クリック：RMS処理モードを有効にします。",
        "RMS測定の積分ウィンドウ時間を制御します。",
        "RMS処理の相対的なアタック/リリース時間を制御します。",
        "RMS処理のミックス割合（％）を制御します。",
        "許容される最大ゲイン変化量を制御します。",
        "リリース前にゲイン変化を保持する最小時間（ホールド）を制御します。",
        "処理後に適用されるメイクアップゲインを制御します。",
        "押下：入力と出力の統合ラウドネス測定を開始します\n松開：測定されたラウドネス差に基づいてメイクアップゲインを自動適用します。",
        "全体のウェット信号のミックス割合（％）を制御します。",
        "Mid/Sideチャンネルを相互にリンクします。",
        "Left/Rightチャンネルを相互にリンクします。",
        "Mid/SideチャンネルをMid/Sideの最大値にリンクします。",
        "Left/RightチャンネルをLeft/Rightの最大値にリンクします。",
        "クリック：サイドチェーンのステレオ入力チャンネルを入れ替えます。",
        "チャンネル間のステレオカップリング率（％）を制御します。",
        "各チャンネルのウェット信号のミックス割合（％）を制御します。",
        "サイドチェーン信号のゲインを制御します。",
        "押下：外部サイドチェーン入力を有効にします\n松開：内部サイドチェーン信号を使用します。",
        "押下：サイドチェーン信号をモニターします。",
        "松開：プラグイン処理をバイパスします。",
        "押下：差分（Delta）信号をモニターします。",
        "サイドチェーン検出器の先読み（ルックアヘッド）遅延を制御します。",
        "オーバーサンプリング倍率を選択します。",
        "ソフトクリッパーのドライブ強度を制御します。",
        "ダブルクリック：UI設定を開きます。",
        "ゲイン伝達関数のフロア（下限）を制御します。",
        "クリック：最大ゲイン変化を無限大に設定します。",
        "ダウンワード・コンプレッション：ダイナミックレンジを狭めるため、スレッショルドを超える信号を減衰させます。",
        "アップワード・コンプレッション：ダイナミックレンジを狭めるため、スレッショルドを下回る信号をブーストします。",
        "ダウンワード・エキスパンション：ダイナミックレンジを広げるため、スレッショルドを下回る信号を減衰させます。",
        "アップワード・エキスパンション：ダイナミックレンジを広げるため、スレッショルドを超える信号をブーストします。",
        "クリック：メーター表示を切り替えます。",
        "クリック：累積RMS分析パネルを切り替えます。",
        "クリック：ゲイン伝達曲線表示を切り替えます。",
        "クリック：サイドチェーンEQ画面を切り替えます。",
        "クリック：サイドチェーン操作パネルを切り替えます。",
        "振幅測定方法を選択します。",
        "振幅測定を行うステレオチャンネルを選択します。",
        "振幅アナライザーの時間長を選択します。",
        "振幅アナライザーの最小デシベル値を選択します。"
    };
}
