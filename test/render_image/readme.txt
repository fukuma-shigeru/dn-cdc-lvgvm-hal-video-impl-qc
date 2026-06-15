
■概要
指定された画像ファイルを描画するプログラム


■制限事項
NV12/PNG/UYVYフォーマットのみ対応
[0.4.0.0-rc8]以降で利用可能。（ツール内部で使用しているライブラリのI/Fを変更しているため）


■ビルド
SDKのenvironment-setup-cortexa78c-arene-linuxで環境設定
mkdir build
cd build
cmake .. ; make all -j24


■実機環境構築
実機にrender_imageと画像ファイル（例：VdecOut0080.yuv）を転送
render_imageに実行権限を付与（chmod +x）


■コマンド
・VdecOut0080.yuvの絵を、サーフェスID(21)で前席に描画
./render_image -F 21 -I VdecOut0080.yuv -D 1440x1088 -Y n &

・VdecOut0080.yuvの絵を、サーフェスID(21)で前席、サーフェスID(150)で後席に描画
./render_image -F 21 -R 150 -I VdecOut0080.yuv -D 1440x1088 -Y n  &

・VdecOut0080.yuvの絵を、サーフェスID(21)で前席、サーフェスID(250)でメーターに描画
./render_image -F 21 -M 250 -I VdecOut0080.yuv -D 1440x1088 -Y n  &


・PNGの場合
実機にrender_imageと画像ファイル（例：sample.png）を転送

■コマンド
・sample.pngの絵を、サーフェスID(21)で前席に描画
./render_image -F 21 -I sample.png -D 229x280 &

■追加オプション
-C オプション：GBMのColorSpace指定(実機の場合のみ有効)
-C 6 　→GBM  COLOR_SPACE_ITU_R_601
-C 6f　→GBM  COLOR_SPACE_ITU_R_601_FR
-C 7 　→GBM  COLOR_SPACE_ITU_R_709
-C 7f　→GBM  COLOR_SPACE_ITU_R_709_FR
-C その他 または -Cの指定なし　→ DMA

-Y オプション：YUVファイルフォーマット指定
-Y n　→ NV12ファイルとして扱う
-Y u　→ UYVYファイルとして扱う

　※ただしファイルの拡張子が以下の場合、
　　-Yオプションの有無にかかわらずファイルフォーマットを決定します。
　　　.png 　→ PNGファイル
　　　.nv12　→ NV12ファイル
　　　.uyvy　→ UYVYファイル
