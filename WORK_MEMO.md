# 作業メモ

更新日: 2026-08-27

## プロジェクト

- ローカル: `C:\Users\kjymk\MPLABXProjects\Tammamy02.X`
- PIC: PIC16F1828
- コンパイラ: MPLAB XC8 v2.05
- GitHub: https://github.com/kjymkw-hobby/Tamammy02
- ブランチ: `main`

## 今回やったこと

1. `README.md`を読んで、15個のLEDをロケット噴射のように明滅させるプログラムだと確認した。
2. MPLAB Xプロジェクトに`main.c`が登録されていなかったため、`nbproject/configurations.xml`へ登録した。
3. XC8 v2.05でPIC16F1828向けにコンパイルできることを確認した。
4. MPLAB XでClean and Buildを実行し、HEX生成に成功した。
5. HEXを書き込み、手元にあった3個のLEDで不規則な明滅を確認した。
6. ローカルGitリポジトリを初期化した。
7. ビルド生成物をGit管理から除外するため、`.gitignore`に次を追加した。
   - `debug/`
   - `*.d`
   - `*.p1`
   - `*.o`
8. 初回コミットを作成し、GitHubへpushした。

## Gitの状態

リモートは設定済み。

```powershell
git remote -v
```

接続先:

```text
https://github.com/kjymkw-hobby/Tamammy02.git
```

## 次回、変更をGitHubへ上げる手順

```powershell
cd C:\Users\kjymk\MPLABXProjects\Tammamy02.X
git status
git add .
git commit -m "変更内容"
git push
```

`git push`だけで、現在の`main`ブランチからGitHubの`origin/main`へ送信できる。

## 注意

- LEDはPIC出力から抵抗を通して接続する。LEDごとに抵抗を1本使う。
- 現在のプログラムは15個のLEDに対応している。
- LEDはActive-HIGH（PIC出力HIGHで点灯）を前提としている。
- `update_led_outputs()`未使用の警告が出るが、ビルド失敗ではない。
