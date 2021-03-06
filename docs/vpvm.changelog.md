# MMDAI2 (VPVM) の変更点まとめ

αの番号がそのままバージョンにつながっています。
0.19.0 までの変更内容は主にブログ記事からの転載です。

## 0.26.0 (2012/11/11)

 - 新機能
  - モーフタイムラインの下にスライダーを設け、モーフの選択状態を左上に表示するようにした
  - アクセサリ名をテキスト入力で検索出来るようにした
  - アクセサリをモデルと同等の扱いで処理できるようにした
 - 仕様変更
  - 物理パラメータの調整
  - ソフトウェアスキニングから OpenCL (CPU) に変更
  - ツールバー構成を変更し、画像つきにした
  - キーフレームを選択したら該当ボーンまたはモーフを選択状態にするようにした
 - 不具合修正
  - アクセサリの回転が一軸方向にしか効かない不具合の修正

## 0.25.1 (2012/9/10)

 - 不具合修正
  - PMX モデルで物理演算を有効にすると暴走する不具合の修正
  - 投影影の設定が正しく反映されていないこととモデルに追従していないことの不具合の修正
  - ボーン表示にモデルの回転が反映されていない不具合の修正

## 0.25.0 (2012/8/31)

 - 機能追加
  - MME 互換機能の実装 (試験運用)
  - MVD の読み込み及び書き出し機能の実装 (試験運用)
   - 現状 MVD 単体出力は可能だがプロジェクトへの書き出しは不可能
  - 背景に動画を読み込む機能の実装
   - MacOSX のみ
  - 光源のキーフレーム登録対応
  - OpenCL (CPU) の追加
   - 従来の OpenCL を OpenCL (GPU) に変更
 - 仕様変更
  - 静止画像出力時サイズを設定してから出力するように変更
  - エッジ幅の計算方法を変更
   - MMD と同じように二番目のボーンとカメラの距離で決定するようにした
  - DDS テクスチャ読み込みの対応
 - 不具合修正
  - 透明を持つテクスチャの描画がおかしくなる不具合の修正
  - 背景色のダイアログで B と G の値が入れ替わる不具合の修正
  - モデルを動かした時ボーンのワイヤー表示位置と実際のボーンの位置がずれる不具合の修正
  - いくつかのモデルの描画がおかしくなる、あるいは読み込めない不具合の修正
  - 編集中に IK ボーンが勝手に動く不具合の修正

## 0.24.1 (2012/7/3)

 - 不具合修正
  - アクセサリの位置と回転が反映されない不具合の修正
  - モーションの読み込みがモデルにすぐに反映されない不具合の修正

## 0.24.0 (2012/7/1)

 - 仕様変更
  - レンダリングエンジンの描画方法変更
   - 今後導入される予定の MME 互換のために描写方法を MME の basic.fx ベースに変更
   - アクセサリの描画が特に大きく変わり、全般的に明るくなる
  - アプリケーションを最小化時描画命令を停止して CPU/GPU 負荷を下げるようにした
 - 不具合修正 
  - モデルを追加したり変更したりするとタイムラインのボーン名やモーフ名の表示が変になることがある不具合の修正
  - アクセサリのテクスチャ解放漏れを起因とするメモリリークの不具合の修正
  - シークするとモーションが正しく動かない(0フレーム目にリセットされる)場合が発生する不具合の修正
  - プロジェクト読み込み後にモデルが透明になってしまう不具合の修正
  - OpenCL 有効時 PMX の頂点モーフが動かなくなる不具合の修正
  - PMX のスフィアが正しく適用されていない不具合の修正
  - PMD のエッジが透明になる不具合の修正

## 0.23.2 (2012/6/22)

 - 不具合修正
  - モーフスライダーが動かない不具合の修正

## 0.23.1 (2012/6/11)

 - 不具合修正
  - アクセサリを追加削除した後に追加すると落ちる不具合の修正

## 0.23.0 (2012/6/10)

 - 機能追加
  - タイムラインの伸縮化
  - 従来は 3600 秒分のフレームが事前に作成され変更できませんでしたが、今回から最初30フレームのみ作成し、ホイール、スクロールバー、現在のフレーム位置の数値指定で伸縮出来るようになっています。これにより処理が高速になります
   - 仕様上モーションが持つ最大キーフレーム値以下には縮小しません。また、モーションがない場合の最小値は30フレームです。
  - OpenCL/頂点シェーダスキニングの実装
   - 全体と個別の組み合わせで切り替えできるようになっています
   - エッジは未実装のため、従来のソフトウェアスキニングを使ってください
   - 頂点シェーダスキニングはボーン数が多いと破錠する恐れがあります
 - 仕様変更
  - ジェスチャ系の操作を全て無効化
   - アプリケーションの突然死の原因になっているため、一旦無効化しました
 - 不具合修正
  - 回転ボーンモーフを使ったモデルを読み込むと落ちる不具合の修正
  - モデル削除した後追加するとクラッシュする不具合の修正
  - プロジェクトからの読み込みでアクセサリ追従の設定が正しく処理されない不具合の修正

## 0.22.1 (2012/5/18)

 - 不具合修正
  - アクセサリのボーン追従に失敗する不具合の修正
  - PMX の付与ボーンの計算式の修正
  - SDEF ボーンのウェイト値が間違って読み込んでいた不具合の修正
  - 材質モーフが最初のひとつしか変形していない不具合の修正
  - 「全てのモーフをリセット」が機能しない不具合を修正
  - アクセサリ削除時にクラッシュする不具合の修正
  - モデル追加して削除後モデル追加してアクセサリ追従を実行するとクラッシュする不具合の修正

## 0.22.0 (2012/5/16)

PMX に対応する libvpvl2 を組み込んだ最初のバージョンです

 - 機能追加
  - PMX 読み込みの実装 ※いくつか未実装あり
  - セルフシャドウの実装
   - デフォルトは無効な点に注意
  - カメラ視点でマウスからボーンを移動させる機能の実装
  - 背景画像読み込みの実装
  - モデルの不透明度を調整する機能の実装
  - カメラ注視点のモデル/ボーン追従機能の実装
  - モーフ変形の巻き戻し/やり直しの実装
  - 捻りボーンの実装
 - 仕様変更
  - キーフレーム補間設定をダイアログとして分離
  - カメラタブの切り替えでモデル未選択、ボーン及びモーフタブの切替で前回選択したモデルに選択するように変更
  - ボーン及びモーフのタイムラインでシークした時カメラの移動を行わないようにした
  - 数値キーでカメラ視点を移動できるようにした
  - モーフタブに「全てのモーフをリセット」を追加
  - 視野角の最大値を 125 に変更
  - 視野距離の最小値を -10000 に変更

## 0.21.2 (2012/4/18)

 - 不具合修正
  - パスにバックスラッシュを含んだテクスチャ名を持つモデルが正しく表示されない不具合の修正

## 0.21.1 (2012/4/5)

 - 不具合修正
  - IK 処理で演算エラーが発生してモデルの一部が消失することがある不具合の修正
  - モーション読み込みでエラーが発生して読めないないことがある不具合を修正
  - 別のモーフに切り替えてもスライダーが更新されない不具合の修正

## 0.21.0 (2012/3/28)

 - 機能追加
  - 右下のハンドルに View モードを追加
 - 仕様変更
  - Local/Global のハンドル操作処理を MMD と近くなるように変更
  - ハンドル操作中はカーソルを非表示になるよう変更
  - 軸表示をハンドルがカーソルにあたったときのみに表示するように変更
  - ボーン表示の大きさを少し調整
 - 不具合修正
  - 新規プロジェクト作成時前回のモーション名が残る不具合の修正
  - モーションが変化しても終了位置の大きさが変わらない不具合を修正
  - カメラのキーフレームが正しく削除されない不具合を修正
 - 既知の不具合
  - カメラのキーフレーム登録が正しく動作しない不具合がある

## 0.20.0 (2012/3/20)

 - 機能追加
  - モデル及びアクセサリの zip 形式での読み込みの対応
  - 「切り取り」の追加
  - 場面の照明設定を追加
 - 仕様変更
  - コピー処理を現在のフレームの位置の列全選択から選択された範囲でコピーするように変更
  - モデルの絶対位置と回転量をプロジェクトに保存するように変更(修正)
  - モデルのエッジ幅の初期値を 0.0 に変更
  - アプリケーションのメニュー構成を変更
  - 再生設定から再生するとき再生設定のダイアログを隠すようにした
  - 背景色を黒にする設定から任意色で設定できるように変更
  - 補間設定をモデルタブからタイムラインタブに移動
  - 回転ハンドルをグローバル変形固定に(既知の問題としても含む)
 - 不具合修正
  - ウィンドウをリサイズすると情報パネルやハンドルの表示がおかしくなる不具合の修正
  - エッジが描画されない不具合の修正
  - 前回のプロジェクトを読み込んだ後新規にプロジェクトを作成すると前のファイル名が残る不具合の修正
  - アクセサリ削除後にアクセサリを追加すると落ちる不具合の修正
  - アクセサリ名が文字化ける不具合の修正
  - 「全ての親」ボーンがタイムラインに表示されない不具合の修正
  - 「センター」ボーンが別カテゴリに含まれる時重複して表示される不具合を修正
  - モーフが正しく登録されない不具合の修正
  - キーフレーム削除後巻き戻しが出来なくなる不具合の修正
  - 子ボーンが「全ての親」ボーンの場合放射状にレンダリングされる不具合の修正

## 0.19.0 (2012/3/10)

 - MacOSX 32bit 上でモデルを読み込むと落ちる不具合の修正

## 0.18.0 (2012/3/4)

 - 再生時及びエンコード時の音声出力機能の実装
  - 受け付ける音声ファイルは WAV フォーマットの PCM 2ch 44KHz です
 - 描画順設定の実装
 - キーフレームの重み付けの実装
 - 再生中に FPS を表示するようにし、再生設定から直接再生出来るようにした
 - タイムラインのスクロール時右ではなく中央に寄せるように変更
 - IK 関連のボーンをオレンジ色で表すようにした
 - 不具合修正
  - 重力設定が反映されない不具合の修正
  - 再生設定及びエンコード設定で終了位置が0になる不具合の修正
  - 静止画出力した結果がおかしくなる不具合の修正
  - モデルが読み込めなくなる問題をおそらく解決
  - 選択出来るはずのボーンが選択できない不具合の修正
  - UUID が重複するプロジェクトが読みこめてしまう不具合の修正
 - 既知の不具合
  - 音声付きで再生するとレンダリングが追いつかないと同期ズレを起こす
  - エンコードが終了すると情報パネルが表示されなくなる
  - 複数ヶ所翻訳漏れがある

## 0.17.0 (2012/2/19)

 - アクセサリのボーン追従の実装
 - 重力設定の実装
 - モデルとカメラのモーションの書き出しを別々にするように変更
 - 「プロジェクト」メニューにある設定をプロジェクトファイルに書きだすように変更
 - 投影影を個別に設定できるようにし、デフォルトで表示しないように変更
  - これにより一部モデルが投影影によって正しく表示されない不具合の修正
 - プロジェクト読み込み後保存(CTRL+S)時にダイアログを開かないように変更
 - プロジェクト読み込み時進捗ダイアログを表示するようにした
 - 「表情」を「モーフ」に変更
 - 不具合修正
  - ハンドルや情報パネルのレンダリングがおかしくなる不具合の修正
  - プロジェクト再読み込みでレンダリングがおかしくなる及び落ちる不具合の修正
  - アクセサリを削除すると「地面」が削除されることがある不具合の修正
  - 再生設定で落ちることがある不具合の修正
  - 動画エンコードで途中で動画が切れてしまう不具合の修正

## 0.16.0 (2012/2/5)

 - 動画出力機能の実装変更
  - OpenCV から libav に変更。これに伴い、Linux 版でも動画出力機能が利用可能になった
  - 出力時のフレームレートの設定が変更可能になった (30/60/120fps)
  - 出力形式は .mov に変更、中身は前と同じ PNG 圧縮のフレームデータ
 - 初期状態の視野角を 16 から 30 に、距離を 100 から 50 に変更
 - ボーンの選択、回転の表示モードを分離(ボーンのタイムラインの下から選択するようにした)
 - エッジ幅の変更をモデル情報から変更できるように変更し、ダイアログ廃止
 - モデル選択で赤色のエッジを出さないように変更
 - 不具合修正
  - プロジェクトが読み込みできない不具合の修正
  - プロジェクトを一旦読み込んで新規作成し、再度読み込むと落ちる不具合の修正
  - プロジェクトを開くダイアログを表示して何も選択しなかった場合モデルが消えて落ちる不具合の修正
 - 既知の問題
  - アクセサリを読み込むとシェーダ側のミスで真っ黒けになる
  - ↑については別の問題と一緒に抱えているのでそれが解決してから

## 0.15.0 (2012/1/25)

 - UI変更と機能追加
  - ドック分けし、各パネルがフロート表示及び移動可能になった
  - コンテキストメニューの追加
  - 黒背景表示の追加
  - ボーン表示を少し改善、選択中のボーンは赤く表示するように
 - 不具合修正
  - SPH/SPA がある場合のみ2番目のテクスチャを読むように変更
  - モデル名とボーン名を表示するパネルが表示されていない不具合の修正
  - アクセラレーションの切り替えで落ちる不具合の修正
  - 再生及び動画出力時物理演算が暴発する不具合の修正
  - 再生処理が異常に遅い不具合の修正
  - 拡張子の食い違いが原因でプロジェクトの読み込みが出来ない不具合の修正
  - Qt 4.8 の切り替えの影響で tga が読めないケースの対処
  - ローカル軸の回転時の挙動の修正
 - 既知で未修正の不具合
  - 右下のハンドル表示の色表示がくすんで見える不具合
  - 動画出力時一定のキーフレームまで出力すると急激に処理速度が落ちる不具合

## 0.14.0 (2012/1/16)

 - Linux 版バイナリの提供開始
  - Ubuntu 11.10 と Knoppix 6.4.1 で動作確認
  - 制約として現在動画出力が現在できない
 - OpenCL の対応 (MacOSX 版のみ)
  - 詳細は OpenCL を使ったアクセラレーション の項目にて
 - ジェスチャー操作にスワイプ操作で巻き戻しとやり直しが出来るように追加
 - タッチ操作に基づくジェスチャー の項目を新規公開及び追加

## 0.13.0 (2012/1/10)

 - ジェスチャー操作の追加
  - タッチ操作に基づくジェスチャー操作 の項目を公開しました (1/16 追記)
  - タッチアンドドラッグ
   - ボーンが選択されている場合はボーン移動、モデル未選択の場合はカメラ移動(ボーン未選択でモデルのみの場合はモデル移動)
  - ピンチ(回転)
   - ボーンが選択されている場合はボーンを回転、モデル未選択の場合はカメラ回転(ボーン未選択でモデルのみの場合はモデル回転)
  - ピンチイン・ピンチアウト
    - 場面の拡大縮小
  - タッチアンドドラッグのみ初期状態では無効、「ビュー」の「Enable move gesture」で有効にできる
  - 上記が無効なのはトラックパッド使用時での誤操作を引き起こしやすいため
  - ピンチ(回転)はいずれも現時点では Y 軸に対してのみ回転する
 - 表情モーフ操作時キーフレームの値が反映されていない不具合の修正
 -【内部情報】Qt 4.8 への移行

## 0.12.0 (2012/1/5)

 - 機能修正
  - プロジェクト及びモーションのファイルを別名として保存するメニューを追加
  - ウィンドウのタイトルに保存したプロジェクト及びモーションのファイル名を表示するように変更
 - 不具合修正
  - MacOSX で起動しない不具合を修正
  - ボーンで巻き戻しが正常に動かないケースがある不具合の修正

## 0.11.0 (2012/1/4)

 - プロジェクト保存の対応
  - xml 形式で保存されます
 - UI の改善
  - ハンドルの実装
   - PMDE のような 3D ハンドルです
   - 仕組み上 180 度以上の回転は出来ません
  - ツールバーの追加
   - 画像はまだありません
   - モデルを読み込んだ後センターボーンを選択するようにした
   - 「次/前のモデルの選択」を追加
   - キーフレームが登録されていればカテゴリ部分に ◇ をつけるようにした
   - 全てのキーフレームの選択及びキーフレーム選択ダイアログの実装
   - モデルを削除する際確認ダイアログを表示するようにした
   - 設定ファイルからキーバインディングの変更可能になった
    - まだ設定するための UI がない
 - 以下の条件に該当するモデルが正しく読み込めない不具合の修正
  - テクスチャ名に Windows のパス区切り (バックスラッシュ) が入ってる
  - 頂点モーフで仕様外のデータが入ってる

## 0.10.0 (2011/11/23)

 - モーション補間パラメータの実装
  - 自動補間機能は持っていません
  - カメラの場合はキーフレームを選択すると現在の補間パラメータが反映される
  - ボーンの場合は現状キーフレームではなくボーンを選択すると現在の補間パラメータが反映される
 - 「再生」の仕組みを変更
  - MMDと似た形のレンダリングチェック方式に変更
  - 従来の自由に再生したり停止したりするやり方はどうするか現在未定
  - 「最近開いたファイル」メニューの追加
  - アクセサリのレンダリング方法を変更
  - 場面からのボーン選択の精度向上
 - 不具合修正
  - モーションの読み込みが異常に遅い不具合の修正
  - 空のカメラモーションを読み込ませてシークすると落ちる不具合の修正
  - カメラの補間パラメータが正しい形で保存されていない不具合の修正

## 0.9.0 (2011/11/16)

 - カメラモーション作成の対応
  - カメラモーションは「モーションをVMDとして保存」で一緒に含まれます
  - カメラモーションとして保存されるには最低２つ以上のキーフレームが必要です
  - カメラタブから各パラメータを直接編集できるようになりました
 - 不具合修正
  - 新規モーション作成で前のモーション削除が正しく機能しない不具合の修正
  - キーフレームの0番目が登録されない不具合の修正
  - センターボーンが表示されず、キーフレームとして登録できない不具合の修正
  - アクセサリ名が文字化けている不具合の修正

## 0.8.0 (2011/11/8)

 - レンダリングエンジンの大幅な修正(詳細は後述)
 - 反転ペーストの実装
 - モデル読み込み時にダイアログを表示するようにした
 - 不具合修正
  - ダミーボーンのようなインデックスがないモデルを読み込むと落ちる不具合の修正
  - キーフレームを削除しようとすると一つ後ろのフレームに対して削除される不具合の修正
  - ヘッダーの入れ替えでデータの並びが変わるため、ヘッダーを移動不可にした

α7 までのレンダリング実装がグーローシェーディングを元にした実装を行なっていましたが
、モデルによって正しく表示されない (特に色が白飛びしたりする) ことが多々あったため、
PMDEditor の README を元に計算式を変更しました。この変更によって MMD とだいたい同じ
レンダリング結果になっているはず...です。

## 0.7.0 (2011/10/30)

 - UI の強化
  - キーフレーム登録をタイムラインとメニューに追加
  - エッジ幅の調整が可能になった
  - タイムラインで個々のキーフレームに選択できるように変更
  - タイムラインのボーン名で該当のボーンを選択出来るようにした
  - Global と Local の変更を右下のテキストクリックで出来るようになった
  - フレーム数の変更で該当箇所に自動的にスクロールするようにした
 - 不具合修正
  - カメラモーションが動かない問題を修正
  - 再選択するとキーが重複してしまう不具合を修正
  - 0番目のキーフレームが削除出来てしまう不具合を修正
  - 削除しても選択状態が残る不具合を修正
  - アイコン付加 (MMDAI と同じく Lat 式ミクを用いたもの)

## 0.6.0 (2011/10/23)

 - 変更点
  - コピー/ペースト機能の実装(ただし反転ペーストはまだなし)
  - ボーンをダブルクリックすることで選択出来るようにした
  - 左上にモデルとボーンの名前を表示するパネルの設置
  - フレームが重複する不具合の修正
  - ポーズを読みこませると0フレーム目に移動してしまう不具合の修正
 - 既知の問題
  - コピペがキーボードショートカットから操作できない。一応メニューを表示させると動作する(MacOSX)
  - ボーンの選択精度が低く、時々意図しない違うボーンが選択される時がある

## 0.5.0 (2011/10/16)

 - 変更点
  - 動画出力機能の実装
   - 現状 MacOSX 版のみ。Windows 版は実行出来ないことを示すダイアログを出す
   - AVI コンテナで、中身は 29.97 fps の PNG として出力
   - 実質生の状態で、かなり大きなファイルになるので要エンコード
  - UI を変更し、タイムラインとタブを統合
  - ライティングの強さを元に戻した
  - MacOSX 版で x86 バイナリがなかった問題を修正
  - 選択されたモデルがない状態で「全てのモデルにモーションを追加」を行うと落ちる問題を修正
 - 既知の問題
  - 新規モーションからそのまま動画出力できないかもしれない。一旦保存して読み込ませると出力可能

## 0.4.0 (2011/10/11)

 - 変更点
  - 日本語訳の追加
  - フレームのスピンボックスでモーションの更新が入らない問題の修正
  - ポーズ読み込みですぐ反映されない問題の修正
  - 「新規モーション」がなくなってしまった問題の修正
  - 最大フレーム数を1800(60秒)から54000(30分)に変更
 - 既知の問題
  - α3にある既知の問題点に関して未だ変更なし
  - 列ではなく行を選択するとフレームのインデックスが0になる。現状仕様
  - 白飛び問題のためモデルを一旦暗くした

## 0.3.0 (2011/10/8)

 - 変更点
  - レンダリングエンジンの変更
  - アクセサリの暫定対応
   - DirectX の X 形式の読み込みが可能に
   - これにあわせて vac の読込と保存に対応
   - ボーン追従は未実装
  - 物理エンジンの有効無効切り替えが可能になった
  - 場面の画像書き出し(スクリーンショット)が可能になった
  - UI 変更
   - ボーンと表情をツリーで表示するようになった
   - 画面左下にハンドルアイコンを付加
   - グリッドとボーンの表示切り替えが可能になった
   - TransformView の "View" 操作の仮対応
  - 既知の問題
   - レンダリング結果に不安あり
   - アクセサリのボーン追従は未実装
   - ハンドル操作が TransformView のプルダウンの状態に依存するがこれがわかりにくい

## 0.2.0 (2011/8/31)

 - 変更点
  - Undo/Redo の実装
  - モーション再生機能の実装
  - 全ボーン表示と選択時の座標系の表示
  - 不具合修正 (#18)
 - 既知の問題
  - モーション再生機能はファイルから読み込んだもののみ
   - 暫定的な対処として一度モーションとして書きだしてからそれを読み出すことで回避可能
  - 新 MBA/MBP などにおける描画の不具合
   - Intel HD Graphics 3000 上だと現在の描画の仕組み (固定機能シェーダ) では正しくされない様子
   - 現在開発中の新しい描画の仕組み (プログラマブルシェーダ) で対処可能なためはやめに採用します

## 0.1.0 (2011/8/20)

 - 初版

