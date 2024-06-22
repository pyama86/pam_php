# pam-php モジュール

`pam-php` は、PAM (Pluggable Authentication Modules) と PHP を統合するためのモジュールです。これにより、PHP スクリプトを利用してシステム認証のプロセスをカスタマイズできます。

## 特徴

- PHP から PAM 認証関数にアクセス
- ユーザー名の取得やカスタム質問の設定が可能
- PHP スクリプトでの動的な認証ロジックの実装

## デモ
![image](https://github.com/pyama86/pam_php/assets/8022082/b84ad42d-9fe9-4551-a915-3dfe6c8b8894)

```
$ make docker
$ su - gopher
$ su - phper
```

## 依存関係

- PHP 7.4 以上
- PAM ライブラリ
- GCC コンパイラ (モジュールのコンパイルに必要)

## インストール

### 前提条件

- PHP がインストールされ、コマンドラインから実行可能であることを確認してください。
- 必要なヘッダーファイルがシステムにインストールされていることを確認してください。

### ビルドとインストール

1. ソースコードをダウンロードまたはクローンします。
2. モジュールをコンパイルします。
3. コンパイルしたモジュールをシステムにインストールします。

## 使用方法

### PHP スクリプトの例

以下は、PAM 認証を行う PHP スクリプトの例です。このスクリプトは、ユーザーにいくつかのクイズ問題を出し、全問正解すると認証を通過するように設計されています。

```php
<?php

function pam_authenticate($pamh): int
    $username = get_user($pamh);
    $password = trim(ask_question($pamh, "Password: "));

    if ($usename == "admin" && $password == "secret") {
        return 0;
    } else {
        return 1;
    }
?>

```

### PAM 設定
/etc/pam.d/ にある適切な設定ファイル (例: common-auth) に以下を追加して、pam-php モジュールを有効にします。

```
auth sufficient pam_php.so /path/to/your/script.php
```


## ライセンス
このプロジェクトは MIT ライセンスの下で提供されます。詳細は LICENSE ファイルを参照してください。

## 貢献
このプロジェクトへの貢献を歓迎します。バグ報告、機能リクエスト、プルリクエストなどを GitHub リポジトリを通じて行ってください。
