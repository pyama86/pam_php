<?php

error_reporting(E_ALL);
ini_set('display_errors', 1);

function pam_authenticate($pamh): int
{
    echo "PHPカンファレンス福岡2024へようこそ！\n";
    $quizCount = 3;
    $phpQuizzes = [
        [
            "question" => "前回のPHPカンファレンスの開催地は？",
            "choices" => [
                "A" => "東京",
                "B" => "大阪",
                "C" => "香川",
                "D" => "北海道"
            ],
            "answer" => "C"
        ],
        [
            "question" => "P山さんが一番好きなPHPの関数名は？",
            "choices" => [
                "A" => "var_dump",
                "B" => "print_r",
                "C" => "array_merge",
                "D" => "json_encode"
            ],
            "answer" => "A"
        ],
        [
            "question" => "清家さんといえば？",
            "choices" => [
                "A" => "サーバレスの達人",
                "B" => "データベース最適化",
                "C" => "フロントエンド開発",
                "D" => "モバイルアプリケーション"
            ],
            "answer" => "A"
        ],
        [
            "question" => "PHPカンファレンスと並ぶ日本最大級のPHPのカンファレンスは？",
            "choices" => [
                "A" => "PHPerKaigi",
                "B" => "DeveloperWeek",
                "C" => "CodeCon",
                "D" => "TechSummit"
            ],
            "answer" => "A"
        ],
        [
            "question" => "PHPカンファレンス福岡2024の実行委員長は？",
            "choices" => [
                "A" => "びきニキさん",
                "B" => "小山さん",
                "C" => "佐藤恵美",
                "D" => "鈴木一郎"
            ],
            "answer" => "A"
        ],
        [
            "question" => "forteeの正しい発音は？",
            "choices" => [
                "A" => "フォルテ",
                "B" => "フォーティー",
                "C" => "フェルテ",
                "D" => "フォルト"
            ],
            "answer" => "A"
        ],
        [
            "question" => "次のPHPカンファレンスが開催されるのは？",
            "choices" => [
                "A" => "沖縄",
                "B" => "広島",
                "C" => "京都",
                "D" => "名古屋"
            ],
            "answer" => "A"
        ],
        [
            "question" => "この世で最高のWAFは？",
            "choices" => [
                "A" => "Scutum",
                "B" => "ModSecurity",
                "C" => "Cloudflare",
                "D" => "AWS WAF"
            ],
            "answer" => "A"
        ]
    ];

    // クイズのランダム選択
    $selectedQuizzes = [];
    $indexes = array_rand($phpQuizzes, $quizCount);
    if (!is_array($indexes)) {
        $indexes = [$indexes];
    }
    foreach ($indexes as $index) {
        $selectedQuizzes[] = $phpQuizzes[$index];
    }

    $totalQuestions = count($selectedQuizzes);
    $user_name = get_user($pamh);
    echo "ユーザ名: $user_name\n";

    $correctCount = 0;
    foreach ($selectedQuizzes as $quiz) {
        echo "\n" . $quiz["question"] . "\n";
        foreach ($quiz["choices"] as $key => $value) {
            echo "$key: $value\n";
        }
        echo "\n";

        $valid = false;
        while (!$valid) {
            $answer = trim(ask_question($pamh, "あなたの回答 (A, B, C, D): "));
            $answer = strtoupper($answer);

            if (!in_array($answer, ['A', 'B', 'C', 'D'])) {
                echo "無効な入力です。A, B, C, Dのいずれかを入力してください。\n";
            } else {
                $valid = true;
            }
        }

        if ($answer === $quiz["answer"]) {
            $remainingQuestions = $totalQuestions - $correctCount - 1;
            $correctCount++;
            if ($remainingQuestions === 0) {
                break;
            }
            echo "正解! 残り{$remainingQuestions}問\n";
        } else {
            echo "不正解。認証失敗です。\n";
            return 1;
        }
    }

    if ($correctCount === $quizCount) {
        echo "おめでとう！すべて正解です！\n";
        return 0;
    } else {
        echo "残念！不正解があります。\n";
        return 1;
    }
}
